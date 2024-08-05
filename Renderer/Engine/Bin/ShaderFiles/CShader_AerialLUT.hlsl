
#include "Engine_Shader_Defines.hpp"

#define PI 3.1415926535897932384626433832795f
#define PLANET_RADIUS_OFFSET 0.01f
#define DEPTHCOUNT 32.0f
#define M_PER_SLICE 4000.0f

RWTexture3D<float4> OutputTexture : register (u0);
Texture2D<float4>	g_TransLUTTexture : register(t0);


cbuffer AtmosphereParams : register(b0)
{
	float4	vScatterRayleigh;
	float	fHDensityRayleigh;

	float	fScatterMie;
	float	fPhaseMieG;
	float	fExtinctionMie;
	float	fHDensityMie;

	float	fEarthRadius;
	float	fAtmosphereRadius;

	float	fSunIlluminance;

	float4	vAbsorbOzone;
 	float4	vOzone;

	float	fMultiScatFactor;
};

cbuffer GlobalParams : register(b1)
{
	matrix g_ProjMatrixInv;
	matrix g_ViewMatrixInv;
	
	float4 g_vCamPosition;

	float4 g_vLightDir;
};

float Cornette_Shanks_Phase(float fG, float fCosTheta)
{
	float fNumer = 1.0f - fG * fG;
	float fDenom = 1.0f + fG * fG + 2.0f * fG * fCosTheta;
	return fNumer / (4.0f * PI * fDenom * sqrt(fDenom));
}

float RayleighPhase(float fCosTheta)
{
	float fFactor = 3.0f / (16.0f * PI);
	return fFactor * (1.0f + fCosTheta * fCosTheta);
}


float raySphereIntersectNearest(float3 r0, float3 rd, float3 s0, float sR)
{
	float a = dot(rd, rd);
	float3 s0_r0 = r0 - s0;
	float b = 2.0 * dot(rd, s0_r0);
	float c = dot(s0_r0, s0_r0) - (sR * sR);
	float delta = b * b - 4.0 * a * c;
	if (delta < 0.0 || a == 0.0)
	{
		return -1.0;
	}
	float sol0 = (-b - sqrt(delta)) / (2.0 * a);
	float sol1 = (-b + sqrt(delta)) / (2.0 * a);
	if (sol0 < 0.0 && sol1 < 0.0)
	{
		return -1.0;
	}
	if (sol0 < 0.0)
	{
		return max(0.0, sol1);
	}
	else if (sol1 < 0.0)
	{
		return max(0.0, sol0);
	}
	return max(0.0, min(sol0, sol1));
}


bool MoveToTopAtmosphere(inout float3 WorldPos, in float3 WorldDir, in float AtmosphereTopRadius)
{
	float viewHeight = length(WorldPos);
	if (viewHeight > AtmosphereTopRadius)
	{
		float tTop = raySphereIntersectNearest(WorldPos, WorldDir, float3(0.0f, 0.0f, 0.0f), AtmosphereTopRadius);
		if (tTop >= 0.0f)
		{
			float3 UpVector = WorldPos / viewHeight;
			float3 UpOffset = UpVector * -PLANET_RADIUS_OFFSET;
			WorldPos = WorldPos + WorldDir * tTop + UpOffset;
		}
		else
		{
			return false;
		}
	}
	return true;
}


struct MediumSampleRGB
{
	float3 vScattering;
	float3 vAbsorption;
	float3 vExtinction;

	float3 vScatteringMie;
	float3 vAbsorptionMie;
	float3 vExtinctionMie;

	float3 vScatteringRay;
	float3 vAbsorptionRay;
	float3 vExtinctionRay;

	float3 vScatteringOzo;
	float3 vAbsorptionOzo;
	float3 vExtinctionOzo;

	float3 vAlbedo;
};

