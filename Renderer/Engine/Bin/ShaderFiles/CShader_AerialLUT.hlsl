
#include "Engine_Shader_Defines.hpp"

#define PI 3.1415926535897932384626433832795f
#define PLANET_RADIUS_OFFSET 0.01f
#define DEPTHCOUNT 32.0f
#define M_PER_SLICE 4000.0f

RWTexture3D<float4> OutputTexture : register (u0);
Texture2D<float4>	g_TransLUTTexture : register(t0);
Texture2D<float4>	g_MultiScatLUTTexture : register(t1);

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

float3 GetMultipleScattering(float3 scattering, float3 extinction, float3 worlPos, float viewZenithCosAngle)
{
	float2 uv = saturate(float2(viewZenithCosAngle * 0.5f + 0.5f, (length(worlPos) - fEarthRadius) / (fAtmosphereRadius - fEarthRadius)));

	float3 multiScatteredLuminance = g_MultiScatLUTTexture.SampleLevel(LinearClampSampler, uv, 0).rgb;
	return multiScatteredLuminance;
}

struct SingleScatteringResult
{
	float3 vL;						
	float3 vOpticalDepth;			
	float3 vTransmittance;			
	float3 vMultiScatAs1;

	float3 vNewMultiScatStep0Out;
	float3 vNewMultiScatStep1Out;
};

SingleScatteringResult IntegrateScatteredLuminance(
	in float3 vWorldPos, in float3 vWorldDir, in float3 vSunDir,
	in float fSampleCountIni, in float fMaxMax = 9000000.0f)
{
	SingleScatteringResult tResult = (SingleScatteringResult)0;

	float3 vEarthOrigin = float3(0.0f, 0.0f, 0.0f);
	float fBottom = raySphereIntersectNearest(vWorldPos, vWorldDir, vEarthOrigin, fEarthRadius);
	float fTop = raySphereIntersectNearest(vWorldPos, vWorldDir, vEarthOrigin, fAtmosphereRadius);
	float fMax = 0.0f;
	if (fBottom < 0.0f)
	{
		if (fTop < 0.0f)
		{
			fMax = 0.0f; 
			return tResult;
		}
		else
		{
			fMax = fTop;
		}
	}
	else
	{
		if (fTop > 0.0f)
		{
			fMax = min(fTop, fBottom);
		}
	}

	fMax = min(fMax, fMaxMax);

	float fSampleCount = fSampleCountIni;
	float fSampleCountFloor = fSampleCountIni;
	float fMaxFloor = fMax;
	float fDt = fMax / fSampleCount;

	const float3 vWi = vSunDir;
	const float3 vWo = vWorldDir;
	float fCosTheta = dot(vWi, vWo);
	float fMiePhaseValue = Cornette_Shanks_Phase(fPhaseMieG, -fCosTheta);
	float fRayleighPhaseValue = RayleighPhase(fCosTheta);

	float3 vGlobalL = fSunIlluminance;

	float3 vL = 0.0f;
	float3 vThroughput = 1.0;
	float3 vOpticalDepth = 0.0;
	float fT = 0.0f;
	float fPrev = 0.0;
	const float fSampleSegmentT = 0.3f;
	for (float i = 0.0f; i < fSampleCount; i += 1.0f)
	{
		float fNewT = fMax * (i + fSampleSegmentT) / fSampleCount;
		fDt = fNewT - fT;
		fT = fNewT;
		float3 vPos = vWorldPos + fT * vWorldDir;

		MediumSampleRGB Medium = SampleMediumRGB(vPos);
		const float3 vSampleOpticalDepth = Medium.vExtinction * fDt;
		const float3 vSampleTransmittance = exp(-vSampleOpticalDepth);
		vOpticalDepth += vSampleOpticalDepth;

		float fHeight = length(vPos);
		const float3 vUpVector = vPos / fHeight;
		float fSunZenithCosAngle = dot(vSunDir, vUpVector);
		float2 vUV;
		LutTransmittanceParamsToUv(fHeight, fSunZenithCosAngle, vUV);
		float3 vTransmittanceToSun = g_TransLUTTexture.SampleLevel(LinearClampSampler, vUV, 0).rgb;

		float3 vPhaseTimesScattering = Medium.vScatteringMie * fMiePhaseValue + Medium.vScatteringRay * fRayleighPhaseValue;

		float fEarth = raySphereIntersectNearest(vPos, vSunDir, vEarthOrigin + PLANET_RADIUS_OFFSET * vUpVector, fEarthRadius);
		float fEarthShadow = fEarth >= 0.0f ? 0.0f : 1.0f;

		float3 multiScatteredLuminance = 0.0f;
		multiScatteredLuminance = GetMultipleScattering(Medium.vScattering, Medium.vExtinction, vPos, fSunZenithCosAngle);

		float3 vS = vGlobalL * (fEarthShadow * vTransmittanceToSun * vPhaseTimesScattering + multiScatteredLuminance * Medium.vScattering);

		float3 vSint = (vS - vS * vSampleTransmittance) / Medium.vExtinction;	
		vL += vThroughput * vSint;	
		vThroughput *= vSampleTransmittance;

		fPrev = fT;
	}

	tResult.vL = vL;
	tResult.vOpticalDepth = vOpticalDepth;
	tResult.vTransmittance = vThroughput;
	return tResult;
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



	float fEarthR = fEarthRadius;
	float3 vCamPos = g_vCamPosition.xyz + float3(0, fEarthR, 0);
	float3 vSunDir = g_vLightDir.xyz;


	float fSlice = ((float(DTid.z) + 0.5f) / DEPTHCOUNT);
	fSlice *= fSlice;
	fSlice *= DEPTHCOUNT;

	vWorldPos = vCamPos;
	float fViewHeight;

	float fMax = fSlice * M_PER_SLICE;
	float3 vNewWorldPos = vWorldPos + fMax * vWorldDir;

	fViewHeight = length(vNewWorldPos);
	if (fViewHeight <= (fEarthRadius + PLANET_RADIUS_OFFSET))
	{
		vNewWorldPos = normalize(vNewWorldPos) * (fEarthRadius);
		vWorldDir = normalize(vNewWorldPos - vCamPos);
		fMax = length(vNewWorldPos - vCamPos);
	}

	float fMaxMax = fMax;

	fViewHeight = length(vWorldPos);
	if (fViewHeight >= fAtmosphereRadius)
	{
		float3 vPrevWorlPos = vWorldPos;
		if (!MoveToTopAtmosphere(vWorldPos, vWorldDir, fAtmosphereRadius))
		{
			OutputTexture[DTid] = float4(0.0f, 0.0f, 0.0f, 1.0f);
			return;
		}
		float fLengthToAtmosphere = length(vPrevWorlPos - vWorldPos);
		if (fMaxMax < fLengthToAtmosphere)
		{
			OutputTexture[DTid] = float4(0.0f, 0.0f, 0.0f, 1.0f);
			return;
		}
		fMaxMax = max(0.0, fMaxMax - fLengthToAtmosphere);
	}

	const float fSampleCountIni = max(1.0f, float(DTid.z + 1.0f) * 2.0f);
	SingleScatteringResult tResult = IntegrateScatteredLuminance(vWorldPos, vWorldDir, vSunDir, fSampleCountIni, fMaxMax);

	const float fTransmittance = dot(tResult.vTransmittance, float3(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f));

	OutputTexture[DTid] = float4(tResult.vL, 1.0 - fTransmittance);

}




