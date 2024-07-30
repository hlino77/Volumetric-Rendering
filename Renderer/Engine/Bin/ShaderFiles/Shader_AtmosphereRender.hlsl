
#include "Engine_Shader_Defines.hpp"

#define PI 3.1415926535897932384626433832795f
#define PLANET_RADIUS_OFFSET 0.01f

matrix			g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix			g_ProjMatrixInv;
matrix			g_ViewMatrixInv;
matrix			g_PrevViewProj;

vector			g_vCamPosition;

texture2D		g_TransLUTTexture;

float3			g_vLightDir = float3(0.0f, 0.9f, 0.4f);
float			g_fSunIlluminance = 1.0f;
float2			g_vRayMarchMinMaxSPP = float2(4.0f, 14.0f);

uint				g_iWinSizeX;
uint				g_iWinSizeY;


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

	float	fPadding;

	float4	vAbsorbOzone;
 	float4	vOzone;
}

struct VS_IN
{
	float3		vPosition : POSITION;
	float2		vTexcoord : TEXCOORD0;
};

struct VS_OUT
{	
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
};



VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT			Out = (VS_OUT)0;
	
	matrix			matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

    Out.vTexcoord = In.vTexcoord;
	Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);

	return Out;
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
};


struct PS_OUT
{
	float4	vColor : SV_TARGET0;
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

	tResult.vScatteringOzo = 0.0;
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

	float fDiscriminant = fViewHeight * fViewHeight * (fViewZenithCosAngle * fViewZenithCosAngle - 1.0) + fAtmosphereRadius * fAtmosphereRadius;
	float fD = max(0.0, (-fViewHeight * fViewZenithCosAngle + sqrt(fDiscriminant)));

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

SingleScatteringResult IntegrateScatteredLuminance(
	in float2 vPixPos, in float3 vWorldPos, in float3 vWorldDir, in float3 vSunDir,
	in bool bGground, in float fSampleCountIni, in float fDepthBufferValue,
	in float fMaxMax = 9000000.0f)
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

	fSampleCount = lerp(g_vRayMarchMinMaxSPP.x, g_vRayMarchMinMaxSPP.y, saturate(fMax * 0.01f));
	fSampleCountFloor = floor(fSampleCount);
	fMaxFloor = fMax * fSampleCountFloor / fSampleCount;

	float fDt = fMax / fSampleCount;

	const float3 fWi = vSunDir;
	const float3 fWo = vWorldDir;
	float fCosTheta = dot(fWi, fWo);
	float fMiePhaseValue = Cornette_Shanks_Phase(fPhaseMieG, -fCosTheta);
	float fRayleighPhaseValue = RayleighPhase(fCosTheta);

	float3 vGlobalL = g_fSunIlluminance;

	float3 vL = 0.0f;
	float3 vThroughput = 1.0;
	float3 vOpticalDepth = 0.0;
	float fT = 0.0f;
	float fPrev = 0.0;
	const float fSampleSegmentT = 0.3f;
	for (float i = 0.0f; i < fSampleCount; i += 1.0f)
	{
		{
			float fT0 = (i) / fSampleCountFloor;
			float fT1 = (i + 1.0f) / fSampleCountFloor;
			fT0 = fT0 * fT0;
			fT1 = fT1 * fT1;
			fT0 = fMaxFloor * fT0;
			if (fT1 > 1.0)
			{
				fT1 = fMax;
			}
			else
			{
				fT1 = fMaxFloor * fT1;
			}
			fT = fT0 + (fT1 - fT0) * fSampleSegmentT;
			fDt = fT1 - fT0;
		}

		float3 vPos = vWorldPos + fT * vWorldDir;

		MediumSampleRGB tMedium = SampleMediumRGB(vPos);
		const float3 vSampleOpticalDepth = tMedium.vExtinction * fDt;
		const float3 vSampleTransmittance = exp(-vSampleOpticalDepth);
		vOpticalDepth += vSampleOpticalDepth;

		float fHeight = length(vPos);
		const float3 vUpVector = vPos / fHeight;
		float fSunZenithCosAngle = dot(vSunDir, vUpVector);
		float2 vUV;
		LutTransmittanceParamsToUv(fHeight, fSunZenithCosAngle, vUV);
		float3 vTransmittanceToSun = g_TransLUTTexture.SampleLevel(LinearClampSampler, vUV, 0).rgb;

		float3 vPhaseTimesScattering = tMedium.vScatteringMie * fMiePhaseValue + tMedium.vScatteringRay * fRayleighPhaseValue;

		float tEarth = raySphereIntersectNearest(vPos, vSunDir, vEarthOrigin + PLANET_RADIUS_OFFSET * vUpVector, fEarthRadius);
		float fEarthShadow = tEarth >= 0.0f ? 0.0f : 1.0f;

		float fShadow = 1.0f;

		float3 vS = vGlobalL * (fEarthShadow * fShadow * vTransmittanceToSun * vPhaseTimesScattering);

		float3 vSint = (vS - vS * vSampleTransmittance) / tMedium.vExtinction;	
		vL += vThroughput * vSint;					
		vThroughput *= vSampleTransmittance;

		fPrev = fT;
	}