MediumSampleRGB SampleMediumRGB(in float3 vWorldPos)
{
	const float fViewHeight = length(vWorldPos) - fEarthRadius;

	const float fDensityMie = exp(-fViewHeight / fHDensityMie);
	const float fDensityRay = exp(-fViewHeight / fHDensityRayleigh);
	const float fDensityOzo = max(0.0f, 1 - 0.5 * abs(fViewHeight - vOzone.x) / vOzone.y);

	MediumSampleRGB tResult;

	tResult.vScatteringMie = fDensityMie * fScatterMie;
	tResult.vAbsorptionMie = fDensityMie * (fExtinctionMie - fScatterMie);
	tResult.vExtinctionMie = fDensityMie * fExtinctionMie;

	tResult.vScatteringRay = fDensityRay * vScatterRayleigh.xyz;
	tResult.vAbsorptionRay = 0.0f;
	tResult.vExtinctionRay = tResult.vScatteringRay + tResult.vAbsorptionRay;

	tResult.vScatteringOzo = 0.0f;
	tResult.vAbsorptionOzo = fDensityOzo * vAbsorbOzone;
	tResult.vExtinctionOzo = tResult.vScatteringOzo + tResult.vAbsorptionOzo;

	tResult.vScattering = tResult.vScatteringMie + tResult.vScatteringRay + tResult.vScatteringOzo;
	tResult.vAbsorption = tResult.vAbsorptionMie + tResult.vAbsorptionRay + tResult.vAbsorptionOzo;
	tResult.vExtinction = tResult.vExtinctionMie + tResult.vExtinctionRay + tResult.vExtinctionOzo;

	return tResult;
}

void LutTransmittanceParamsToUv(in float fViewHeight, in float fViewZenithCosAngle, out float2 vUV)
{
	float fH = sqrt(max(0.0f, fAtmosphereRadius * fAtmosphereRadius - fEarthRadius * fEarthRadius));
	float fRho = sqrt(max(0.0f, fViewHeight * fViewHeight - fEarthRadius * fEarthRadius));

	float fDiscriminant = fViewHeight * fViewHeight * (fViewZenithCosAngle * fViewZenithCosAngle - 1.0f) + fAtmosphereRadius * fAtmosphereRadius;
	float fD = max(0.0f, (-fViewHeight * fViewZenithCosAngle + sqrt(fDiscriminant)));

	float fMin = fAtmosphereRadius - fViewHeight;
	float fMax = fRho + fH;
	float fX = (fD - fMin) / (fMax - fMin);
	float fY = fRho / fH;

	vUV = float2(fX, fY);
}

struct SingleScatteringResult
{
	float3 L;						
	float3 OpticalDepth;			
	float3 Transmittance;			
	float3 MultiScatAs1;

	float3 NewMultiScatStep0Out;
	float3 NewMultiScatStep1Out;
};

SingleScatteringResult IntegrateScatteredLuminance(
	in float3 WorldPos, in float3 WorldDir, in float3 SunDir,
	in float SampleCountIni, in float tMaxMax = 9000000.0f)
{
	SingleScatteringResult result = (SingleScatteringResult)0;

	float3 earthO = float3(0.0f, 0.0f, 0.0f);
	float tBottom = raySphereIntersectNearest(WorldPos, WorldDir, earthO, fEarthRadius);
	float tTop = raySphereIntersectNearest(WorldPos, WorldDir, earthO, fAtmosphereRadius);
	float tMax = 0.0f;
	if (tBottom < 0.0f)
	{
		if (tTop < 0.0f)
		{
			tMax = 0.0f; 
			return result;
		}
		else
		{
			tMax = tTop;
		}
	}
	else
	{
		if (tTop > 0.0f)
		{
			tMax = min(tTop, tBottom);
		}
	}

	tMax = min(tMax, tMaxMax);

	float SampleCount = SampleCountIni;
	float SampleCountFloor = SampleCountIni;
	float tMaxFloor = tMax;
	float dt = tMax / SampleCount;

	const float3 wi = SunDir;
	const float3 wo = WorldDir;
	float cosTheta = dot(wi, wo);
	float MiePhaseValue = Cornette_Shanks_Phase(fPhaseMieG, -cosTheta);
	float RayleighPhaseValue = RayleighPhase(cosTheta);

	float3 globalL = fSunIlluminance;

	float3 L = 0.0f;
	float3 throughput = 1.0;
	float3 OpticalDepth = 0.0;
	float t = 0.0f;
	float tPrev = 0.0;
	const float SampleSegmentT = 0.3f;
	for (float s = 0.0f; s < SampleCount; s += 1.0f)
	{
		float NewT = tMax * (s + SampleSegmentT) / SampleCount;
		dt = NewT - t;
		t = NewT;
		float3 P = WorldPos + t * WorldDir;

		MediumSampleRGB medium = SampleMediumRGB(P);
		const float3 SampleOpticalDepth = medium.vExtinction * dt;
		const float3 SampleTransmittance = exp(-SampleOpticalDepth);
		OpticalDepth += SampleOpticalDepth;

		float pHeight = length(P);
		const float3 UpVector = P / pHeight;
		float SunZenithCosAngle = dot(SunDir, UpVector);
		float2 uv;
		LutTransmittanceParamsToUv(pHeight, SunZenithCosAngle, uv);
		float3 TransmittanceToSun = 0.5f;//g_TransLUTTexture.SampleLevel(LinearClampSampler, uv, 0).rgb;

		float3 PhaseTimesScattering = medium.vScatteringMie * MiePhaseValue + medium.vScatteringRay * RayleighPhaseValue;

		float tEarth = raySphereIntersectNearest(P, SunDir, earthO + PLANET_RADIUS_OFFSET * UpVector, fEarthRadius);
		float earthShadow = tEarth >= 0.0f ? 0.0f : 1.0f;


		float3 S = globalL * (TransmittanceToSun * PhaseTimesScattering);


		float3 Sint = (S - S * SampleTransmittance) / medium.vExtinction;	
		L += throughput * Sint;	
		throughput *= SampleTransmittance;

		tPrev = t;
	}

	result.L = L;
	result.OpticalDepth = OpticalDepth;
	result.Transmittance = throughput;
	return result;
}

