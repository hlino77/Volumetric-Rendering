#include "..\Public\Renderer.h"
#include "GameObject.h"
#include "Target_Manager.h"
#include "Light_Manager.h"
#include "VIBuffer_Rect.h"
#include "PipeLine.h"
#include "Shader.h"
#include "Cloud.h"
#include "GameInstance.h"
#include "Texture.h"

CRenderer::CRenderer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent(pDevice, pContext)
	, m_pTarget_Manager(CTarget_Manager::GetInstance())
	, m_pLight_Manager(CLight_Manager::GetInstance())
{
	Safe_AddRef(m_pLight_Manager);
	Safe_AddRef(m_pTarget_Manager);
}

HRESULT CRenderer::Initialize_Prototype()
{
	D3D11_VIEWPORT		ViewportDesc;

	_uint				iNumViewports = 1;

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	/* For.Target_Diffuse */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_Diffuse"), 
		ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM, Vec4(1.f, 1.f, 1.f, 0.f))))
		return E_FAIL;

	/* For.Target_Normal */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_Normal"),
		ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_UNORM, Vec4(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;

	/* For.Target_Depth */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_Depth"),
		ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, Vec4(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;

	/* For.Target_Shade */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_Shade"),
		ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_UNORM, Vec4(0.f, 0.f, 0.f, 1.f))))
		return E_FAIL;

	/* For.Target_Specular */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_Specular"),
		ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_UNORM, Vec4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_LightDepth */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_LightDepth"),
		ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, Vec4(1.0f, 1.0f, 1.0f, 1.f))))
		return E_FAIL;

	/* For.Target_VolumeRender */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_VolumeRender"),
		ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, Vec4(0.0f, 0.0f, 0.0f, 0.f))))
		return E_FAIL;

#ifdef _DEBUG
	if (FAILED(m_pTarget_Manager->Ready_Debug(TEXT("Target_Diffuse"), 100.0f, 100.f, 200.0f, 200.0f)))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Ready_Debug(TEXT("Target_Normal"), 100.0f, 300.0f, 200.0f, 200.0f)))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Ready_Debug(TEXT("Target_Depth"), 100.0f, 500.0f, 200.0f, 200.0f)))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Ready_Debug(TEXT("Target_Shade"), 300.0f, 100.f, 200.0f, 200.0f)))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Ready_Debug(TEXT("Target_Specular"), 300.0f, 300.0f, 200.0f, 200.0f)))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Ready_Debug(TEXT("Target_VolumeRender"), ViewportDesc.Width - 250.0f, 250.0f, 500.0f, 500.0f)))
		return E_FAIL;
		/*if (FAILED(m_pTarget_Manager->Ready_Debug(TEXT("Target_VolumeRender"), ViewportDesc.Width * 0.5f, ViewportDesc.Height * 0.5f, ViewportDesc.Width, ViewportDesc.Height)))
			return E_FAIL;*/



#endif

	/* 이 렌더타겟들은 그려지는 객체로부터 값을 저장받는다. */
	/* For.MRT_GameObjects */
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Diffuse"))))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Normal"))))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Depth"))))
		return E_FAIL;
	/* 이 렌더타겟들은 게임내에 존재하는 빛으로부터 연산한 결과를 저장받는다. */
	/* For.MRT_Lights */

	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_VolumeRender"), TEXT("Target_VolumeRender"))))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_Lights"), TEXT("Target_Shade"))))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_Lights"), TEXT("Target_Specular"))))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_LightDepth"), TEXT("Target_LightDepth"))))
		return E_FAIL;


	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Deferred.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	m_pVolumeRenderShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VolumeRender.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	m_pBlueNoiseTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/BlueNoise470.png"));
	if (nullptr == m_pBlueNoiseTexture)
		return E_FAIL;

	m_pCurlNoiseTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/CurlNoise.png"));
	if (nullptr == m_pCurlNoiseTexture)
		return E_FAIL;

	m_pCloud = CCloud::Create(m_pDevice, m_pContext);
	if (nullptr == m_pCloud)
		return E_FAIL;
	Safe_AddRef(m_pCloud);

	m_WorldMatrix = XMMatrixIdentity();
	m_WorldMatrix._11 = ViewportDesc.Width;
	m_WorldMatrix._22 = ViewportDesc.Height;

	m_ViewMatrix = XMMatrixIdentity();
	m_ProjMatrix = XMMatrixOrthographicLH(ViewportDesc.Width, ViewportDesc.Height, 0.f, 1.f);

	return S_OK;
}

