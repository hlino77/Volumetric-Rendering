#include "..\Public\Light_Manager.h"
#include "Light.h"

IMPLEMENT_SINGLETON(CLight_Manager)

CLight_Manager::CLight_Manager()
{

}

const LIGHT_DESC* CLight_Manager::Get_LightDesc(_uint iLightIndex)
{
	if (iLightIndex >= m_Lights.size())
		return nullptr;

	auto	iter = m_Lights.begin();

	for (size_t i = 0; i < iLightIndex; i++)
		++iter;

	return (*iter)->Get_LightDesc();	
}

HRESULT CLight_Manager::Add_Light(const LIGHT_DESC & LightDesc)
{
	CLight*		pLight = CLight::Create(LightDesc);

	if (nullptr == pLight)
		return E_FAIL;

	m_Lights.push_back(pLight);

	return S_OK;
}

HRESULT CLight_Manager::Add_Sun(const LIGHT_DESC& LightDesc)
{
	m_pSunLight = CLight::Create(LightDesc);

	if (nullptr == m_pSunLight)
		return E_FAIL;

	m_Lights.push_back(m_pSunLight);

	return S_OK;
}

HRESULT CLight_Manager::Render(CShader * pShader, CVIBuffer_Rect * pVIBuffer)
{
	for (auto& pLight : m_Lights)
	{
 		pLight->Render(pShader, pVIBuffer);
	}

	return S_OK;
}

void CLight_Manager::Set_SunPos(Vec3 vPos)
{
	if (m_pSunLight == nullptr)
	{
		return;
	}

	m_pSunLight->Set_LightPos(vPos);
}

void CLight_Manager::Set_TransLUT(ID3D11ShaderResourceView* pSRV)
{
	if (m_pSunLight == nullptr)
	{
		return;
	}

	m_pSunLight->Set_TransLUT(pSRV);
}

void CLight_Manager::Free()
{
	for (auto& pLight : m_Lights)
		Safe_Release(pLight);

	m_Lights.clear();
}