[numthreads(8, 8, 8)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
	float2 vUV = float2((float)DTid.x / 32.0f, (float)DTid.y / 32.0f);

	vector		vClipPos;

	vClipPos.x = vUV.x * 2.f - 1.f;
	vClipPos.y = vUV.y * -2.f + 1.f;
	vClipPos.z = 1.0f;
	vClipPos.w = 1.f;

	vClipPos = vClipPos * 1000.0f;
	vClipPos = mul(vClipPos, g_ProjMatrixInv);
	vClipPos = mul(vClipPos, g_ViewMatrixInv);

	float3 vWorldPos = vClipPos.xyz;
	float3 vWorldDir = normalize(vWorldPos - g_vCamPosition.xyz);
	vWorldPos = g_vCamPosition.xyz + float3(0, fEarthRadius, 0);
	vWorldPos = vWorldPos.xzy;
	vWorldDir = vWorldDir.xzy;



	float earthR = fEarthRadius;
	float3 camPos = g_vCamPosition.xzy + float3(0, 0, earthR);
	float3 SunDir = g_vLightDir.xzy;


	float Slice = ((float(DTid.z) + 0.5f) / DEPTHCOUNT);
	Slice *= Slice;
	Slice *= DEPTHCOUNT;

	float3 WorldPos = camPos;
	float viewHeight;

	float tMax = Slice * M_PER_SLICE;
	float3 newWorldPos = WorldPos + tMax * vWorldDir;

	viewHeight = length(newWorldPos);
	if (viewHeight <= (fEarthRadius + PLANET_RADIUS_OFFSET))
	{
		newWorldPos = normalize(newWorldPos) * (fEarthRadius + PLANET_RADIUS_OFFSET + 0.001f);
		vWorldDir = normalize(newWorldPos - camPos);
		tMax = length(newWorldPos - camPos);
	}

	float tMaxMax = tMax;

	viewHeight = length(WorldPos);
	if (viewHeight >= fAtmosphereRadius)
	{
		float3 prevWorlPos = WorldPos;
		if (!MoveToTopAtmosphere(WorldPos, vWorldDir, fAtmosphereRadius))
		{
			OutputTexture[DTid] = float4(1.0f, 0.0f, 0.0f, 1.0f);
			return;
		}
		float LengthToAtmosphere = length(prevWorlPos - WorldPos);
		if (tMaxMax < LengthToAtmosphere)
		{
			OutputTexture[DTid] = float4(1.0f, 0.0f, 0.0f, 1.0f);
			return;
		}
		tMaxMax = max(0.0, tMaxMax - LengthToAtmosphere);
	}

	const float SampleCountIni = max(1.0f, float(DTid.z + 1.0f) * 2.0f);
	SingleScatteringResult ss = IntegrateScatteredLuminance(WorldPos, vWorldDir, SunDir, SampleCountIni, tMaxMax);

	const float Transmittance = dot(ss.Transmittance, float3(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f));

	OutputTexture[DTid] = float4(ss.L, 1.0 - Transmittance);

}




