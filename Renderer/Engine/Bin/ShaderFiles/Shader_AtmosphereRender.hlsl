#include "AtmosphereDefines.hlsl"


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


SingleScatteringResult IntegrateScatteredLuminance(
	in float3 vWorldPos, in float3 vWorldDir, in float3 vSunDir,
	in bool bGground, in float fSampleCountIni, in float fDepthBufferValue,
	in float fMaxMax = 90000000.0f)
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

	float3 vGlobalL = 1.0f;

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

		float3 fMultiScatteredLuminance = 0.0f;
		fMultiScatteredLuminance = GetMultipleScattering(vPos, fSunZenithCosAngle);

		float3 vS = vGlobalL * (fEarthShadow * vTransmittanceToSun * vPhaseTimesScattering + fMultiScatteredLuminance * tMedium.vScattering);

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


PS_OUT PS_SKY_VEIW_LUT(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

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
	 
	float2 vUV = In.vTexcoord;

	float fViewHeight = length(vWorldPos);

	float fViewZenithCosAngle;
	float fLightViewCosAngle;
	UvToSkyViewLutParams(fViewZenithCosAngle, fLightViewCosAngle, fViewHeight, vUV);

	float3 vSunDirection = normalize(g_vSunPos - vWorldPos);

	float3 vSunDir;
	{
		float3 vUpVector = vWorldPos / fViewHeight;
		float fSunZenithCosAngle = dot(vUpVector, vSunDirection);
		vSunDir = normalize(float3(sqrt(1.0f - fSunZenithCosAngle * fSunZenithCosAngle), fSunZenithCosAngle, 0.0f));
	}

	vWorldPos = float3(0.0f, fViewHeight, 0.0f);

	float fViewZenithSinAngle = sqrt(1.0f - fViewZenithCosAngle * fViewZenithCosAngle);
	vWorldDir = float3(
		fViewZenithSinAngle * fLightViewCosAngle,
		fViewZenithCosAngle,
		fViewZenithSinAngle * sqrt(1.0f - fLightViewCosAngle * fLightViewCosAngle));

	if (!MoveToTopAtmosphere(vWorldPos, vWorldDir, fAtmosphereRadius))
	{
		Out.vColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
		return Out;
	}

	const bool bGround = false;
	const float fSampleCountIni = 30.0f;
	const float fDepthBufferValue = -1.0f;
	SingleScatteringResult tResult = IntegrateScatteredLuminance(vWorldPos, vWorldDir, vSunDir, bGround, fSampleCountIni, fDepthBufferValue);

	float3 vL = tResult.vL;

	Out.vColor =  float4(vL, 1.0f);

	return Out;
}


///////////////////////////////////////////Atmosphere


PS_OUT PS_ATMOSPHERE(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

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

	float3 vLightDir = normalize(g_vSunPos - vWorldPos);


	float3 vSunDisk = GetSunLuminance(vWorldPos, vWorldDir, vLightDir);

	float2 pixPos = In.vTexcoord;

	float3 vSunDirection = normalize(g_vSunPos - vWorldPos);

	float fViewHeight = length(vWorldPos);
	float3 vL = 0;


	float4 vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
	

	if (vDepthDesc.x == 1.0f && fViewHeight < fAtmosphereRadius)
	{
		if (vSunDisk.x > 0.0f)
		{
			Out.vColor = float4(vSunDisk, 1.0f);
			return Out;
		}

		float2 vUV;
		float3 vUpVector = normalize(vWorldPos);
		float fViewZenithCosAngle = dot(vWorldDir, vUpVector);

		float3 vSideVector = normalize(cross(vUpVector, vWorldDir));		
		float3 vForwardVector = normalize(cross(vSideVector, vUpVector));
		float2 vLightOnPlane = float2(dot(vSunDirection, vForwardVector), dot(vSunDirection, vSideVector));
		vLightOnPlane = normalize(vLightOnPlane);
		float fLightViewCosAngle = vLightOnPlane.x;

		bool bIntersectGround = raySphereIntersectNearest(vWorldPos, vWorldDir, float3(0, 0, 0), fEarthRadius) >= 0.0f;

		SkyViewLutParamsToUv(bIntersectGround, fViewZenithCosAngle, fLightViewCosAngle, fViewHeight, vUV);

		Out.vColor = float4(g_SkyViewLUTTexture.SampleLevel(LinearClampSampler, vUV, 0).rgb, 1.0f);
	}
	else
	{
		if (g_bAerial == true)
		{
			float		fViewZ = vDepthDesc.y * 1000.f;

			float4 vDepthBufferWorldPos;

			vDepthBufferWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
			vDepthBufferWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
			vDepthBufferWorldPos.z = vDepthDesc.x;
			vDepthBufferWorldPos.w = 1.f;

			vDepthBufferWorldPos = vDepthBufferWorldPos * fViewZ;
			vDepthBufferWorldPos = mul(vDepthBufferWorldPos, g_ProjMatrixInv);
			vDepthBufferWorldPos = mul(vDepthBufferWorldPos, g_ViewMatrixInv);

			float fDepth = length(vDepthBufferWorldPos.xyz - (vWorldPos + float3(0.0f, -fEarthRadius, 0.0f)));
			fDepth *= 30.0f;
			float fSlice = fDepth * (1.0f / M_PER_SLICE);


			float fWeight = 1.0;
			if (fSlice < 0.5)
			{
				fWeight = saturate(fSlice * 2.0);
				fSlice = 0.5;
			}
			float w = sqrt(fSlice / DEPTHCOUNT);

			const float4 vAP = fWeight * g_AerialLUTTexture.SampleLevel(LinearClampSampler, float3(In.vTexcoord, w), 0);
			vL.rgb += vAP.rgb;
			float fOpacity = vAP.a;

			Out.vColor = float4(vL, fOpacity);
		}
	}
	
	Out.vColor.xyz *= fSunIlluminance;


	return Out;
}

PS_OUT PS_LUTTEST(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;


	Out.vColor = g_MultiScatLUTTexture.SampleLevel(PointSampler, In.vTexcoord, 0);

	return Out;
}

PS_OUT PS_CLOUD(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	float4 vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);

	if (vDepthDesc.x == 1.0f)
	{
		Out.vColor = g_CloudTexture.Sample(LinearSampler, In.vTexcoord);

		if (Out.vColor.a == 0.0f)
		{
			discard;
		}

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

		float fBottom = raySphereIntersectNearest(vWorldPos, vWorldDir, float3(0, 0, 0), fEarthRadius) * 20.0f;
		float fTop = raySphereIntersectNearest(vWorldPos, vWorldDir, float3(0, 0, 0), fAtmosphereRadius);

		float fDistance = 0.0f;

		if (fBottom < 0.0f)
		{
			fDistance = fTop;
		}
		else
		{
			if (fTop > 0.0f)
			{
				fDistance = min(fTop, fBottom);
			}
		}

		fDistance *= 0.0000013f;
		fDistance = pow(fDistance, 8);

		Out.vColor.a = lerp(0.0f, Out.vColor.a, exp(-fDistance));
	}

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
		PixelShader = compile ps_5_0 PS_ATMOSPHERE();
	}
	
	pass SkyViewLUT
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_SKY_VEIW_LUT();
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

	pass Cloud
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_AlphaBlendMax, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_CLOUD();
	}
}