HRESULT CRenderer::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CRenderer::Add_RenderGroup(RENDERGROUP eRenderGroup, CGameObject * pGameObject)
{
	if (eRenderGroup >= RG_END)
		return E_FAIL;

	m_RenderObjects[eRenderGroup].push_back(pGameObject);

	Safe_AddRef(pGameObject);

	return S_OK;
}

HRESULT CRenderer::Draw_RenderObjects()
{
	if (FAILED(Render_Priority()))
		return E_FAIL;
	if (FAILED(Render_NonLight()))
		return E_FAIL;	
	if (FAILED(Render_LightDepth()))
		return E_FAIL;
	if (FAILED(Render_NonBlend()))
		return E_FAIL;
	if (FAILED(Render_LightAcc()))
		return E_FAIL;
	if (FAILED(Render_Volume()))
		return E_FAIL;
	if (FAILED(Render_Deferred()))
		return E_FAIL;
	if (FAILED(Render_Blend()))
		return E_FAIL;
	if (FAILED(Render_UI()))
		return E_FAIL;

#ifdef _DEBUG
	if (FAILED(Render_Debug()))
		return E_FAIL;
#endif

	return S_OK;
}

HRESULT CRenderer::Render_Priority()
{
	for (auto& pGameObject : m_RenderObjects[RG_PRIORITY])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[RG_PRIORITY].clear();

	return S_OK;
}

HRESULT CRenderer::Render_NonLight()
{
	for (auto& pGameObject : m_RenderObjects[RG_NONLIGHT])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[RG_NONLIGHT].clear();
	
	return S_OK;
}

