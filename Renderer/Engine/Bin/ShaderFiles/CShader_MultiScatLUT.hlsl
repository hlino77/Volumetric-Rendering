#include "Engine_Shader_Defines.hpp"

#define THREAD_GROUP_SIZE_X 16
#define THREAD_GROUP_SIZE_Y 16

#define STEP_COUNT 40

#define PLANET_RADIUS_OFFSET 0.01f
#define SQRTSAMPLECOUNT 8
#define LUTSIZE 32.0f

#define PI 3.1415926535897932384626433832795f

RWTexture2D<float4> MultiScatTexture : register(u0);
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



struct SingleScatteringResult
{
	float3 vL;						
	float3 vOpticalDepth;			
	float3 vTransmittance;			
	float3 vMultiScatAs1;

	float3 vNewMultiScatStep0Out;
	float3 vNewMultiScatStep1Out;
};

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

MediumSampleRGB SampleMediumRGB(float3 vWorldPos)
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

	tResult.vScatteringOzo = 0.0;
	tResult.vAbsorptionOzo = fDensityOzo * vAbsorbOzone;
	tResult.vExtinctionOzo = tResult.vScatteringOzo + tResult.vAbsorptionOzo;

	tResult.vScattering = tResult.vScatteringMie + tResult.vScatteringRay + tResult.vScatteringOzo;
	tResult.vAbsorption = tResult.vAbsorptionMie + tResult.vAbsorptionRay + tResult.vAbsorptionOzo;
	tResult.vExtinction = tResult.vExtinctionMie + tResult.vExtinctionRay + tResult.vExtinctionOzo;

	return tResult;
}



