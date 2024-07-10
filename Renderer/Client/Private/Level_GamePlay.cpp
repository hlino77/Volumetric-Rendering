#include "stdafx.h"
#include "..\Public\Level_GamePlay.h"

#include "GameInstance.h"
#include "LandObject.h"

#include "Camera_Debug.h"

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CLevel(pDevice, pContext)
{

}

HRESULT CLevel_GamePlay::Initialize()
{
	if (FAILED(Ready_Lights()))
		return E_FAIL;

	if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Tick(_float fTimeDelta)
{
	return S_OK;
}

HRESULT CLevel_GamePlay::LateTick(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("게임플레이 레벨입니다."));

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Lights()
{	
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	LIGHT_DESC			LightDesc;

	/* 방향성 광원을 추가하낟. */
	ZeroMemory(&LightDesc, sizeof LightDesc);
	LightDesc.eLightType = LIGHT_DESC::LIGHT_DIRECTIONAL;
	LightDesc.vLightDir = Vec4(1.f, -1.f, 1.f, 0.f);

	LightDesc.vDiffuse = Vec4(0.5f, 0.5f, 0.5f, 1.f);
	LightDesc.vAmbient = Vec4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = Vec4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	///* 점 광원을 추가한다. */
	//ZeroMemory(&LightDesc, sizeof LightDesc);
	//LightDesc.eLightType = LIGHT_DESC::LIGHT_POINT;
	//LightDesc.vLightPos = Vec4(35.f, 3.f, 35.f, 1.f);
	//LightDesc.fLightRange = 20.f;

	//LightDesc.vDiffuse = Vec4(1.f, 1.f, 1.f, 1.f);
	//LightDesc.vAmbient = Vec4(1.f, 1.f, 1.f, 1.f);
	//LightDesc.vSpecular = LightDesc.vDiffuse;

	//if (FAILED(pGameInstance->Add_Light(LightDesc)))
	//	return E_FAIL;

	//ZeroMemory(&LightDesc, sizeof LightDesc);
	//LightDesc.eLightType = LIGHT_DESC::LIGHT_POINT;
	//LightDesc.vLightPos = Vec4(70.f, 3.f, 35.f, 1.f);
	//LightDesc.fLightRange = 20.f;

	//LightDesc.vDiffuse = Vec4(1.f, 0.0f, 0.f, 1.f);
	//LightDesc.vAmbient = Vec4(1.f, 1.f, 1.f, 1.f);
	//LightDesc.vSpecular = LightDesc.vDiffuse;

	//if (FAILED(pGameInstance->Add_Light(LightDesc)))
	//	return E_FAIL;

	RELEASE_INSTANCE(CGameInstance);

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_BackGround(const wstring & strLayerTag)
{
	/* 원형객체를 복제하여 사본객체를 생성하고 레이어에 추가한다. */
	CGameInstance*		pGameInstance = CGameInstance::GetInstance();
	Safe_AddRef(pGameInstance);

	if (FAILED(pGameInstance->Add_GameObject(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Terrain"))))
		return E_FAIL;

	Safe_Release(pGameInstance);

	return S_OK;
}



HRESULT CLevel_GamePlay::Ready_Layer_Camera(const wstring & strLayerTag)
{
	/* 원형객체를 복제하여 사본객체를 생성하고 레이어에 추가한다. */
	CGameInstance*		pGameInstance = CGameInstance::GetInstance();
	Safe_AddRef(pGameInstance);

	CCamera_Debug::CAMERA_DEBUG_DESC			CameraDebugDesc;
	ZeroMemory(&CameraDebugDesc, sizeof CameraDebugDesc);

	CameraDebugDesc.fMouseSensitive = 0.4f;
	CameraDebugDesc.vEye = Vec4(0.f, 10.f, -8.f, 1.f);
	CameraDebugDesc.vAt = Vec4(0.f, 0.f, 0.f, 1.f);
	CameraDebugDesc.fFovy = XMConvertToRadians(60.0f);
	CameraDebugDesc.fAspect = g_iWinSizeX / (_float)g_iWinSizeY;
	CameraDebugDesc.fNear = 0.2f;
	CameraDebugDesc.fFar = 1000.0f;
	CameraDebugDesc.fSpeedPerSec = 50.f;
	CameraDebugDesc.fRotationRadianPerSec = XMConvertToRadians(90.0f);

	if (FAILED(pGameInstance->Add_GameObject(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Camera_Debug"), &CameraDebugDesc)))
		return E_FAIL;

	Safe_Release(pGameInstance);

	return S_OK;
}



CLevel_GamePlay * CLevel_GamePlay::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CLevel_GamePlay*	pInstance = new CLevel_GamePlay(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_GamePlay");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_GamePlay::Free()
{
	__super::Free();


}
