#include "..\Public\GameInstance.h"
#include "Timer_Manager.h"
#include "Graphic_Device.h"
#include "Level_Manager.h"
#include "Object_Manager.h"
#include "Light_Manager.h"
#include "Target_Manager.h"
#include "Font_Manager.h"
#include "Frustum.h"

IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance()
	: m_pTimer_Manager(CTimer_Manager::GetInstance())
	, m_pGraphic_Device(CGraphic_Device::GetInstance())
	, m_pLevel_Manager(CLevel_Manager::GetInstance())
	, m_pObject_Manager(CObject_Manager::GetInstance())
	, m_pComponent_Manager(CComponent_Manager::GetInstance())
	, m_pPipeLine(CPipeLine::GetInstance())
	, m_pInput_Device(CInput_Device::GetInstance())
	, m_pLight_Manager(CLight_Manager::GetInstance())
	, m_pTarget_Manager(CTarget_Manager::GetInstance())
	, m_pFont_Manager(CFont_Manager::GetInstance())
	, m_pFrustum(CFrustum::GetInstance())
{
	Safe_AddRef(m_pFrustum);
	Safe_AddRef(m_pFont_Manager);
	Safe_AddRef(m_pTarget_Manager);
	Safe_AddRef(m_pLight_Manager);
	Safe_AddRef(m_pInput_Device);
	Safe_AddRef(m_pPipeLine);
	Safe_AddRef(m_pComponent_Manager);
	Safe_AddRef(m_pObject_Manager);
	Safe_AddRef(m_pLevel_Manager);
	Safe_AddRef(m_pGraphic_Device);
	Safe_AddRef(m_pTimer_Manager);
}

HRESULT CGameInstance::Initialize_Engine(_uint iNumLevels, HINSTANCE hInstance, const GRAPHIC_DESC& GraphicDesc, _Inout_ ID3D11Device** ppDevice, _Inout_ ID3D11DeviceContext** ppContext)
{
	/* 그래픽디바이스 초기화 처리. */
	if (FAILED(m_pGraphic_Device->Ready_Graphic_Device(GraphicDesc.hWnd, GraphicDesc.eWinMode, GraphicDesc.iWinSizeX, GraphicDesc.iWinSizeY, ppDevice, ppContext)))
		return E_FAIL;

	/* 사운드디바이스 초기화 처리. */
	/* 입력디바이스 초기화 처리. */
	if (FAILED(m_pInput_Device->Ready_Input_Device(hInstance, GraphicDesc.hWnd)))
		return E_FAIL;

	/* 오브젝트 매니져의 예약 처리. */
	if (FAILED(m_pObject_Manager->Reserve_Manager(iNumLevels)))
		return E_FAIL;

	/* 컴포넌트 매니져의 예약 처리. */
	if (FAILED(m_pComponent_Manager->Reserve_Manager(iNumLevels)))
		return E_FAIL;

	if (FAILED(m_pPipeLine->Initialize()))
		return E_FAIL;

	if (FAILED(m_pFrustum->Initialize()))
		return E_FAIL;

	return S_OK;
}

void CGameInstance::Tick(_float fTimeDelta)
{
	/* 키보드와 마우스의 상태를 저장한다. */
	m_pInput_Device->Update_InputDev();

	m_pObject_Manager->PriorityTick(fTimeDelta);

	m_pObject_Manager->Tick(fTimeDelta);
	m_pLevel_Manager->Tick(fTimeDelta);

	m_pPipeLine->Tick();
	m_pFrustum->Tick();

	m_pObject_Manager->LateTick(fTimeDelta);
	m_pLevel_Manager->LateTick(fTimeDelta);
}

void CGameInstance::Clear(_uint iLevelIndex)
{
	m_pObject_Manager->Clear(iLevelIndex);
	// m_pComponent_Manager->Clear(iLevelIndex);
}

_float CGameInstance::Compute_TimeDelta(const wstring & strTimerTag)
{
	if (nullptr == m_pTimer_Manager)
		return 0.f;

	return m_pTimer_Manager->Compute_TimeDelta(strTimerTag);	
}

HRESULT CGameInstance::Add_Timer(const wstring & strTimerTag)
{
	if (nullptr == m_pTimer_Manager)
		return E_FAIL;

	return m_pTimer_Manager->Add_Timer(strTimerTag);
}

HRESULT CGameInstance::Clear_BackBuffer_View(Vec4 vClearColor)
{
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;
	return m_pGraphic_Device->Clear_BackBuffer_View(vClearColor);
}

HRESULT CGameInstance::Clear_DepthStencil_View()
{

	if (nullptr == m_pGraphic_Device)
		return E_FAIL;
	return m_pGraphic_Device->Clear_DepthStencil_View();
}

HRESULT CGameInstance::Present()
{
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;
	return m_pGraphic_Device->Present();
}

_byte CGameInstance::Get_DIKeyState(_ubyte byKeyID)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->Get_DIKeyState(byKeyID);	
}

_byte CGameInstance::Get_DIMouseState(CInput_Device::MOUSEKEYSTATE eMouse)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->Get_DIMouseState(eMouse);
}

_long CGameInstance::Get_DIMouseMove(CInput_Device::MOUSEMOVESTATE eMouseState)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->Get_DIMouseMove(eMouseState);
}

HRESULT CGameInstance::Open_Level(_uint iLevelIndex, CLevel * pNewLevel)
{
	if (nullptr == m_pLevel_Manager)
		return E_FAIL;
	return m_pLevel_Manager->Open_Level(iLevelIndex, pNewLevel);
}

