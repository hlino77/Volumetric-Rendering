#include "stdafx.h"
#include "..\Public\Terrain.h"

#include "GameInstance.h"

CTerrain::CTerrain(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{

}

CTerrain::CTerrain(const CGameObject & rhs)
	: CGameObject(rhs)
{

}

HRESULT CTerrain::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTerrain::Initialize(void* pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE::STATE_POSITION, Vec3(-500.0f, 0.0f, -500.0f));

	return S_OK;
}

void CTerrain::PriorityTick(_float fTimeDelta)
{	
}

void CTerrain::Tick(_float fTimeDelta)
{

}

void CTerrain::LateTick(_float fTimeDelta)
{
	// m_pVIBufferCom->Culling(m_pTransformCom);

	//m_pRendererCom->Add_RenderGroup(CRenderer::RG_SHADOW, this);
	m_pRendererCom->Add_RenderGroup(CRenderer::RG_NONBLEND, this);	
}

HRESULT CTerrain::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	m_pVIBufferCom->Render();


	return S_OK;
}

HRESULT CTerrain::Render_LightDepth()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResources(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	CGameInstance* pGameInstance = CGameInstance::GetInstance();
	Safe_AddRef(pGameInstance);

	if (FAILED(pGameInstance->Bind_TransformToShader(m_pShaderCom, "g_LightViewMatrix", CPipeLine::D3DTS_LIGHTVIEW)))
		return E_FAIL;
	if (FAILED(pGameInstance->Bind_TransformToShader(m_pShaderCom, "g_LightProjMatrix", CPipeLine::D3DTS_LIGHTPROJ)))
		return E_FAIL;

	Safe_Release(pGameInstance);

	m_pShaderCom->Begin(1);

	m_pVIBufferCom->Render();

	return S_OK;
}

HRESULT CTerrain::Ready_Components()
{
	/* Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"),
		TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxNorTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Terrain"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	/* Com_Texture*/
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom[TEX_DIFFUSE])))
		return E_FAIL;

	/* Com_Texture_Mask*/
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain_Mask"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTextureCom[TEX_MASK])))
		return E_FAIL;

	/* Com_Texture_Brush */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Brush"),
		TEXT("Com_Texture_Brush"), (CComponent**)&m_pTextureCom[TEX_BRUSH])))
		return E_FAIL;

	/* Com_Transform */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"),
		TEXT("Com_Transform"), (CComponent**)&m_pTransformCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CTerrain::Bind_ShaderResources()
{
	/* 셰이더 전역변수로 던져야 할 값들을 던지자. */
	//if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &IdentityMatrix)))
	//	return E_FAIL;


	if (FAILED(m_pTransformCom->Bind_ShaderResources(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;


	CGameInstance* pGameInstance = CGameInstance::GetInstance();
	Safe_AddRef(pGameInstance);

	if (FAILED(pGameInstance->Bind_TransformToShader(m_pShaderCom, "g_ViewMatrix", CPipeLine::D3DTS_VIEW)))
		return E_FAIL;
	if (FAILED(pGameInstance->Bind_TransformToShader(m_pShaderCom, "g_ProjMatrix", CPipeLine::D3DTS_PROJ)))
		return E_FAIL;

	Safe_Release(pGameInstance);

	if (FAILED(m_pTextureCom[TEX_DIFFUSE]->Bind_ShaderResources(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (FAILED(m_pTextureCom[TEX_MASK]->Bind_ShaderResources(m_pShaderCom, "g_MaskTexture")))
		return E_FAIL;

	if (FAILED(m_pTextureCom[TEX_BRUSH]->Bind_ShaderResources(m_pShaderCom, "g_BrushTexture")))
		return E_FAIL;

	m_pShaderCom->Begin(0);

	return S_OK;
}

CTerrain * CTerrain::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTerrain*	pInstance = new CTerrain(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTerrain");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CTerrain::Clone(void* pArg)
{
	CTerrain*	pInstance = new CTerrain(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTerrain");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTerrain::Free()
{
	__super::Free();

	Safe_Release(m_pTransformCom);

	for (auto& pTextureCom : m_pTextureCom)
		Safe_Release(pTextureCom);
	
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pRendererCom);
}
