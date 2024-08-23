#include "AtmosphereDefines.hlsl"

#define PI 3.1415926535897932384626433832795f

texture2D		g_NormalTexture;
texture2D		g_DiffuseTexture;
texture2D		g_SkyTexture;
texture2D		g_DefferedTexture;

texture2D		g_ShadeTexture;
texture2D		g_SpecularTexture;


texture2D		g_Texture;



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


VS_OUT VS_MAIN(/* Á¤Á¡ */VS_IN In)
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

	return Out;
}

PS_OUT_LIGHT PS_MAIN_POINT(PS_IN In)
{
	PS_OUT_LIGHT		Out = (PS_OUT_LIGHT)0;

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

	vector		vDiffuse = g_DiffuseTexture.Sample(PointSampler, In.vTexcoord);
	if (vDiffuse.a == 0.f)
		discard;

	vector		vShade = g_ShadeTexture.Sample(LinearSampler, In.vTexcoord);
	Out.vColor = (vDiffuse * vShade);

	return Out;
}


PS_OUT PS_MAIN_DEFERRED_ATMOSPHERE(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	
	Out.vColor = g_SkyTexture.Sample(LinearSampler, In.vTexcoord);

	return Out;
}

PS_OUT PS_MAIN_DEFERRED_CLOUD(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	float4 vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);

	if (vDepthDesc.x == 1.0f)
	{
		Out.vColor = g_SkyTexture.Sample(LinearSampler, In.vTexcoord);
	}

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
	
	pass Deferred_Atmosphere
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_DEFERRED_ATMOSPHERE();
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

		pass Deferred_Cloud
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_DEFERRED_CLOUD();
	}
	
}




