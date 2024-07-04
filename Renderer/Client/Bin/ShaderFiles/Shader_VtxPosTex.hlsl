
#include "Engine_Shader_Defines.hpp"

/* 상수테이블. */
matrix			g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D		g_Texture;
texture2D		g_Textures[2];
texture2D		g_DepthTexture;

struct VS_IN
{
	float3		vPosition : POSITION;
	float2		vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
	/* float4 : w값을 반드시 남겨야하는 이유는 w에 뷰스페이스 상의 깊이(z)를 보관하기 위해서다. */
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
};


/* 버텍스에 대한 변환작업을 거친다.  */
/* 변환작업 : 정점의 위치에 월드, 뷰, 투영행렬을 곱한다. and 필요한 변환에 대해서 자유롭게 추가해도 상관없다 .*/
/* 버텍스의 구성 정보를 변경한다. */
VS_OUT VS_MAIN(/* 정점 */VS_IN In)
{
	VS_OUT			Out = (VS_OUT)0;

	/* mul : 모든(곱하기가 가능한) 행렬의 곱하기를 수행한다. */
	matrix			matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);

	Out.vTexcoord = In.vTexcoord;

	return Out;	
}

struct VS_OUT_EFFECT
{
	/* float4 : w값을 반드시 남겨야하는 이유는 w에 뷰스페이스 상의 깊이(z)를 보관하기 위해서다. */
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
	float4		vProjPos : TEXCOORD1;
};

VS_OUT_EFFECT VS_MAIN_EFFECT(VS_IN In)
{
	VS_OUT_EFFECT			Out = (VS_OUT_EFFECT)0;

	/* mul : 모든(곱하기가 가능한) 행렬의 곱하기를 수행한다. */
	matrix			matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
	Out.vTexcoord = In.vTexcoord;
	Out.vProjPos = Out.vPosition;

	return Out;
}

/* w나누기 연산. 진정한 투영변환. */
/* 뷰포트스페이스(윈도우좌표)로 위치를 변환한다. */
/* 래스터라이즈 : 정점에 둘러쌓인 픽셀의 정보를 생성한다. */
/* 픽셀정보는 정점정보에 기반한다. */

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
};

/* 받아온 픽셀의 정보를 바탕으로 하여 화면에 그려질 픽셀의 최종적인 색을 결정하낟. */
struct PS_OUT
{
	float4	vColor : SV_TARGET0;
};

/* 전달받은 픽셀의 정보를 이용하여 픽셀의 최종적인 색을 결정하자. */
PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	// Out.vColor = tex2D(DefaultSampler, In.vTexcoord);
	/*g_Texture에서부터 PointSampler방식으로  In.vTexcoord위치에 해당하는 색을 샘플링(가져와)해와. */
	// Out.vColor = g_Texture.Sample(PointSampler, In.vTexcoord);
	// Out.vColor = float4(1.f, 0.f, 0.f, 1.f);

	vector		vSourColor = g_Textures[0].Sample(LinearSampler, In.vTexcoord);
	vector		vDestColor = g_Textures[1].Sample(LinearSampler, In.vTexcoord);

	Out.vColor = vSourColor + vDestColor;

	return Out;
}

struct PS_IN_EFFECT
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
	float4		vProjPos : TEXCOORD1;
};

PS_OUT PS_MAIN_EFFECT(PS_IN_EFFECT In)
{
	PS_OUT			Out = (PS_OUT)0;

	
	Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);

	float2		vDepthUV;

	vDepthUV.x = In.vProjPos.x / In.vProjPos.w * 0.5f + 0.5f;
	vDepthUV.y = In.vProjPos.y / In.vProjPos.w * -0.5f + 0.5f;

	vector		vDepthDesc = g_DepthTexture.Sample(PointSampler, vDepthUV);

	float		fOldViewZ = vDepthDesc.y * 1000.f;

	Out.vColor.a = Out.vColor.a  * saturate(fOldViewZ - In.vProjPos.w) * 1.5f;

	return Out;
}

technique11 DefaultTechnique
{
	/* */
	pass UI
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		/* 여러 셰이더에 대해서 각각 어떤 버젼으로 빌드하고 어떤 함수를 호출하여 해당 셰이더가 구동되는지를 설정한다. */
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

	pass Effect
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		/* 여러 셰이더에 대해서 각각 어떤 버젼으로 빌드하고 어떤 함수를 호출하여 해당 셰이더가 구동되는지를 설정한다. */
		VertexShader = compile vs_5_0 VS_MAIN_EFFECT();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_EFFECT();
	}
}




