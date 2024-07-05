#include "..\Public\Cloud.h"
#include "GameInstance.h"

CCloud::CCloud(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
{

	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CCloud::CCloud(const CCloud & rhs)
	: m_pDevice(rhs.m_pDevice)
	, m_pContext(rhs.m_pContext)
	
{

	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CCloud::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCloud::Initialize(void* pArg)
{
	
	return S_OK;
}

void CCloud::PriorityTick(_float fTimeDelta)
{

}

void CCloud::Tick(_float fTimeDelta)
{
}

void CCloud::LateTick(_float fTimeDelta)
{
}

HRESULT CCloud::Render()
{
	return S_OK;
}

CCloud* CCloud::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCloud* pInstance = new CCloud(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CCloud");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCloud::Free()
{
	__super::Free();


	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