HRESULT CGameInstance::Add_Prototype(const wstring & strPrototypeTag, CGameObject * pPrototype)
{
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	return m_pObject_Manager->Add_Prototype(strPrototypeTag, pPrototype);	
}

HRESULT CGameInstance::Add_GameObject(_uint iLevelIndex, const wstring & strLayerTag, const wstring & strPrototypeTag, void * pArg)
{
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	return m_pObject_Manager->Add_GameObject(iLevelIndex, strLayerTag, strPrototypeTag, pArg);
}

CGameObject * CGameInstance::Clone_GameObject(const wstring & strPrototypeTag, void * pArg)
{
	if (nullptr == m_pObject_Manager)
		return nullptr;

	return m_pObject_Manager->Clone_GameObject(strPrototypeTag, pArg);
}

CGameObject * CGameInstance::Get_GameObject(_uint iLevelIndex, const wstring & strLayerTag, _uint iIndex)
{
	if (nullptr == m_pObject_Manager)
		return nullptr;

	return m_pObject_Manager->Get_GameObject(iLevelIndex, strLayerTag, iIndex);
}

CComponent * CGameInstance::Get_Component(_uint iLevelIndex, const wstring & strLayerTag, const wstring & strComponentTag, _uint iIndex)
{
	if (nullptr == m_pObject_Manager)
		return nullptr;

	return m_pObject_Manager->Get_Component(iLevelIndex, strLayerTag, strComponentTag, iIndex);
}

HRESULT CGameInstance::Add_Prototype(_uint iLevelIndex, const wstring & strPrototypeTag, CComponent * pPrototype)
{
	if (nullptr == m_pComponent_Manager)
		return E_FAIL;

	return m_pComponent_Manager->Add_Prototype(iLevelIndex, strPrototypeTag, pPrototype);
}

CComponent * CGameInstance::Clone_Component(_uint iLevelIndex, const wstring & strPrototypeTag, void * pArg)
{
	if (nullptr == m_pComponent_Manager)
		return nullptr;

	return m_pComponent_Manager->Clone_Component(iLevelIndex, strPrototypeTag, pArg);	
}

HRESULT CGameInstance::Bind_TransformToShader(CShader * pShader, const char * pConstantName, CPipeLine::TRANSFORMSTATE eState)
{
	return m_pPipeLine->Bind_TransformToShader(pShader, pConstantName, eState);	
}

Vec3 CGameInstance::Get_CamPosition() const
{	
	return m_pPipeLine->Get_CamPosition();
}


const LIGHT_DESC * CGameInstance::Get_LightDesc(_uint iLightIndex)
{
	return m_pLight_Manager->Get_LightDesc(iLightIndex);	
}

HRESULT CGameInstance::Add_Light(const LIGHT_DESC & LightDesc)
{
	return m_pLight_Manager->Add_Light(LightDesc);
}

HRESULT CGameInstance::Bind_SRV(CShader * pShader, const wstring & strTargetTag, const _char * pConstantName)
{
	if (nullptr == m_pTarget_Manager)
		return E_FAIL;
	return m_pTarget_Manager->Bind_SRV(pShader, strTargetTag, pConstantName);
}

HRESULT CGameInstance::Add_Font(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const wstring & strFontTag, const wstring & strFontFilePath)
{
	if (nullptr == m_pFont_Manager)
		return E_FAIL;

	return m_pFont_Manager->Add_Font(pDevice, pContext, strFontTag, strFontFilePath);
}

HRESULT CGameInstance::Render_Font(const wstring & strFontTag, const wstring & strText, const Vec2 & vPos, FXMVECTOR color, float rotation, XMFLOAT2 const & origin, float scale)
{
	if (nullptr == m_pFont_Manager)
		return E_FAIL;

	return m_pFont_Manager->Render(strFontTag, strText, vPos, color, rotation, origin, scale);
}

_bool CGameInstance::isIn_Frustum_World(Vec3 vWorldPos, _float fRadius)
{
	if (nullptr == m_pFrustum)
		return false;

	return m_pFrustum->isIn_Frustum_World(vWorldPos, fRadius);
}

void CGameInstance::Release_Engine()
{
	CGameInstance::GetInstance()->DestroyInstance();

	CFont_Manager::GetInstance()->DestroyInstance();
	CTarget_Manager::GetInstance()->DestroyInstance();
	CLevel_Manager::GetInstance()->DestroyInstance();
	CObject_Manager::GetInstance()->DestroyInstance();
	CComponent_Manager::GetInstance()->DestroyInstance();
	CTimer_Manager::GetInstance()->DestroyInstance();		
	CFrustum::GetInstance()->DestroyInstance();
	CPipeLine::GetInstance()->DestroyInstance();
	CLight_Manager::GetInstance()->DestroyInstance();
	CInput_Device::GetInstance()->DestroyInstance();
	CGraphic_Device::GetInstance()->DestroyInstance();
}

void CGameInstance::Free()
{
	Safe_Release(m_pFrustum);
	Safe_Release(m_pFont_Manager);
	Safe_Release(m_pTarget_Manager);
	Safe_Release(m_pLight_Manager);
	Safe_Release(m_pInput_Device);
	Safe_Release(m_pPipeLine);
	Safe_Release(m_pComponent_Manager);
	Safe_Release(m_pObject_Manager);
	Safe_Release(m_pLevel_Manager);
	Safe_Release(m_pGraphic_Device);
	Safe_Release(m_pTimer_Manager);
}