HRESULT CRenderer::Render_LightDepth()
{
	if (FAILED(m_pTarget_Manager->Begin_MRT(m_pContext, TEXT("MRT_LightDepth"))))
		return E_FAIL;

	for (auto& pGameObject : m_RenderObjects[RG_SHADOW])
	{
		if (nullptr != pGameObject)
			pGameObject->Render_LightDepth();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[RG_SHADOW].clear();

	if (FAILED(m_pTarget_Manager->End_MRT(m_pContext)))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_NonBlend()
{
	/* Diffuse + Normal + Depth */
	if (FAILED(m_pTarget_Manager->Begin_MRT(m_pContext, TEXT("MRT_GameObjects"))))
		return E_FAIL;

	for (auto& pGameObject : m_RenderObjects[RG_NONBLEND])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[RG_NONBLEND].clear();

	if (FAILED(m_pTarget_Manager->End_MRT(m_pContext)))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_LightAcc()
{
	if (FAILED(m_pTarget_Manager->Begin_MRT(m_pContext, TEXT("MRT_Lights"))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	CPipeLine*		pPipeLine = GET_INSTANCE(CPipeLine);

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", &pPipeLine->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", &pPipeLine->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", &pPipeLine->Get_CamPosition(), sizeof(Vec3))))
		return E_FAIL;

	RELEASE_INSTANCE(CPipeLine);

	if (FAILED(m_pTarget_Manager->Bind_SRV(m_pShader, TEXT("Target_Normal"), "g_NormalTexture")))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Bind_SRV(m_pShader, TEXT("Target_Depth"), "g_DepthTexture")))
		return E_FAIL;

	m_pLight_Manager->Render(m_pShader, m_pVIBuffer);

	if (FAILED(m_pTarget_Manager->End_MRT(m_pContext)))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Volume()
{
	if (FAILED(m_pTarget_Manager->Begin_MRT(m_pContext, TEXT("MRT_VolumeRender"))))
		return E_FAIL;

	if (FAILED(m_pVolumeRenderShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pVolumeRenderShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pVolumeRenderShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	CPipeLine* pPipeLine = GET_INSTANCE(CPipeLine);

	if (FAILED(m_pVolumeRenderShader->Bind_Matrix("g_ViewMatrixInv", &pPipeLine->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pVolumeRenderShader->Bind_Matrix("g_ProjMatrixInv", &pPipeLine->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pVolumeRenderShader->Bind_RawValue("g_vCamPosition", &pPipeLine->Get_CamPosition(), sizeof(Vec3))))
		return E_FAIL;

	if (FAILED(m_pVolumeRenderShader->Bind_Texture("g_NoiseTexture", m_pCloud->Get_SRV())))
	{
		return E_FAIL;
	}

	if (FAILED(m_pBlueNoiseTexture->Bind_ShaderResource(m_pVolumeRenderShader, "g_BlueNoiseTexture", 0)))
	{
		return E_FAIL;
	}

	if (FAILED(m_pCurlNoiseTexture->Bind_ShaderResource(m_pVolumeRenderShader, "g_CurlNoiseTexture", 0)))
	{
		return E_FAIL;
	}

	//m_fTest += 0.0001f;

	CGameInstance* pGameInstance = GET_INSTANCE(CGameInstance);
	if (pGameInstance->Get_DIKeyState(DIK_Y) & 0x80)
	{
		m_fTest += 0.0001f;
	}
	if (pGameInstance->Get_DIKeyState(DIK_H) & 0x80)
	{
		m_fTest -= 0.0001f;
	}

	/*if (m_fTest > 1.0f)
	{
		m_fTest -= 1.0f;
	}*/

	if (FAILED(m_pVolumeRenderShader->Bind_RawValue("g_fOffset", &m_fTest, sizeof(float))))
	{
		return E_FAIL;
	}

	RELEASE_INSTANCE(CPipeLine);

	if (FAILED(m_pVolumeRenderShader->Begin(1)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->End_MRT(m_pContext)))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Deferred()
{
	D3D11_VIEWPORT		ViewportDesc;

	_uint				iNumViewports = 1;

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Bind_SRV(m_pShader, TEXT("Target_Diffuse"), "g_DiffuseTexture")))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Bind_SRV(m_pShader, TEXT("Target_Shade"), "g_ShadeTexture")))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Bind_SRV(m_pShader, TEXT("Target_Specular"), "g_SpecularTexture")))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Bind_SRV(m_pShader, TEXT("Target_Depth"), "g_DepthTexture")))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Bind_SRV(m_pShader, TEXT("Target_LightDepth"), "g_LightDepthTexture")))
		return E_FAIL;

	Matrix		ViewMatrix, ProjMatrix;

	XMStoreFloat4x4(&ViewMatrix, XMMatrixLookAtLH(XMVectorSet(-30.f, 30.f, -30.0f, 1.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 1.f, 0.f, 0.f)));
	XMStoreFloat4x4(&ProjMatrix, XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (_float)ViewportDesc.Width / ViewportDesc.Height, 0.1f, 1000.f));

	if (FAILED(m_pShader->Bind_Matrix("g_LightViewMatrix", &ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_LightProjMatrix", &ProjMatrix)))
		return E_FAIL;


	CPipeLine*		pPipeLine = GET_INSTANCE(CPipeLine);

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", &pPipeLine->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", &pPipeLine->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;	

	RELEASE_INSTANCE(CPipeLine);

	if (FAILED(m_pShader->Begin(3)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return E_FAIL;



	if (FAILED(m_pTarget_Manager->Bind_SRV(m_pShader, TEXT("Target_VolumeRender"), "g_VolumeTexture")))
		return E_FAIL;

	if (FAILED(m_pShader->Begin(4)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Blend()
{
	for (auto& pGameObject : m_RenderObjects[RG_BLEND])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[RG_BLEND].clear();

	return S_OK;
}

HRESULT CRenderer::Render_UI()
{
	for (auto& pGameObject : m_RenderObjects[RG_UI])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[RG_UI].clear();


	return S_OK;
}

#ifdef _DEBUG

HRESULT CRenderer::Render_Debug()
{
	for (auto& pDebugCom : m_RenderDebug)
	{
		pDebugCom->Render();
		Safe_Release(pDebugCom);
	}
	m_RenderDebug.clear();

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	/*if (FAILED(m_pTarget_Manager->Render(TEXT("MRT_GameObjects"), m_pShader, m_pVIBuffer)))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Render(TEXT("MRT_Lights"), m_pShader, m_pVIBuffer)))
		return E_FAIL;*/
	if (FAILED(m_pTarget_Manager->Render(TEXT("MRT_VolumeRender"), m_pShader, m_pVIBuffer)))
		return E_FAIL;



	return S_OK;
}

#endif

CRenderer * CRenderer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CRenderer*	pInstance = new CRenderer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CRenderer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CRenderer::Clone(void * pArg)
{
	AddRef();

	return this;
}

void CRenderer::Free()
{
	__super::Free();
	
	Safe_Release(m_pCloud);
	Safe_Release(m_pShader);
	Safe_Release(m_pVIBuffer);

	Safe_Release(m_pLight_Manager);
	Safe_Release(m_pTarget_Manager);

}
