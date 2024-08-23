#include "Engine_Shader_Defines.hpp"

#define PI 3.1415926535897932384626433832795f
#define PLANET_RADIUS_OFFSET 0.01f
#define DEPTHCOUNT 32.0f
#define M_PER_SLICE 4000.0f



matrix			g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix			g_ProjMatrixInv;
matrix			g_ViewMatrixInv;
matrix			g_PrevViewProj;

vector			g_vCamPosition;
vector			g_vLightDir;
vector			g_vLightPos;

texture2D		g_TransLUTTexture;
texture2D		g_SkyViewLUTTexture;
texture2D		g_MultiScatLUTTexture;
texture3D		g_AerialLUTTexture;

texture2D		g_DepthTexture;
texture2D		g_CloudTexture;
texture2D		g_AtmosphereTexture;

float3			g_vSunPos;
float2			g_vRayMarchMinMaxSPP = float2(4.0f, 14.0f);

uint				g_iWinSizeX;
uint				g_iWinSizeY;

float			g_fTest;
bool			g_bAerial;

float3			g_vLowScatter = float3(1.0f, 0.7f, 0.5f);


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


float Henyey_Greenstein_Phase(float fCos, float fG)
{
	float fG2 = fG * fG;
	return ((1.0f - fG2) / pow(1.0f + fG2 - 2.0f * fG * fCos, 1.5f)) / (4.0f * 3.1415f);
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


void UvToSkyViewLutParams(out float fViewZenithCosAngle, out float fLightViewCosAngle, in float fViewHeight, in float2 vUV)
{
	float fVhorizon = sqrt(fViewHeight * fViewHeight - fEarthRadius * fEarthRadius);
	float fCosBeta = fVhorizon / fViewHeight;				
	float fBeta = acos(fCosBeta);
	float fZenithHorizonAngle = PI - fBeta;

	if (vUV.y < 0.5f)
	{
		float fCoord = 2.0* vUV.y;
		fCoord = 1.0 - fCoord;
		fCoord *= fCoord;

		fCoord = 1.0 - fCoord;
		fViewZenithCosAngle = cos(fZenithHorizonAngle * fCoord);
	}
	else
	{
		float fCoord = vUV.y * 2.0f - 1.0f;
		fCoord *= fCoord;

		fViewZenithCosAngle = cos(fZenithHorizonAngle + fBeta * fCoord);
	}

	float fCoord = vUV.x;
	fCoord *= fCoord;
	fLightViewCosAngle = -(fCoord * 2.0f - 1.0f);
}


bool MoveToTopAtmosphere(inout float3 vWorldPos, in float3 vWorldDir, in float fAtmosphereTopRadius)
{
	float fViewHeight = length(vWorldPos);
	if (fViewHeight > fAtmosphereTopRadius)
	{
		float fTop = raySphereIntersectNearest(vWorldPos, vWorldDir, float3(0.0f, 0.0f, 0.0f), fAtmosphereTopRadius);
		if (fTop >= 0.0f)
		{
			float3 vUpVector = vWorldPos / fViewHeight;
			float3 vUpOffset = vUpVector * -PLANET_RADIUS_OFFSET;
			vWorldPos = vWorldPos + vWorldDir * fTop + vUpOffset;
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

void SkyViewLutParamsToUv(bool bIntersectGround, float fViewZenithCosAngle, float fLightViewCosAngle, float fViewHeight, out float2 vUV)
{
	float fVhorizon = sqrt(fViewHeight * fViewHeight - fEarthRadius * fEarthRadius);
	float fCosBeta = fVhorizon / fViewHeight;
	float fBeta = acos(fCosBeta);
	float fZenithHorizonAngle = PI - fBeta;

	if (!bIntersectGround)
	{
		float fCoord = acos(fViewZenithCosAngle) / fZenithHorizonAngle;
		fCoord = 1.0 - fCoord;
		fCoord = sqrt(fCoord);
		fCoord = 1.0 - fCoord;
		vUV.y = fCoord * 0.5f;
	}
	else
	{
		float fCoord = (acos(fViewZenithCosAngle) - fZenithHorizonAngle) / fBeta;
		fCoord = sqrt(fCoord);
		vUV.y = fCoord * 0.5f + 0.5f;
	}

	{
		float fCoord = -fLightViewCosAngle * 0.5f + 0.5f;
		fCoord = sqrt(fCoord);
		vUV.x = fCoord;
	}
}


float3 GetSunLuminance(float3 vWorldPos, float3 vWorldDir, float3 vLightDir)
{
	if (dot(vWorldDir, vLightDir) > 0.99997f)
	{
		float fT = raySphereIntersectNearest(vWorldPos, vWorldDir, float3(0.0f, 0.0f, 0.0f), fEarthRadius);
		if (fT < 0.0f)
		{
			const float3 vSunLuminance = fSunIlluminance;
			return vSunLuminance;
		}
	}
	return 0;
}

float3 GetMultipleScattering(float3 vWorlPos, float fViewZenithCosAngle)
{
	float2 vUV = saturate(float2(fViewZenithCosAngle * 0.5f + 0.5f, (length(vWorlPos) - fEarthRadius) / (fAtmosphereRadius - fEarthRadius)));

	float3 vMultiScatteredLuminance = g_MultiScatLUTTexture.SampleLevel(LinearClampSampler, vUV, 0).rgb;
	return vMultiScatteredLuminance;
}