#include "stdafx.h"
#include "..\Public\Level_GamePlay.h"


#include "LandObject.h"
#include "GameInstance.h"
#include "Camera_Debug.h"

#include "Sky.h"

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
	++m_iFrameCount;
	m_fTimer -= fTimeDelta;

	if (m_fTimer < 0.0f)
	{
		Update_Performance();
		m_fTimer = 1.0f;
		m_iFrameCount = 0;
	}
	



	Update_Sky();
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
	LightDesc.eLightType = LIGHT_DESC::LIGHT_SUN;

	LightDesc.vDiffuse = Vec4(0.5f, 0.5f, 0.5f, 1.f);
	LightDesc.vAmbient = Vec4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = Vec4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vLightPos = Vec4(1.0f, 1.0f, 1.0f, 1.0f);


	if (FAILED(pGameInstance->Add_SunLight(LightDesc)))
		return E_FAIL;


	RELEASE_INSTANCE(CGameInstance);

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_BackGround(const wstring & strLayerTag)
{
	CGameInstance*		pGameInstance = CGameInstance::GetInstance();
	Safe_AddRef(pGameInstance);

	if (FAILED(pGameInstance->Add_GameObject(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Terrain"))))
		return E_FAIL;


	if (FAILED(pGameInstance->Add_GameObject(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Sky"))))
		return E_FAIL;

	m_pSky = dynamic_cast<CSky*>(pGameInstance->Find_GameObject(LEVEL_GAMEPLAY, strLayerTag, L"Sky"));

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

void CLevel_GamePlay::Update_Sky()
{
	ImGui::Begin("Atmosphere");

	AtmosphereProperties tAtmo = m_pSky->Get_Properties(true);


	if(ImGui::Button("Reset", ImVec2(50.0f, 25.0f)))
	{
		m_pSky->Set_Aerial(true);
		m_pSky->Set_PropertiesToStd(AtmosphereProperties());
	}
	else
	{

		_bool bAerial = m_pSky->Is_Aerial();
		ImGui::Checkbox("Aerial PersPective", &bAerial);
		m_pSky->Set_Aerial(bAerial);


		ImGui::SliderFloat("Multi Scattering", &tAtmo.fMultiScatFactor, 0.0f, 1.0f);


		ImGui::SliderFloat("Sun Illuminance", &tAtmo.fSunIlluminance, 0.0f, 150000.0f);

		ImGui::Text("");

		//Rayleigh
		ImGui::DragFloat3("Rayleigh Scattering", &tAtmo.fScatterRayleigh.x, 1.f, 0.0f, 255.0f);
		ImGui::SliderFloat("Rayleigh Height", &tAtmo.fHDensityRayleigh, 0.0f, 50.0f);

		ImGui::Text("");

		//Mie
		ImGui::SliderFloat("Mie Scattering", &tAtmo.fScatterMie, 0.0f, 30.0f);
		ImGui::SliderFloat("Mie Extinction", &tAtmo.fExtinctionMie, 0.0f, 50.0f);
		ImGui::SliderFloat("Mie Height", &tAtmo.fHDensityMie, 0.0f, 40.0f);

		ImGui::Text("");

		//Ozone
		ImGui::DragFloat3("Ozone Absorption", &tAtmo.fAbsorbOzone.x, 1.f, 0.0f, 255.0f);


		m_pSky->Set_PropertiesToStd(tAtmo);
	}
	


	ImGui::End();

	return;
}

void CLevel_GamePlay::Update_Performance()
{
	CGameInstance* pGameInstance = GET_INSTANCE(CGameInstance);

	_uint iFPS = m_iFrameCount;
	_float fFrameMS = 1000.0f / (_float)iFPS;
	
	ImGui::Begin("Performance");
	
	string strFPS = "FPS : ";
	strFPS += to_string(iFPS) + "frame";
	ImGui::Text(strFPS.c_str());

	string strFrameMS = "Frame : ";
	strFrameMS += to_string(fFrameMS) + "ms";
	ImGui::Text(strFrameMS.c_str());
	

	ImGui::End();

	RELEASE_INSTANCE(CGameInstance);
	return;
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
