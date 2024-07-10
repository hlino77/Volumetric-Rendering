
#include "Engine_Shader_Defines.hpp"

matrix			g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix			g_LightViewMatrix, g_LightProjMatrix;
matrix			g_ProjMatrixInv;
matrix			g_ViewMatrixInv;

vector			g_vCamPosition;

texture3D		g_NoiseTexture;
texture2D		g_BlueNoiseTexture;



//Test

float3			g_vLightDir = float3(1.0f, -1.0f, 1.0f);
float			g_fOffset = 0.0f;
float			g_fStepLength = 5.0f;
float			g_fSunStepLength = 10.0f;
int				g_iMaxStep = 100;
int				g_iSunStep = 10;

float3			g_vBoxMin = float3(0.0f, 100.0f, 0.0f);
float3			g_vBoxMax = float3(500.0f, 200.0f, 500.0f);

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

float remap(float fValue, float fIn1, float fIn2, float fOut1, float fOut2)
{
    return fOut1 + (fValue - fIn1) * (fOut2 - fOut1) / (fIn2 - fIn1);
}


bool CheckRayYRange(float3 vPos, float3 vDir, float fYMin, float fYMax, out float3 vStart, out float3 vEnd)
{
   if (vPos.y >= fYMin && vPos.y <= fYMax)
    {
        // 시작점이 범위 내에 있는 경우
        vStart = vPos;

        // t_max 값을 계산하여 끝점을 구함
        float fTMax = (fYMax - vPos.y) / vDir.y;
		vEnd = vPos + fTMax * vDir;
        return true;
    }
    else
    {
        // 시작점이 범위 밖에 있는 경우
        float t1 = (fYMin - vPos.y) / vDir.y;
        float t2 = (fYMax - vPos.y) / vDir.y;

        if (t1 > t2)
        {
            float temp = t1;
            t1 = t2;
            t2 = temp;
        }

        if (t2 >= 0)
        {
            // t1이 음수인 경우를 처리 (시작점이 범위 아래쪽에 있고, 레이가 위로 향하는 경우)
            if (t1 < 0)
            {
                t1 = 0;
            }

            vStart = vPos + t1 * vDir;
            vEnd = vPos + t2 * vDir;
            return true;
        }
    }

    return false;
}


float3 Compute_Texcoord(float3 vWorldPos)
{
	float3 vTexcoord = float3(0.0f, 0.0f, 0.0f);

	vTexcoord.x = fmod(vWorldPos.x, 100.0f);
	vTexcoord.y = fmod(vWorldPos.y, 50.0f);
	vTexcoord.z = fmod(vWorldPos.z, 100.0f);

	if (vTexcoord.x < 0.0f)
	{
		vTexcoord.x = 100.0f - vTexcoord.x;
	}
	if (vTexcoord.y < 0.0f)
	{
		vTexcoord.y = 50.0f - vTexcoord.y;
	}
	if (vTexcoord.z < 0.0f)
	{
		vTexcoord.z = 100.0f - vTexcoord.z;
	}
	
	vTexcoord.x = remap(vTexcoord.x, 0.0f, 100.0f, 0.0f, 1.0f);
	vTexcoord.y = remap(vTexcoord.y, 0.0f, 100.0f, 0.0f, 1.0f);
	vTexcoord.z = remap(vTexcoord.z, 0.0f, 100.0f, 0.0f, 1.0f);

	return vTexcoord;
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

	float3 vStart, vEnd;
	if (CheckRayYRange(vWorldPos, vRayDir, 100.0f, 120.0f, vStart, vEnd) == false)
	{
		discard;
	}
	
	Out.vColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	vWorldPos = vStart;

	int iMaxStep = 10;
	float fLength =  min(1000.0f, length(vEnd - vStart));
	float fStepSize = fLength / (float)iMaxStep;

	//float3 vSphereWorld = float3(20.0f, 20.0f, 20.0f);
	float fRadius = 5.0f;
	
	float fDensity = 0.0f;
	
	for (int i = 0; i < iMaxStep; ++i)
	{
		float3 vTexcoord = Compute_Texcoord(vWorldPos);
		//vTexcoord += g_fOffset;
		//vTexcoord *= 0.25f;
		fDensity += g_NoiseTexture.Sample(CloudSampler, vTexcoord).x * fStepSize * 0.5f;
		
		//fDensity = fStepSize * 0.05f;
		vWorldPos += vRayDir * fStepSize;
	}
	
	if (fDensity == 0.0f)
	{
		discard;
	}
	
	Out.vColor.a *= min(fDensity, 1.0f);

	return Out;
}

float Blue_Noise(float2 vTexcoord, float fStepLength)
{
	return (g_BlueNoiseTexture.Sample(LinearSampler, vTexcoord).x - 0.5f) * 2.0f * fStepLength;
}

bool Check_BoxIn(float3 vWorldPos)
{
	if (vWorldPos.x > g_vBoxMin.x 
		&& vWorldPos.x < g_vBoxMax.x 
		&& vWorldPos.y > g_vBoxMin.y 
		&& vWorldPos.y < g_vBoxMax.y 
		&& vWorldPos.z > g_vBoxMin.z 
		&& vWorldPos.z < g_vBoxMax.z)
	{
		return true;
	}

	return false;
}



float Calculate_Light(float fDensity, float3 vWorldPos, float fCos)
{
	float3 vDir = -g_vLightDir;
	for (int i = 0; i < g_iSunStep; ++i)
	{
		if (Check_BoxIn(vWorldPos) == true)
		{
			
		}
	}

	return fDensity * g_fStepLength * 0.05f;
}

float4 RayMarch(float3 vStartPos, float3 vRayDir)
{
	float fDensity = 0.0f;
	
	for (int i = 0; i < g_iMaxStep; ++i)
	{
		if (Check_BoxIn(vStartPos))
		{
			float3 vTexcoord = float3(remap(vStartPos.x, 0.0f, 500.0f, 0.0f, 1.0f), remap(vStartPos.y, 100.0f, 200.0f, 0.0f, 1.0f), remap(vStartPos.z, 0.0f, 500.0f, 0.0f, 1.0f));
			vTexcoord.z += g_fOffset;
			float fSampleDensity = g_NoiseTexture.Sample(CloudSampler, vTexcoord).x;

			if (fSampleDensity > 0.0f)
			{
				fDensity += Calculate_Light(fSampleDensity, vStartPos, dot(vRayDir, -g_vLightDir));
			}
		}
		vStartPos += vRayDir * g_fStepLength;
	}
	
	if (fDensity == 0.0f)
	{
		discard;
	}
	
	return float4(1.0f, 1.0f, 1.0f, min(fDensity, 1.0f));
}


PS_OUT PS_MAIN_PERLINWORLEYTEST(PS_IN In)
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
	vWorldPos += vRayDir * Blue_Noise(In.vTexcoord, g_fStepLength);

	Out.vColor = RayMarch(vWorldPos, vRayDir);

	return Out;
}


PS_OUT PS_MAIN_PERLINWORLEY2D(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	float4 vSample = g_NoiseTexture.Sample(CloudSampler, float3(In.vTexcoord.x, g_fOffset, In.vTexcoord.y));
	float fDensity = 0.0f;
	Out.vColor = float4(1.0f, 1.0f, 1.0f, fDensity);
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
		PixelShader = compile ps_5_0 PS_MAIN_PERLINWORLEYTEST();
	}

	pass PerlinWorley2D
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_PERLINWORLEY2D();
	}
}




