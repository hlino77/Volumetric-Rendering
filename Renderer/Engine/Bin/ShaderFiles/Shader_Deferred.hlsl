
#include "Engine_Shader_Defines.hpp"

#define PI 3.1415926535897932384626433832795f

matrix			g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix			g_LightViewMatrix, g_LightProjMatrix;
matrix			g_ProjMatrixInv;
matrix			g_ViewMatrixInv;

vector			g_vCamPosition;
vector			g_vLightDir;
vector			g_vLightPos;

vector			g_vLightDiffuse;
vector			g_vLightAmbient;
vector			g_vLightSpecular;

vector			g_vMtrlAmbient = vector(0.5f, 0.5f, 0.5f, 1.f);
vector			g_vMtrlSpecular = vector(1.f, 1.f, 1.f, 1.f);


texture2D		g_NormalTexture;
texture2D		g_DiffuseTexture;
texture2D		g_DepthTexture;
texture2D		g_SkyTexture;
texture2D		g_DefferedTexture;

texture2D		g_ShadeTexture;
texture2D		g_SpecularTexture;
texture2D		g_LightDepthTexture;

texture2D		g_TransLUTTexture;
texture2D		g_MultiScatLUTTexture;

texture2D		g_Texture;


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

float3 GetMultipleScattering(float3 vWorlPos, float fViewZenithCosAngle)
{
	float2 vUV = saturate(float2(fViewZenithCosAngle * 0.5f + 0.5f, (length(vWorlPos) - fEarthRadius) / (fAtmosphereRadius - fEarthRadius)));

	float3 vMultiScatteredLuminance = g_MultiScatLUTTexture.SampleLevel(LinearClampSampler, vUV, 0).rgb;
	return vMultiScatteredLuminance;
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

VS_OUT VS_MAIN(/* 정점 */VS_IN In)
{
	VS_OUT			Out = (VS_OUT)0;
	
	matrix			matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);

	Out.vTexcoord = In.vTexcoord;

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


PS_OUT PS_MAIN_DEBUG(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);
	
	return Out;
}

struct PS_OUT_LIGHT
{
	float4	vShade : SV_TARGET0;	
	float4	vSpecular : SV_TARGET1;
};

PS_OUT_LIGHT PS_MAIN_DIRECTIONAL(PS_IN In)
{
	PS_OUT_LIGHT		Out = (PS_OUT_LIGHT)0;

	vector		vNormalDesc = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
	vector		vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
	float		fViewZ = vDepthDesc.y * 1000.f;

	vector		vNormal = vector(vNormalDesc.xyz * 2.f - 1.f, 0.f);

	Out.vShade = g_vLightDiffuse * (saturate(dot(normalize(g_vLightDir) * -1.f, vNormal)) + (g_vLightAmbient * g_vMtrlAmbient));

	vector		vReflect = reflect(normalize(g_vLightDir), vNormal);

	vector		vWorldPos;

	/* 투영스페이스 상의 위치를 구한다. */
	vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
	vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
	vWorldPos.z = vDepthDesc.x;
	vWorldPos.w = 1.f;

	/* 뷰스페이스 상의 위치를 구한다. */
	vWorldPos = vWorldPos * fViewZ;
	vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* 월드까지 가자. */
	vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

	vector		vLook = vWorldPos - g_vCamPosition;

	//Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * pow(saturate(dot(normalize(vLook) * -1.f, normalize(vReflect))), 20.f);
		

	return Out;
}

PS_OUT_LIGHT PS_MAIN_POINT(PS_IN In)
{
	PS_OUT_LIGHT		Out = (PS_OUT_LIGHT)0;

	vector		vNormalDesc = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
	vector		vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
	float		fViewZ = vDepthDesc.y * 1000.f;

	vector		vNormal = vector(vNormalDesc.xyz * 2.f - 1.f, 0.f);

	vector		vWorldPos;

	/* 투영스페이스 상의 위치를 구한다. */
	vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
	vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
	vWorldPos.z = vDepthDesc.x;
	vWorldPos.w = 1.f;

	/* 뷰스페이스 상의 위치를 구한다. */
	vWorldPos = vWorldPos * fViewZ;
	vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* 월드까지 가자. */
	vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

	vector		vLightDir = vWorldPos - g_vLightPos;
	float		fDistance = length(vLightDir);

	Out.vShade = g_vLightDiffuse * (saturate(dot(normalize(vLightDir) * -1.f, vNormal)) + (g_vLightAmbient * g_vMtrlAmbient));

	vector		vReflect = reflect(normalize(vLightDir), vNormal);

	vector		vLook = vWorldPos - g_vCamPosition;

	//Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * pow(saturate(dot(normalize(vLook) * -1.f, normalize(vReflect))), 20.f);

	return Out;
}


