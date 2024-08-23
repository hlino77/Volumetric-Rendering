#include "..\Public\Light.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

CLight::CLight()
{
}

HRESULT CLight::Initialize(const LIGHT_DESC & LightDesc)
{
	memmove(&m_LightDesc, &LightDesc, sizeof LightDesc);

	return S_OK;
}

HRESULT CLight::Render(CShader * pShader, CVIBuffer_Rect * pVIBuffer)
{
	_uint		iPassIndex = 0;



	if (LIGHT_DESC::LIGHT_SUN == m_LightDesc.eLightType)
	{
		if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vLightPos, sizeof(Vec4))))
			return E_FAIL;

		if (FAILED(pShader->Bind_Texture("g_TransLUTTexture", m_LightDesc.pTransLUT)))
		{
			return E_FAIL;
		}

		if (FAILED(pShader->Bind_Texture("g_MultiScatLUTTexture", m_LightDesc.pMultiScatLUT)))
		{
			return E_FAIL;
		}

		if (FAILED(pShader->Bind_ConstantBuffer("AtmosphereParams", m_LightDesc.pAtmosphereBuffer)))
		{
			return E_FAIL;
		}

		iPassIndex = 5;
	}

	if (FAILED(pShader->Begin(iPassIndex)))
		return E_FAIL;

	if (FAILED(pVIBuffer->Render()))
		return E_FAIL;

	return S_OK;
}

CLight * CLight::Create(const LIGHT_DESC & LightDesc)
{
	CLight*		pInstance = new CLight;

	if (FAILED(pInstance->Initialize(LightDesc)))
	{
		MSG_BOX("Failed to Created : CLight");
		Safe_Release(pInstance);		
	}

	return pInstance;
}

void CLight::Free()
{
}
