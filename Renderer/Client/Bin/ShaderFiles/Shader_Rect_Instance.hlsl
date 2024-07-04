
#include "Engine_Shader_Defines.hpp"

/* ������̺�. */
matrix			g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D		g_Texture;



struct VS_IN
{
	/* �׸���� ����. m_pVB */
	float3		vPosition : POSITION;
	float2		vTexcoord : TEXCOORD0;

	/* ���º�ȯ�� ����. m_pVBInstance */
	float4		vRight : TEXCOORD1;
	float4		vUp : TEXCOORD2;
	float4		vLook : TEXCOORD3;
	float4		vTranslation : TEXCOORD4;
};

struct VS_OUT
{
	/* float4 : w���� �ݵ�� ���ܾ��ϴ� ������ w�� �佺���̽� ���� ����(z)�� �����ϱ� ���ؼ���. */
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
};


/* ���ؽ��� ���� ��ȯ�۾��� ��ģ��.  */
/* ��ȯ�۾� : ������ ��ġ�� ����, ��, ��������� ���Ѵ�. and �ʿ��� ��ȯ�� ���ؼ� �����Ӱ� �߰��ص� ������� .*/
/* ���ؽ��� ���� ������ �����Ѵ�. */
VS_OUT VS_MAIN(/* ���� */VS_IN In)
{
	VS_OUT			Out = (VS_OUT)0;

	float4x4		TransformMatrix = float4x4(In.vRight, In.vUp, In.vLook, In.vTranslation);

	vector		vPosition = mul(float4(In.vPosition, 1.f), TransformMatrix);


	/* mul : ���(���ϱⰡ ������) ����� ���ϱ⸦ �����Ѵ�. */
	matrix			matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(vPosition, matWVP);

	Out.vTexcoord = In.vTexcoord;

	return Out;	
}

/* w������ ����. ������ ������ȯ. */
/* ����Ʈ�����̽�(��������ǥ)�� ��ġ�� ��ȯ�Ѵ�. */
/* �����Ͷ����� : ������ �ѷ����� �ȼ��� ������ �����Ѵ�. */
/* �ȼ������� ���������� ����Ѵ�. */

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
};

/* �޾ƿ� �ȼ��� ������ �������� �Ͽ� ȭ�鿡 �׷��� �ȼ��� �������� ���� �����ϳ�. */
struct PS_OUT
{
	float4	vColor : SV_TARGET0;
};

/* ���޹��� �ȼ��� ������ �̿��Ͽ� �ȼ��� �������� ���� ��������. */
PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	Out.vColor = g_Texture.Sample(PointSampler, In.vTexcoord);

	if (Out.vColor.a < 0.1f)
		discard;

	return Out;
}

technique11 DefaultTechnique
{
	/* */
	pass Particle
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		/* ���� ���̴��� ���ؼ� ���� � �������� �����ϰ� � �Լ��� ȣ���Ͽ� �ش� ���̴��� �����Ǵ����� �����Ѵ�. */
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
}




