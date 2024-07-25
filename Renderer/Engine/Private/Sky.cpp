#include "..\Public\Sky.h"
#include "GameInstance.h"
#include "TransmittanceLUT.h"
#include "Target_Manager.h"
#include "Renderer.h"

CSky::CSky(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
	, m_pTarget_Manager(CTarget_Manager::GetInstance())
{
	Safe_AddRef(m_pTarget_Manager);
}

CSky::CSky(const CSky& rhs)
	: CGameObject(rhs)
	, m_pTarget_Manager(CTarget_Manager::GetInstance())
	, m_WorldMatrix(rhs.m_WorldMatrix)
	, m_ViewMatrix(rhs.m_ViewMatrix)
	, m_ProjMatrix(rhs.m_ProjMatrix)
{
	Safe_AddRef(m_pTarget_Manager);
}

HRESULT CSky::Initialize_Prototype()
{
	m_WorldMatrix = XMMatrixIdentity();
	m_WorldMatrix._11 = m_iWinSizeX;
	m_WorldMatrix._22 = m_iWinSizeY;


	m_ViewMatrix = XMMatrixIdentity();
	m_ProjMatrix = XMMatrixOrthographicLH(m_iWinSizeX, m_iWinSizeY, 0.f, 1.f);

	if (FAILED(Ready_RenderTargets()))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CSky::Initialize(void* pArg)
{
	m_tUnitAtmo = m_tUnitAtmo.toStdUnit();
	
	if (FAILED(Ready_For_LUT()))
	{
		return E_FAIL;
	}

	if (FAILED(Ready_Components()))
	{
		return E_FAIL;
	}

	return S_OK;
}

void CSky::PriorityTick(_float fTimeDelta)
{

}

void CSky::Tick(_float fTimeDelta)
{
}

void CSky::LateTick(_float fTimeDelta)
{
	m_pRendererCom->Add_RenderGroup(CRenderer::RG_SKY, this);
}

HRESULT CSky::Render()
{
	if (FAILED(m_pTarget_Manager->Begin_MRT(m_pContext, L"MRT_LUT")))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	CPipeLine* pPipeLine = GET_INSTANCE(CPipeLine);

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", &pPipeLine->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", &pPipeLine->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", &pPipeLine->Get_CamPosition(), sizeof(Vec3))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Texture("g_TransLUTTexture", m_pTransLUTSRV)))
	{
		return E_FAIL;
	}


	RELEASE_INSTANCE(CPipeLine);

	if (FAILED(m_pShader->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->End_MRT(m_pContext)))
		return E_FAIL;


	return S_OK;
}

HRESULT CSky::Ready_For_LUT()
{
	CTransmittanceLUT* pTransLUT = CTransmittanceLUT::Create(m_pDevice, m_pContext);

 	m_pTransLUTSRV = pTransLUT->Generate_TransmittanceLUT(m_tUnitAtmo);

	if (m_pTransLUTSRV == nullptr)
	{
		return E_FAIL;
	}

	Safe_Release(pTransLUT);

	return S_OK;
}

HRESULT CSky::Ready_Components()
{
	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_AtmosphereRender.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	/* Com_Renderer */
	if (FAILED(__super::Add_Component(0, TEXT("Prototype_Component_Renderer"),
		TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSky::Ready_RenderTargets()
{
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, L"Target_TransLUT",
		m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R32G32B32A32_FLOAT, Vec4(0.0f, 0.0f, 0.0f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Add_MRT(L"MRT_LUT", L"Target_TransLUT")))
		return E_FAIL;

	return S_OK;
}


CGameObject* CSky::Clone(void* pArg)
{
	CSky* pInstance = new CSky(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTerrain");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CSky* CSky::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSky* pInstance = new CSky(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CCloud");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSky::Free()
{
	__super::Free();

	Safe_Release(m_pShader);

	Safe_Release(m_pTransLUTSRV);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
