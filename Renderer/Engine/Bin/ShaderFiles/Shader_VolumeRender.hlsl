
#include "Engine_Shader_Defines.hpp"

matrix			g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix			g_LightViewMatrix, g_LightProjMatrix;
matrix			g_ProjMatrixInv;
matrix			g_ViewMatrixInv;

vector			g_vCamPosition;

texture3D		g_NoiseTexture;

float			g_fTest;


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


struct OBB 
{
	float3 vCenter;
	float3 vExtents;
	float3x3 vOrientation;
};

float remap(float x, float a, float b, float c, float d)
{
    return (((x - a) / (b - a)) * (d - c)) + c;
}


bool Intersect_VolumeBox(float3 vWorldPos, float3 vDir, out float fMin, out float fMax)
{
	OBB VolumeBox;


	VolumeBox.vCenter = float3(20.0f, 20.0f, 20.0f);
	VolumeBox.vExtents = float3(10.0f, 10.0f, 10.0f);

	VolumeBox.vOrientation = float3x3(1.0, 0.0, 0.0,
									  0.0, 1.0, 0.0,
									  0.0, 0.0, 1.0);

	fMin = 0.0f;
	fMax = 1000.0f;

	float3 vDel = VolumeBox.vCenter - vWorldPos;

	for (int i = 0; i < 3; ++i) 
	{
		float fE = dot(VolumeBox.vOrientation[i], vDel);
		float fD = dot(VolumeBox.vOrientation[i], vDir);

		if (abs(fD) > 0.001f)
		{
			float fT1 = (fE + VolumeBox.vExtents[i]) / fD;
			float fT2 = (fE - VolumeBox.vExtents[i]) / fD;

			if (fT1 > fT2)
			{
				float fTemp = fT1;
				fT1 = fT2;
				fT2 = fTemp;
			}

			fMin = max(fMin, fT1);
			fMax = min(fMax, fT2);

			if (fMin > fMax) 
			{
				return false;
			}
		}
		else if (-fE - VolumeBox.vExtents[i] > 0.0f || -fE + VolumeBox.vExtents[i] < 0.0f)
		{
			return false;
		}
	}

	return true;
}



PS_OUT PS_MAIN_VOLUMERENDERTEST(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	vector		vClipPos;

	vClipPos.x = In.vTexcoord.x * 2.f - 1.f;
	vClipPos.y = In.vTexcoord.y * -2.f + 1.f;
	vClipPos.z = 1.f;
	vClipPos.w = 1.f;

	vClipPos = vClipPos * 1000.0f;
	vClipPos = mul(vClipPos, g_ProjMatrixInv);

	vClipPos = mul(vClipPos, g_ViewMatrixInv);
	
	float3		vWorldPos = vClipPos.xyz;
	float3		vRayDir = normalize(vWorldPos - g_vCamPosition.xyz);
	vWorldPos = g_vCamPosition.xyz;

	Out.vColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

	float fMin, fMax;

	if (Intersect_VolumeBox(vWorldPos, vRayDir, fMin, fMax) == false)
	{
		discard;
	}

	vWorldPos += vRayDir * fMin;

	int iMaxStep = 50;
	float fStepSize = (fMax - fMin) / (float)iMaxStep;

	//float3 vSphereWorld = float3(20.0f, 20.0f, 20.0f);
	float3 vLocal = vWorldPos - float3(10.0f, 10.0f, 10.0f);
	float fRadius = 5.0f;
	
	float fDensity = 0.0f;
	
	for (int i = 0; i < iMaxStep; ++i)
	{
		float3 vTexcoord = float3(remap(vLocal.x, 0.0f, 20.0f, 0.0f, 1.0f), remap(vLocal.y, 0.0f, 20.0f, 0.0f, 1.0f), remap(vLocal.z, 0.0f, 20.0f, 0.0f, 1.0f));
		fDensity += g_NoiseTexture.Sample(LinearSampler, vTexcoord).x * fStepSize * 0.04f;
		
		
		vLocal += vRayDir * fStepSize;
		vWorldPos += vRayDir * fStepSize;

	}
	
	if (fDensity == 0.0f)
	{
		discard;
	}
	
	Out.vColor.a *= min(fDensity, 1.0f);

	return Out;
}


PS_OUT PS_MAIN_CLOUDRENDERTEST(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	
	float3 vTexcoord = float3(In.vTexcoord.x, In.vTexcoord.y, g_fTest);

	float fValue = g_NoiseTexture.Sample(LinearSampler, vTexcoord).x;
	
	Out.vColor = float4(fValue, fValue, fValue, 1.0f);

	return Out;
}

technique11 DefaultTechnique
{
	pass VolumeRender
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_VOLUMERENDERTEST();
	}

	pass CloudRender
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_CLOUDRENDERTEST();
	}
}




