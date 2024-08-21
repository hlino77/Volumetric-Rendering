#include "stdafx.h"
#include "..\Public\MainApp.h"

#include "GameInstance.h"
#include "Level_Loading.h"



CMainApp::CMainApp()	
	: m_pGameInstance(CGameInstance::GetInstance())
{	
	// D3D11_SAMPLER_DESC
	
	// D3D11_SAMPLER_DESC

	Safe_AddRef(m_pGameInstance);

	/* 래스터라이져 (후면 추려내기, 와이어프레임, 솔리드) */

	//D3D11_RASTERIZER_DESC
	// D3D11_DEPTH_STENCIL_DESC
	// D3D11_BLEND_DESC

	//ID3D11RasterizerState*		pRS = nullptr;

	//D3D11_RASTERIZER_DESC		RasterizerDesc = {};


	//m_pDevice->CreateRasterizerState(&RasterizerDesc, &pRS);

	//m_pContext->RSSetState(pRS);

	/* 깊이 스텐실 (깊이 비교, 깊이 기록..) */
	// m_pContext->OMSetDepthStencilState();

	///* 블렌딩. (블렌딩 방식 )*/
	//m_pContext->OMSetBlendState();

	//
	/*
	m_pGraphic_Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	m_pGraphic_Device->SetRenderState(D3DRS_ALPHABLENDENBALE, TRUE);
	m_pGraphic_Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	m_pGraphic_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pGraphic_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	*/

	/* 
	_float4		vSrcColor(내가 백버퍼에 그릴려고하는 픽셀의 색), vDestColor(이미 백버퍼에 그려져있던 색);

	_float4		vResult =
		(vSrcColor * vSrcColor.a) + (vDestColor * (1.f - vSrcColor.a));
		*/

}


HRESULT CMainApp::Initialize()
{
	/* 1. 내 게임의 초기화를 수행할꺼야. */
	/* 1-1. 그래픽장치를 초기화한다. */
	/* 1-2. 사운드장치를 초기화한다. */
	/* 1-3. 입력장치를 초기화한다. */
	GRAPHIC_DESC		GraphicDesc;
	ZeroMemory(&GraphicDesc, sizeof GraphicDesc);

	GraphicDesc.eWinMode = GRAPHIC_DESC::WINMODE_WIN;
	GraphicDesc.hWnd = g_hWnd;
	GraphicDesc.iWinSizeX = g_iWinSizeX;
	GraphicDesc.iWinSizeY = g_iWinSizeY;

	if (FAILED(m_pGameInstance->Initialize_Engine(LEVEL_END, g_hInstance, GraphicDesc, &m_pDevice, &m_pContext)))
		return E_FAIL;



	if (FAILED(Ready_Prototype_Components()))
		return E_FAIL;

	// MakeSpriteFont "넥슨lv1고딕 Bold" /FontSize:30 /FastPack /CharacterRegion:0x0020-0x00FF /CharacterRegion:0x3131-0x3163 /CharacterRegion:0xAC00-0xD800 /DefaultCharacter:0xAC00 137ex.spritefont
	
	if (FAILED(m_pGameInstance->Add_Font(m_pDevice, m_pContext, TEXT("Font_Default"), TEXT("../Bin/Resources/Fonts/137ex.spriteFont"))))
		return E_FAIL;

	/* 1-4. 게임내에서 사용할 레벨(씬)을 생성한다.   */
	if (FAILED(Open_Level(LEVEL_GAMEPLAY)))
		return E_FAIL;

	//Imgui

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();


	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pContext);

	/* 1-4-1. 게임내에서 사용할 여러 자원(텍스쳐, 모델, 객체) 들을 준비한다.  */

	CGameInstance::GetInstance()->Add_Timer(L"Timer_Test");


	return S_OK;
}

void CMainApp::Tick(_float fTimeDelta)
{

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_pGameInstance->Tick(fTimeDelta);


}

HRESULT CMainApp::Render()
{
	m_pGameInstance->Clear_BackBuffer_View(Vec4(0.f, 0.f, 0.f, 1.f));
	m_pGameInstance->Clear_DepthStencil_View();

	m_pRenderer->Draw_RenderObjects();


	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_pGameInstance->Present();

	m_pGameInstance->AfterRenderTick();

	return S_OK;
}

HRESULT CMainApp::Open_Level(LEVELID eLevelID)
{
	if (nullptr == m_pGameInstance)
		return E_FAIL;

	/* 로고레베릉ㄹ 할당하고 싶었지만. 로고레벨을 위한 로딩레벨을 먼저 생성하여 로딩작업을 수행할꺼야. */
	/* 로딩객체에게 eLevelID라는 내가 실제 할당ㅎ아고 싶었던 레벨열거체를 준거지?! 실제할당하고싶었던 레벨에 자원을 준비라하라고 */
	if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, eLevelID))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMainApp::Ready_Prototype_Components()
{
	/* For.Prototype_Component_Renderer */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), 
		m_pRenderer = CRenderer::Create(m_pDevice, m_pContext))))
		return E_FAIL;	

	/* For.Prototype_Component_Transform */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Transform"),
		CTransform::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxPosTex */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_VIBuffer_Rect */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	Safe_AddRef(m_pRenderer);
	
	return S_OK;
}


CMainApp * CMainApp::Create()
{
	CMainApp*	pInstance = new CMainApp();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void Client::CMainApp::Free()
{	
	__super::Free();

	Safe_Release(m_pRenderer);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	Safe_Release(m_pGameInstance);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();


	CGameInstance::Release_Engine();

}

