
#include "Engine_Shader_Defines.hpp"

/* ������̺�. */
matrix			g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D		g_DiffuseTexture[2];
texture2D		g_MaskTexture;

texture2D		g_BrushTexture;

vector			g_vBrushPos = vector(35.f, 0.f, 35.f, 1.f);
float			g_fBrushRange = 15.0f;

struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
	/* float4 : w���� �ݵ�� ���ܾ��ϴ� ������ w�� �佺���̽� ���� ����(z)�� �����ϱ� ���ؼ���. */
	float4		vPosition : SV_POSITION;
	float4		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
	float4		vWorldPos : TEXCOORD1;
	float4		vProjPos : TEXCOORD2;
};


/* ���ؽ��� ���� ��ȯ�۾��� ��ģ��.  */
/* ��ȯ�۾� : ������ ��ġ�� ����, ��, ��������� ���Ѵ�. and �ʿ��� ��ȯ�� ���ؼ� �����Ӱ� �߰��ص� ������� .*/
/* ���ؽ��� ���� ������ �����Ѵ�. */
VS_OUT VS_MAIN(/* ���� */VS_IN In)
{
	VS_OUT			Out = (VS_OUT)0;

	/* mul : ���(���ϱⰡ ������) ����� ���ϱ⸦ �����Ѵ�. */
	matrix			matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
	Out.vTexcoord = In.vTexcoord;
	Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
	Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
	Out.vProjPos = Out.vPosition;

	return Out;	
}

/* w������ ����. ������ ������ȯ. */
/* ����Ʈ�����̽�(��������ǥ)�� ��ġ�� ��ȯ�Ѵ�. */
/* �����Ͷ����� : ������ �ѷ����� �ȼ��� ������ �����Ѵ�. */
/* �ȼ������� ���������� ����Ѵ�. */

struct PS_IN
{
	/* Viewport */
	float4		vPosition : SV_POSITION;
	float4		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
	float4		vWorldPos : TEXCOORD1;
	float4		vProjPos : TEXCOORD2;
};

/* �޾ƿ� �ȼ��� ������ �������� �Ͽ� ȭ�鿡 �׷��� �ȼ��� �������� ���� �����ϳ�. */
struct PS_OUT
{
	float4	vDiffuse : SV_TARGET0;
	float4	vNormal : SV_TARGET1;
	float4	vDepth : SV_TARGET2;
};

float4 Compute_TerrainPixelColor(PS_IN In)
{
	vector	vSourDiffuse = g_DiffuseTexture[0].Sample(LinearSampler, In.vTexcoord * 30.f);
	vector	vDestDiffuse = g_DiffuseTexture[1].Sample(LinearSampler, In.vTexcoord * 30.f);
	vector	vMaskColor = g_MaskTexture.Sample(LinearSampler, In.vTexcoord);

	return vDestDiffuse * vMaskColor + vSourDiffuse * (1.f - vMaskColor);
}

/* ���޹��� �ȼ��� ������ �̿��Ͽ� �ȼ��� �������� ���� ��������. */
PS_OUT PS_MAIN_DIRECTIONAL(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	vector	vMtrlDiffuse = Compute_TerrainPixelColor(In);

	Out.vDiffuse = vMtrlDiffuse;

	/* -1 ~ 1 *//* * 0.5f + 0.5f*/
	/*  0 ~ 1 */
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.f, 0.f);

	return Out;
}


technique11 DefaultTechnique
{
	/* */
	pass Terrain_Directional
	{
		// SetRasterizerState(RS_Wireframe);
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);
		/* ���� ���̴��� ���ؼ� ���� � �������� �����ϰ� � �Լ��� ȣ���Ͽ� �ش� ���̴��� �����Ǵ����� �����Ѵ�. */
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL();
	}
}