	tResult.vL = vL;
	tResult.vOpticalDepth = vOpticalDepth;
	tResult.vTransmittance = vThroughput;
	return tResult;
}


PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	int iX = In.vTexcoord.x * g_iWinSizeX;
	int iY = In.vTexcoord.y * g_iWinSizeY;

	vector		vClipPos;

	vClipPos.x = In.vTexcoord.x * 2.f - 1.f;
	vClipPos.y = In.vTexcoord.y * -2.f + 1.f;
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
	 
	float2 vUV = In.vTexcoord;

	float fViewHeight = length(vWorldPos);

	float fViewZenithCosAngle;
	float fLightViewCosAngle;
	UvToSkyViewLutParams(fViewZenithCosAngle, fLightViewCosAngle, fViewHeight, vUV);

	float3 vSunDirection = normalize(g_vLightDir);

	float3 vSunDir;
	{
		float3 vUpVector = vWorldPos / fViewHeight;
		float fSunZenithCosAngle = dot(vUpVector, vSunDirection);
		vSunDir = normalize(float3(sqrt(1.0 - fSunZenithCosAngle * fSunZenithCosAngle), 0.0, fSunZenithCosAngle));
	}

	vWorldPos = float3(0.0f, 0.0f, fViewHeight);

	float fViewZenithSinAngle = sqrt(1 - fViewZenithCosAngle * fViewZenithCosAngle);
	vWorldDir = float3(
		fViewZenithSinAngle * fLightViewCosAngle,
		fViewZenithSinAngle * sqrt(1.0 - fLightViewCosAngle * fLightViewCosAngle),
		fViewZenithCosAngle);

	if (!MoveToTopAtmosphere(vWorldPos, vWorldDir, fAtmosphereRadius))
	{
		Out.vColor = float4(0.0f, 0.0f, 0.0f, 1);
		return Out;
	}

	float2 vPixPos = In.vTexcoord * float2(g_iWinSizeX, g_iWinSizeY);
	const bool bGround = false;
	const float fSampleCountIni = 30;
	const float fDepthBufferValue = -1.0;
	SingleScatteringResult tResult = IntegrateScatteredLuminance(vPixPos, vWorldPos, vWorldDir, vSunDir, bGround, fSampleCountIni, fDepthBufferValue);

	float3 vL = tResult.vL;

	Out.vColor =  float4(vL, 1) * 5.0f;
	return Out;
}




PS_OUT PS_LUTTEST(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

 	In.vTexcoord.y *= 2.0f;
 
 	if (In.vTexcoord.y > 1.0f)
 	{
 		discard;
 	}
	Out.vColor = g_TransLUTTexture.Sample(PointSampler, In.vTexcoord);
	return Out;
}


technique11 DefaultTechnique
{
	pass AtmosphereRender
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}


	pass LUTTest
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_LUTTEST();
	}
}