float raySphereIntersectNearest(float3 r0, float3 rd, float3 s0, float sR)
{
	float a = dot(rd, rd);
	float3 s0_r0 = r0 - s0;
	float b = 2.0 * dot(rd, s0_r0);
	float c = dot(s0_r0, s0_r0) - (sR * sR);
	float delta = b * b - 4.0*a*c;
	if (delta < 0.0 || a == 0.0)
	{
		return -1.0;
	}
	float sol0 = (-b - sqrt(delta)) / (2.0*a);
	float sol1 = (-b + sqrt(delta)) / (2.0*a);
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

void LutTransmittanceParamsToUv(float fViewHeight, float fViewZenithCosAngle, out float2 vUV)
{
	float fH = sqrt(max(0.0f, fAtmosphereRadius * fAtmosphereRadius - fEarthRadius * fEarthRadius));
	float fRho = sqrt(max(0.0f, fViewHeight * fViewHeight - fEarthRadius * fEarthRadius));

	float fDiscriminant = fViewHeight * fViewHeight * (fViewZenithCosAngle * fViewZenithCosAngle - 1.0) + fAtmosphereRadius * fAtmosphereRadius;
	float fD = max(0.0, (-fViewHeight * fViewZenithCosAngle + sqrt(fDiscriminant)));

	float fMin = fAtmosphereRadius - fViewHeight;
	float fMax = fRho + fH;
	float fX = (fD - fMin) / (fMax - fMin);
	float fY = fRho / fH;

	vUV = float2(fX, fY);
}

SingleScatteringResult IntegrateScatteredLuminance(
	in float2 vPixPos, in float3 vWorldPos, in float3 vWorldDir, in float3 vSunDir,
	in float fSampleCountIni)
{
	SingleScatteringResult Result = (SingleScatteringResult)0;

	float3 vEarthOrigin = float3(0.0f, 0.0f, 0.0f);
	float fBottom = raySphereIntersectNearest(vWorldPos, vWorldDir, vEarthOrigin, fEarthRadius);
	float fTop = raySphereIntersectNearest(vWorldPos, vWorldDir, vEarthOrigin, fAtmosphereRadius);
	float fMax = 0.0f;

	if (fBottom < 0.0f)
	{
		if (fTop < 0.0f)
		{
			fMax = 0.0f;
			return Result;
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

	fMax = min(fMax, 9000000.0f);

	float fSampleCount = fSampleCountIni;
	float fSampleCountFloor = fSampleCountIni;
	float fMaxFloor = fMax;
	float fDt = fMax / fSampleCount;

	const float fUniformPhase = 1.0 / (4.0 * PI);
	const float3 fWi = vSunDir;
	const float3 fWo = vWorldDir;
	float fCosTheta = dot(fWi, fWo);

	float3 vGlobalL = 1.0f;

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

		float3 vPhaseTimesScattering = Medium.vScattering * fUniformPhase;

		float fEarth = raySphereIntersectNearest(vPos, vSunDir, vEarthOrigin + PLANET_RADIUS_OFFSET * vUpVector, fEarthRadius);
		float fEarthShadow = fEarth >= 0.0f ? 0.0f : 1.0f;

		float3 vS = vGlobalL * (fEarthShadow * vTransmittanceToSun * vPhaseTimesScattering);

		float3 vMS = Medium.vScattering;
		float3 vMSint = (vMS - vMS * vSampleTransmittance) / Medium.vExtinction;
		Result.vMultiScatAs1 += vThroughput * vMSint;

		float3 vSint = (vS - vS * vSampleTransmittance) / Medium.vExtinction;	
		vL += vThroughput * vSint;
		vThroughput *= vSampleTransmittance;

		fPrev = fT;
	}


	Result.vL = vL;
	Result.vOpticalDepth = vOpticalDepth;
	Result.vTransmittance = vThroughput;
	return Result;
}


groupshared float3 MultiScatAs1SharedMem[64];
groupshared float3 LSharedMem[64];

[numthreads(1, 1, 64)]
void CSMultiScatLUT(uint3 iThreadId : SV_DispatchThreadID)
{
	float2 vPixPos = float2(iThreadId.xy) + 0.5f;
	float2 vUV = vPixPos / 32.0f;


	float fCosSunZenithAngle = vUV.x * 2.0f - 1.0f;
	float3 vSunDir = float3(0.0, sqrt(saturate(1.0 - fCosSunZenithAngle * fCosSunZenithAngle)), fCosSunZenithAngle);
	float fViewHeight = fEarthRadius + saturate(vUV.y + PLANET_RADIUS_OFFSET) * (fAtmosphereRadius - fEarthRadius - PLANET_RADIUS_OFFSET);

	float3 vWorldPos = float3(0.0f, 0.0f, fViewHeight);
	float3 vWorldDir = float3(0.0f, 0.0f, 1.0f);

	const float fSampleCountIni = 20;
	const float fDepthBufferValue = -1.0;

	const float fSphereSolidAngle = 4.0 * PI;
	const float fIsotropicPhase = 1.0 / fSphereSolidAngle;


	const float fSqrtSample = float(SQRTSAMPLECOUNT);
	float fI = 0.5f + float(iThreadId.z / SQRTSAMPLECOUNT);
	float fJ = 0.5f + float(iThreadId.z - float((iThreadId.z / SQRTSAMPLECOUNT) * SQRTSAMPLECOUNT));
	{
		float fRandA = fI / fSqrtSample;
		float fRandB = fJ / fSqrtSample;
		float fTheta = 2.0f * PI * fRandA;
		float fPhi = acos(1.0f - 2.0f * fRandB);	
		float fCosPhi = cos(fPhi);
		float fSinPhi = sin(fPhi);
		float fCosTheta = cos(fTheta);
		float fSinTheta = sin(fTheta);
		vWorldDir.x = fCosTheta * fSinPhi;
		vWorldDir.y = fSinTheta * fSinPhi;
		vWorldDir.z = fCosPhi;
		SingleScatteringResult Result = IntegrateScatteredLuminance(vPixPos, vWorldPos, vWorldDir, vSunDir, fSampleCountIni);

 		MultiScatAs1SharedMem[iThreadId.z] = Result.vMultiScatAs1 * fSphereSolidAngle / (fSqrtSample * fSqrtSample);
 		LSharedMem[iThreadId.z] = Result.vL * fSphereSolidAngle / (fSqrtSample * fSqrtSample);
	}

	

	GroupMemoryBarrierWithGroupSync();

	if (iThreadId.z < 32)
	{
		MultiScatAs1SharedMem[iThreadId.z] += MultiScatAs1SharedMem[iThreadId.z + 32];
		LSharedMem[iThreadId.z] += LSharedMem[iThreadId.z + 32];
	}
	GroupMemoryBarrierWithGroupSync();

	if (iThreadId.z < 16)
	{
		MultiScatAs1SharedMem[iThreadId.z] += MultiScatAs1SharedMem[iThreadId.z + 16];
		LSharedMem[iThreadId.z] += LSharedMem[iThreadId.z + 16];
	}
	GroupMemoryBarrierWithGroupSync();

	if (iThreadId.z < 8)
	{
		MultiScatAs1SharedMem[iThreadId.z] += MultiScatAs1SharedMem[iThreadId.z + 8];
		LSharedMem[iThreadId.z] += LSharedMem[iThreadId.z + 8];
	}
	GroupMemoryBarrierWithGroupSync();
	if (iThreadId.z < 4)
	{
		MultiScatAs1SharedMem[iThreadId.z] += MultiScatAs1SharedMem[iThreadId.z + 4];
		LSharedMem[iThreadId.z] += LSharedMem[iThreadId.z + 4];
	}
	GroupMemoryBarrierWithGroupSync();
	if (iThreadId.z < 2)
	{
		MultiScatAs1SharedMem[iThreadId.z] += MultiScatAs1SharedMem[iThreadId.z + 2];
		LSharedMem[iThreadId.z] += LSharedMem[iThreadId.z + 2];
	}
	GroupMemoryBarrierWithGroupSync();
	if (iThreadId.z < 1)
	{
		MultiScatAs1SharedMem[iThreadId.z] += MultiScatAs1SharedMem[iThreadId.z + 1];
		LSharedMem[iThreadId.z] += LSharedMem[iThreadId.z + 1];
	}
	GroupMemoryBarrierWithGroupSync();
	if (iThreadId.z > 0)
		return;

	float3 vMultiScatAs1			= MultiScatAs1SharedMem[0] * fIsotropicPhase;
	float3 vInScatteredLuminance	= LSharedMem[0] * fIsotropicPhase;

	const float3 vR = vMultiScatAs1;
	const float3 vSumOfAllMultiScatteringEventsContribution = 1.0f / (1.0 - vR);
	float3 vL = vInScatteredLuminance * vSumOfAllMultiScatteringEventsContribution;

	MultiScatTexture[iThreadId.xy] = float4(vL * fMultiScatFactor, 1.0f);
}