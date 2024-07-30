#include "..\Public\Camera.h"
#include "PipeLine.h"

CCamera::CCamera(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
	, m_pPipeLine(CPipeLine::GetInstance())
{
	Safe_AddRef(m_pPipeLine);
}

CCamera::CCamera(const CCamera & rhs)
	: CGameObject(rhs)
	, m_pPipeLine(rhs.m_pPipeLine)
{
	Safe_AddRef(m_pPipeLine);
}

HRESULT CCamera::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera::Initialize(void * pArg)
{
	CAMERA_DESC*	pCameraDesc = (CAMERA_DESC*)pArg;

	/* 내가 초기에 셋팅하고 싶은 카메라의 상태 설정. */
	/* 저장만했다. */
	m_vEye = pCameraDesc->vEye;
	m_vAt = pCameraDesc->vAt;
	m_fFovy = pCameraDesc->fFovy;
	m_fAspect = pCameraDesc->fAspect;
	m_fNear = pCameraDesc->fNear;
	m_fFar = pCameraDesc->fFar;

	/* 카메라의 상태를 제어한다. */
	/* 원점에 항등상태로 존재한다. */
	m_pTransform = CTransform::Create(m_pDevice, m_pContext);
	if (nullptr == m_pTransform)
		return E_FAIL;

	/* 월드공간에서의 이동, 회전 속도를 셋팅하낟. */
	if (FAILED(m_pTransform->Initialize(pArg)))
		return E_FAIL;

	/* 내가 원하는 카메라의 상태를 CTransform 에게 동기화한다. */
	m_pTransform->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&m_vEye));
	m_pTransform->LookAt(XMLoadFloat4(&m_vAt));
	

	return S_OK;
}

void CCamera::Tick(_float fTimeDelta)
{
	if (nullptr == m_pPipeLine)
		return;

	/* 자식클래스에 필요한 상태 변환을 마쳐놓은 상태다. */

	
	/* 갱신된 정보를 파이프라인에 저장하자. */

	/* 카메라 월드행렬의 역행렬 == 뷰스페이스 변환행렬. */
	m_pPipeLine->Set_Transform(CPipeLine::D3DTS_VIEW, m_pTransform->Get_WorldMatrix_Inverse());
	m_pPipeLine->Set_Transform(CPipeLine::D3DTS_PROJ, XMMatrixPerspectiveFovLH(m_fFovy, m_fAspect, m_fNear, m_fFar));
	
}

void CCamera::LateTick(_float fTimeDelta)
{
}

void CCamera::Free()
{
	__super::Free();

	Safe_Release(m_pTransform);
	Safe_Release(m_pPipeLine);
}
