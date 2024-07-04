
#include "Engine_Shader_Defines.hpp"

matrix			g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix			g_LightViewMatrix, g_LightProjMatrix;
matrix			g_ProjMatrixInv;
matrix			g_ViewMatrixInv;

vector			g_vCamPosition;


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


struct AABB {
    float3 vMin;
    float3 vMax;
};

bool Intersect_VolumeBox(float3 vWorldPos, float3 vDir, out float fMin, out float fMax)
{
	AABB VolumeBox;

	VolumeBox.vMin = float3(10.0f, 10.0f, 10.0f);
	VolumeBox.vMax = float3(30.0f, 30.0f, 30.0f);

	float3 vInvDir = 1.0f / vDir; // Inverse of ray direction

    float3 vLine1 = (VolumeBox.vMin - vWorldPos) * vInvDir;
    float3 vLine2 = (VolumeBox.vMax - vWorldPos) * vInvDir;

    float3 vMinLine = min(vLine1, vLine2);
    float3 vMaxLine = max(vLine1, vLine2);

    fMin = max(vMinLine.x, max(vMinLine.y, vMinLine.z));
    fMax = min(vMaxLine.x, min(vMaxLine.y, vMaxLine.z));

    return fMax >= max(0.0, fMin);
}



PS_OUT PS_MAIN_VOLUMERENDERTEST(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	
	//float3 vWorldPos = g_vCamPosition.xyz;

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


	float fMin, fMax;

	if (Intersect_VolumeBox(vWorldPos, vRayDir, fMin, fMax) == false)
	{
		discard;
	}

	vWorldPos += vRayDir * fMin;

	float fStepSize = 0.5f;
	int iMaxStep = (fMax - fMin) / fStepSize;
	
	float3 vSphereWorld = float3(20.0f, 20.0f, 20.0f);
	float fRadius = 5.0f;
	
	float fDensity = 0.0f;
	
	for (int i = 0; i < iMaxStep; ++i)
	{
		if (distance(vSphereWorld, vWorldPos) < fRadius)
		{
			fDensity += 0.01f;
		}
	
		fDensity += 0.01f;
		vWorldPos += vRayDir * fStepSize;
	}
	
	if (fDensity == 0.0f)
	{
		discard;
	}
	
	Out.vColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	Out.vColor.a *= min(fDensity, 1.0f);

	return Out;
}

technique11 DefaultTechnique
{
	pass Deferred
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

	
}