PS_OUT_LIGHT PS_MAIN_SUN(PS_IN In)
{
	PS_OUT_LIGHT		Out = (PS_OUT_LIGHT)0;

	vector		vNormalDesc = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
	vector		vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
	float		fViewZ = vDepthDesc.y * 1000.f;

	if (vDepthDesc.x == 1.0f)
	{
		return Out;
	}

	vector		vNormal = vector(vNormalDesc.xyz * 2.f - 1.f, 0.f);

	vector		vClipPos;

	vClipPos.x = In.vTexcoord.x * 2.f - 1.f;
	vClipPos.y = In.vTexcoord.y * -2.f + 1.f;
	vClipPos.z = vDepthDesc.x;
	vClipPos.w = 1.f;

	vClipPos = vClipPos * fViewZ;
	vClipPos = mul(vClipPos, g_ProjMatrixInv);

	vClipPos = mul(vClipPos, g_ViewMatrixInv);

	float3 vWorldPos = vClipPos.xyz + float3(0.0f, fEarthRadius, 0.0f);
	float3	vLightDir = normalize(g_vLightPos - vWorldPos);

	float fViewHeight = length(vWorldPos);
	const float3 vUpVector = vWorldPos / fViewHeight;
	float fSunZenithCosAngle = dot(vLightDir, vUpVector);
	float2 vUV;
	LutTransmittanceParamsToUv(fViewHeight, fSunZenithCosAngle, vUV);
	const float3 vTrans = g_TransLUTTexture.SampleLevel(LinearClampSampler, vUV, 0).rgb;


	float3 fMultiScatteredLuminance = GetMultipleScattering(vWorldPos, fSunZenithCosAngle);

	float fLamBRDF = saturate(dot(vLightDir, vNormal)) / PI;
	float3 vColor = (fLamBRDF * vTrans + fMultiScatteredLuminance) * fSunIlluminance;

	Out.vShade = float4(vColor, 1.0f);

	return Out;
}

PS_OUT PS_MAIN_DEFERRED(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	vector		vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
	float		fViewZ = vDepthDesc.y * 1000.f;

	vector		vWorldPos;

	vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
	vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
	vWorldPos.z = vDepthDesc.x;
	vWorldPos.w = 1.f;

	vWorldPos = vWorldPos * fViewZ;
	vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

	vector	vPosition = mul(vWorldPos, g_LightViewMatrix);
	vPosition = mul(vPosition, g_LightProjMatrix);


	float2	vUV;
	vUV.x = (vPosition.x / vPosition.w) * 0.5f + 0.5f;
	vUV.y = (vPosition.y / vPosition.w) * -0.5f + 0.5f;

	vector		vLightDepthDesc = g_LightDepthTexture.Sample(PointSampler, vUV);

	float		fOldZ = vLightDepthDesc.x * 1000.f;
	

	vector		vDiffuse = g_DiffuseTexture.Sample(PointSampler, In.vTexcoord);
	if (vDiffuse.a == 0.f)
		discard;


	vector		vShade = g_ShadeTexture.Sample(LinearSampler, In.vTexcoord);

	Out.vColor = (vDiffuse * vShade);

	return Out;
}


PS_OUT PS_MAIN_DEFERRED_VOLUMERENDER(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	
	Out.vColor = g_SkyTexture.Sample(LinearSampler, In.vTexcoord);

	return Out;
}

PS_OUT PS_MAIN_DEFERRED_TOMEMAP(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	
	Out.vColor = g_DefferedTexture.Sample(PointSampler, In.vTexcoord);


	float3 vWhitePoint = float3(1.08241f, 0.96756f, 0.95003f);
	float fExposure = 0.0001f;
	Out.vColor = float4(pow((float3) 1.0f - exp(-Out.vColor.rgb / vWhitePoint * fExposure), (float3)(1.0f / 2.2f)), 1.0f);

	return Out;
}

technique11 DefaultTechnique
{
	pass Target_Debug
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);
		
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_DEBUG();
	}

	pass Light_Directional
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL();
	}

	pass Light_Point
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_POINT();
	}

	pass Deferred
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_DEFERRED();
	}
	
	pass Deferred_VolumeRender
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_DEFERRED_VOLUMERENDER();
	}

	pass Light_Sun
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_SUN();
	}


	pass Deferred_ToneMap
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_DEFERRED_TOMEMAP();
	}
	
}




