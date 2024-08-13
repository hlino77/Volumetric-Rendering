#include "stdafx.h"
#include "..\Public\Camera_Debug.h"

#include "GameInstance.h"

CCamera_Debug::CCamera_Debug(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CCamera(pDevice, pContext)
{

}

CCamera_Debug::CCamera_Debug(const CCamera_Debug & rhs)
	: CCamera(rhs)
{

}

HRESULT CCamera_Debug::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera_Debug::Initialize(void * pArg)
{
	CAMERA_DEBUG_DESC*		pCamera_Debug_Desc = (CAMERA_DEBUG_DESC*)pArg;

	m_fMouseSensitive = pCamera_Debug_Desc->fMouseSensitive;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CCamera_Debug::Tick(_float fTimeDelta)
{
	
	CGameInstance*	pGameInstance = GET_INSTANCE(CGameInstance);
	

	if (pGameInstance->Get_DIKeyState(DIK_A) & 0x80)
	{
		m_pTransform->Go_Left(fTimeDelta * 10.0f);
	}
	if (pGameInstance->Get_DIKeyState(DIK_D) & 0x80)
	{
		m_pTransform->Go_Right(fTimeDelta * 10.0f);
	}
	if (pGameInstance->Get_DIKeyState(DIK_W) & 0x80)
	{
		m_pTransform->Go_Straight(fTimeDelta * 10.0f);
	}
	if (pGameInstance->Get_DIKeyState(DIK_S) & 0x80)
	{
		m_pTransform->Go_Backward(fTimeDelta * 10.0f);
	}

	_long	MouseMove = 0l;

	if (pGameInstance->Get_DIKeyState(DIK_LSHIFT) & 0x80)
	{
		if (MouseMove = pGameInstance->Get_DIMouseMove(CInput_Device::MMS_X))
		{
			m_pTransform->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), MouseMove * m_fMouseSensitive * 0.005f);
		}

		if (MouseMove = pGameInstance->Get_DIMouseMove(CInput_Device::MMS_Y))
		{
			m_pTransform->Turn(m_pTransform->Get_State(CTransform::STATE_RIGHT), MouseMove * m_fMouseSensitive * 0.005f);
		}
	}

	

	RELEASE_INSTANCE(CGameInstance);

	__super::Tick(fTimeDelta);
}

void CCamera_Debug::LateTick(_float fTimeDelta)
{
}

CCamera_Debug * CCamera_Debug::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CCamera_Debug*	pInstance = new CCamera_Debug(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CCamera_Debug");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CCamera_Debug::Clone(void* pArg)
{
	CCamera_Debug*	pInstance = new CCamera_Debug(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CCamera_Debug");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_Debug::Free()
{
	__super::Free();

}
