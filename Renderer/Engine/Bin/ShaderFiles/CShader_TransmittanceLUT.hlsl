#include "AtmosphereDefines.hlsl"

#define THREAD_GROUP_SIZE_X 16
#define THREAD_GROUP_SIZE_Y 16

#define STEP_COUNT 40

RWTexture2D<float4> Transmittance;

void UvToHeightAngle(out float fHeight, out float fAngle, in float2 vUV)
{
	float fX = vUV.x; //각도
	float fY = vUV.y; //높이

	float fH = sqrt(fAtmosphereRadius * fAtmosphereRadius - fEarthRadius * fEarthRadius);
	float fRho = fH * fY;
	fHeight = sqrt(fRho * fRho + fEarthRadius * fEarthRadius);

	float fMin = fAtmosphereRadius - fHeight;
	float fMax = fRho + fH;
	float fD = fMin + fX * (fMax - fMin);
	fAngle = fD == 0.0 ? 1.0f : (fH * fH - fRho * fRho - fD * fD) / (2.0 * fHeight * fD);
	fAngle = clamp(fAngle, -1.0, 1.0);
}

float3 GetTransmittance(float fHeight)
{
	float3 vRayleigh = vScatterRayleigh.xyz * exp(-fHeight / fHDensityRayleigh);
	float fMie = fExtinctionMie * exp(-fHeight / fHDensityMie);
	float3 fDensityOzo = max(0.0f, 1 - 0.5 * abs(fHeight - vOzone.x) / vOzone.y);
	float3 vOzo = vAbsorbOzone * fDensityOzo;

	return vRayleigh + fMie + vOzo;
}



[numthreads(THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1)]
void CSTransLUT(int3 iThreadIdx : SV_DispatchThreadID)
{
	int iWidth, iHeight;
	Transmittance.GetDimensions(iWidth, iHeight);
	if (iThreadIdx.x >= iWidth || iThreadIdx.y >= iHeight)
		return;

	float2 vUV = float2(iThreadIdx.x + 0.5f, iThreadIdx.y + 0.5f) / float2(iWidth, iHeight);
	float fViewHeight;
	float fViewZenithCosAngle;
	UvToHeightAngle(fViewHeight, fViewZenithCosAngle, vUV);

 	float3 vWorldPos = float3(0.0f, fViewHeight, 0.0f);
 	float3 vWorldDir = float3(0.0f,   fViewZenithCosAngle, sqrt(1.0 - fViewZenithCosAngle * fViewZenithCosAngle));

	float3 fEarthOrigin = float3(0.0f, 0.0f, 0.0f);
	float fBottom = raySphereIntersectNearest(vWorldPos, vWorldDir, fEarthOrigin, fEarthRadius);
	float fTop = raySphereIntersectNearest(vWorldPos, vWorldDir, fEarthOrigin, fAtmosphereRadius);
	float fMax = 0.0f;

	if (fBottom < 0.0f)
	{
		if (fTop < 0.0f)
		{
			fMax = 0.0f;
			Transmittance[iThreadIdx.xy] = float4(0.0f, 0.0f, 0.0f, 1);
			return;
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
	fMax = min(fMax, 90000000.f);

	float fDt = fMax / STEP_COUNT;
	float fT = 0.0f;
	const float fSampleSegmentT = 0.3f;

 	float3 vSum = 0.0f;
 	for (int i = 0; i < STEP_COUNT; ++i)
 	{
		float fNewT = fMax * (i + fSampleSegmentT) / STEP_COUNT;
		fDt = fNewT - fT;
		fT = fNewT;

		float3 vPos = vWorldPos + fT * vWorldDir;

 		float fCurrHeight = length(vPos) - fEarthRadius;
 		float3 vResult = GetTransmittance(fCurrHeight) * fDt;
 		vSum += vResult;
 	}

	float3 vResult = exp(-vSum);
	Transmittance[iThreadIdx.xy] = float4(vResult, 1);
}