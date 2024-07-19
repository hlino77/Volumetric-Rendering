#include "..\Public\Cloud.h"
#include "GameInstance.h"
#include "NoiseGenerator.h"
#include "Target_Manager.h"
#include "Renderer.h"

CCloud::CCloud(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
	, m_pTarget_Manager(CTarget_Manager::GetInstance())
{
	Safe_AddRef(m_pTarget_Manager);
}

CCloud::CCloud(const CCloud& rhs)
	: CGameObject(rhs)
	, m_pTarget_Manager(CTarget_Manager::GetInstance())
	, m_Targets(rhs.m_Targets)
	, m_WorldMatrix(rhs.m_WorldMatrix)
	, m_ViewMatrix(rhs.m_ViewMatrix)
	, m_ProjMatrix(rhs.m_ProjMatrix)
	, m_iWinSizeX(rhs.m_iWinSizeX)
	, m_iWinSizeY(rhs.m_iWinSizeY)
{
	Safe_AddRef(m_pTarget_Manager);
}

HRESULT CCloud::Initialize_Prototype()
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

HRESULT CCloud::Initialize(void* pArg)
{

	if (FAILED(Ready_For_NoiseTexture3D()))
	{
		return E_FAIL;
	}

	if (FAILED(Ready_Components()))
	{
		return E_FAIL;
	}

	if (FAILED(Ready_UpdatePixel()))
	{
		return E_FAIL;
	}

	return S_OK;
}

void CCloud::PriorityTick(_float fTimeDelta)
{

}

void CCloud::Tick(_float fTimeDelta)
{
	m_iCurrUpdatePixel = (m_iCurrUpdatePixel + 1) % (m_iGridSize * m_iGridSize);

	m_bSwap = !m_bSwap;
}

void CCloud::LateTick(_float fTimeDelta)
{
	m_pRendererCom->Add_RenderGroup(CRenderer::RG_SKY, this);
}

HRESULT CCloud::Render()
{
	if (FAILED(m_pTarget_Manager->Begin_MRT(m_pContext, m_Targets[m_bSwap].szMRT)))
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
	if (FAILED(m_pShader->Bind_Matrix("g_PrevViewMatrixInv", &m_PrevViewMatrixInv)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", &pPipeLine->Get_CamPosition(), sizeof(Vec3))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Texture("g_ShapeTexture", m_pShapeSRV)))
	{
		return E_FAIL;
	}

	if (FAILED(m_pShader->Bind_Texture("g_DetailTexture", m_pDetailSRV)))
	{
		return E_FAIL;
	}

	if (FAILED(m_pBlueNoiseTexture->Bind_ShaderResource(m_pShader, "g_BlueNoiseTexture", 0)))
	{
		return E_FAIL;
	}

	if (FAILED(m_pCurlNoiseTexture->Bind_ShaderResource(m_pShader, "g_CurlNoiseTexture", 0)))
	{
		return E_FAIL;
	}

	if (FAILED(m_pTarget_Manager->Bind_SRV(m_pShader, m_Targets[!m_bSwap].szTarget, "g_PrevFrameTexture")))
	{
		return E_FAIL;
	}

	if (FAILED(m_pShader->Bind_RawValue("g_iGridSize", &m_iGridSize, sizeof(_uint))))
	{
		return E_FAIL;
	}

	if (FAILED(m_pShader->Bind_RawValue("g_iUpdatePixel", &m_UpdatePixel[m_iCurrUpdatePixel], sizeof(_uint))))
	{
		return E_FAIL;
	}

 	if (FAILED(m_pShader->Bind_RawValue("g_iWinSizeX", &m_iWinSizeX, sizeof(_uint))))
	{
		return E_FAIL;
	}
	if (FAILED(m_pShader->Bind_RawValue("g_iWinSizeY", &m_iWinSizeY, sizeof(_uint))))
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

	m_pRendererCom->Set_SkyTargetName(m_Targets[m_bSwap].szTarget);
	m_PrevViewMatrixInv = pPipeLine->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_VIEW);

	return S_OK;
}

HRESULT CCloud::Ready_For_NoiseTexture3D()
{
	CNoiseGenerator* pNoiseGenerator = CNoiseGenerator::Create(m_pDevice, m_pContext);

	m_pShapeSRV = pNoiseGenerator->Generate_ShapeNoise();

	if (m_pShapeSRV == nullptr)
	{
		return E_FAIL;
	}

	m_pDetailSRV = pNoiseGenerator->Generate_DetailNoise();

	if (m_pDetailSRV == nullptr)
	{
		return E_FAIL;
	}

	Safe_Release(pNoiseGenerator);

	return S_OK;
}

HRESULT CCloud::Ready_Components()
{
	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VolumeRender.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	m_pBlueNoiseTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/BlueNoise470.png"));
	if (nullptr == m_pBlueNoiseTexture)
		return E_FAIL;

	m_pCurlNoiseTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/CurlNoise.png"));
	if (nullptr == m_pCurlNoiseTexture)
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

HRESULT CCloud::Ready_RenderTargets()
{

	CloudTarget Cloud1, Cloud2;

	Cloud1.szMRT = L"MRT_Cloud1";
	Cloud1.szTarget = L"Target_Cloud1";

	Cloud2.szMRT = L"MRT_Cloud2";
	Cloud2.szTarget = L"Target_Cloud2";

	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, Cloud1.szTarget,
		m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R32G32B32A32_FLOAT, Vec4(0.0f, 0.0f, 0.0f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, Cloud2.szTarget,
		m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R32G32B32A32_FLOAT, Vec4(0.0f, 0.0f, 0.0f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Add_MRT(Cloud1.szMRT, Cloud1.szTarget)))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Add_MRT(Cloud2.szMRT, Cloud2.szTarget)))
		return E_FAIL;

	m_Targets.push_back(Cloud1);
	m_Targets.push_back(Cloud2);

	return S_OK;
}

HRESULT CCloud::Ready_UpdatePixel()
{
	m_UpdatePixel.push_back(0);
	m_UpdatePixel.push_back(10);
	m_UpdatePixel.push_back(2);
	m_UpdatePixel.push_back(8);

	m_UpdatePixel.push_back(5);
	m_UpdatePixel.push_back(15);
	m_UpdatePixel.push_back(7);
	m_UpdatePixel.push_back(13);

	m_UpdatePixel.push_back(1);
	m_UpdatePixel.push_back(11);
	m_UpdatePixel.push_back(3);
	m_UpdatePixel.push_back(9);

	m_UpdatePixel.push_back(4);
	m_UpdatePixel.push_back(14);
	m_UpdatePixel.push_back(6);
	m_UpdatePixel.push_back(12);

	return S_OK;
}



CGameObject* CCloud::Clone(void* pArg)
{
	CCloud* pInstance = new CCloud(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTerrain");
		Safe_Release(pInstance);
	}

	return pInstance;
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

	Safe_Release(m_pShader);
	Safe_Release(m_pBlueNoiseTexture);
	Safe_Release(m_pCurlNoiseTexture);

	Safe_Release(m_pDetailSRV);
	Safe_Release(m_pShapeSRV);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
