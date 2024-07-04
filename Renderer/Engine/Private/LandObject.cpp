#include "..\Public\LandObject.h"

#include "Transform.h"
#include "VIBuffer_Terrain.h"

CLandObject::CLandObject(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{

}

CLandObject::CLandObject(const CLandObject & rhs)
	: CGameObject(rhs)
{

}

HRESULT CLandObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLandObject::Initialize(void * pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	LANDOBJECT_DESC*		pDesc = (LANDOBJECT_DESC*)pArg;

	m_pTerrainTransform = pDesc->pTerrainTransform;
	m_pVIBuffer_Terrain = pDesc->pVIBuffer_Terrain;

	Safe_AddRef(m_pTerrainTransform);
	Safe_AddRef(m_pVIBuffer_Terrain);

	return S_OK;
}

void CLandObject::PriorityTick(_float fTimeDelta)
{
}

void CLandObject::Tick(_float fTimeDelta)
{
}

void CLandObject::LateTick(_float fTimeDelta)
{
}

HRESULT CLandObject::Render()
{
	return S_OK;
}

Vec4 CLandObject::SetUp_OnTerrain(Vec4 vWorldPos)
{
	return m_pVIBuffer_Terrain->SetUp_OnTerrain(m_pTerrainTransform, vWorldPos);	
}

void CLandObject::Free()
{
	__super::Free();

	Safe_Release(m_pTerrainTransform);
	Safe_Release(m_pVIBuffer_Terrain);
}
