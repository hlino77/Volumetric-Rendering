#include "..\Public\Transform.h"
#include "Shader.h"

CTransform::CTransform(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent(pDevice, pContext)
{

}

CTransform::CTransform(const CTransform & rhs)
	: CComponent(rhs)
	, m_WorldMatrix(rhs.m_WorldMatrix)
{

}

Vec3 CTransform::Get_Scaled()
{
	
	return Vec3(XMVectorGetX(XMVector3Length(Get_State(STATE_RIGHT))),
		XMVectorGetX(XMVector3Length(Get_State(STATE_UP))),
		XMVectorGetX(XMVector3Length(Get_State(STATE_LOOK))));
}

void CTransform::Set_State(STATE eState, Vec3 vState)
{
	memcpy(&m_WorldMatrix.m[eState], &vState, sizeof(Vec3));
}

void CTransform::Set_Scaling(const Vec3 & vScale)
{
	Vec3		vRight = Get_State(STATE_RIGHT);
	Vec3		vUp = Get_State(STATE_UP);
	Vec3		vLook = Get_State(STATE_LOOK);

	Set_State(STATE_RIGHT, XMVector3Normalize(vRight) * vScale.x);
	Set_State(STATE_UP, XMVector3Normalize(vUp) * vScale.y);
	Set_State(STATE_LOOK, XMVector3Normalize(vLook) * vScale.z);
}

HRESULT CTransform::Initialize_Prototype()
{
	/* XMFloat4x4 -> XMMatrix*/
	/*XMLoadFloat4x4(&m_WorldMatrix);*/

	/* XMMatrix -> XMFloat4x4*/
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());	

	/*_float3		vTmp;
	_vector		vTmp1;

	vTmp1 = XMLoadFloat3(&vTmp);

	XMStoreFloat3(&vTmp, vTmp1);*/


	return S_OK;
}

HRESULT CTransform::Initialize(void * pArg)
{
	if (nullptr != pArg)
	{	
		TRANSFORM_DESC	TransformDesc;

		memmove(&TransformDesc, pArg, sizeof TransformDesc);

		m_fSpeedPerSec = TransformDesc.fSpeedPerSec;
		m_fRotationRadianPerSec = TransformDesc.fRotationRadianPerSec;
	}

	return S_OK;
}

HRESULT CTransform::Bind_ShaderResources(CShader * pShader, const char * pConstantName)
{
	return pShader->Bind_Matrix(pConstantName, &m_WorldMatrix);	
}

void CTransform::Go_Straight(_float fTimeDelta)
{
	Vec4		vLook = Get_State(STATE_LOOK);

	Vec3		vPosition = Get_State(STATE_POSITION);

	vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Go_Backward(_float fTimeDelta)
{
	Vec3		vLook = Get_State(STATE_LOOK);

	Vec3		vPosition = Get_State(STATE_POSITION);

	vPosition -= XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Go_Left(_float fTimeDelta)
{
	Vec3		vRight = Get_State(STATE_RIGHT);

	Vec3		vPosition = Get_State(STATE_POSITION);

	vPosition -= XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Go_Right(_float fTimeDelta)
{
	Vec3		vRight = Get_State(STATE_RIGHT);

	Vec3		vPosition = Get_State(STATE_POSITION);

	vPosition += XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Fix_Rotation(Vec3 vAxis, _float fRadian)
{
	Vec3		vScaled = Get_Scaled();

	/* 항등상태 기준으로 정해준 각도만큼 회전시켜놓는다. */
	/* Right, Up, Look를 회전시킨다. */
	Vec3		vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScaled.x;
	Vec3		vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScaled.y;
	Vec3		vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScaled.z;

	Matrix		RotationMatrix = XMMatrixRotationAxis(vAxis, fRadian);

	vRight = XMVector4Transform(vRight, RotationMatrix);
	vUp = XMVector4Transform(vUp, RotationMatrix);
	vLook = XMVector4Transform(vLook, RotationMatrix);
	/*XMVector3TransformNormal();
	XMVector3TransformCoord();*/

	Set_State(STATE_RIGHT, vRight);
	Set_State(STATE_UP, vUp);
	Set_State(STATE_LOOK, vLook);
}

void CTransform::Turn(Vec3 vAxis, _float fTimeDelta)
{
	/* 현재 상태기준 정해준 각도만큼 회전시켜놓는다. */
	Vec3		vRight = Get_State(STATE_RIGHT);
	Vec3		vUp = Get_State(STATE_UP);
	Vec3		vLook = Get_State(STATE_LOOK);

	Matrix		RotationMatrix = XMMatrixRotationAxis(vAxis, m_fRotationRadianPerSec * fTimeDelta);

	vRight = XMVector4Transform(vRight, RotationMatrix);
	vUp = XMVector4Transform(vUp, RotationMatrix);
	vLook = XMVector4Transform(vLook, RotationMatrix);

	Set_State(STATE_RIGHT, vRight);
	Set_State(STATE_UP, vUp);
	Set_State(STATE_LOOK, vLook);
}

void CTransform::LookAt(Vec3 vPoint)
{
	Vec3		vScaled = Get_Scaled();

	Vec3		vPosition = Get_State(STATE_POSITION);
	Vec3		vLook = XMVector3Normalize(vPoint - vPosition) * vScaled.z;
	Vec3		vRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook)) * vScaled.x;
	Vec3		vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight)) * vScaled.y;

	Set_State(STATE_RIGHT, vRight);
	Set_State(STATE_UP, vUp);
	Set_State(STATE_LOOK, vLook);
}

void CTransform::Chase(Vec3 vPoint, _float fTimeDelta, _float fMargin)
{
	Vec3		vPosition = Get_State(STATE_POSITION);

	Vec3		vDir = vPoint - vPosition;

	if(XMVectorGetX(XMVector3Length(vDir)) > fMargin)
		vPosition += XMVector3Normalize(vDir) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);
}

CTransform * CTransform::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTransform*	pInstance = new CTransform(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTransform");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CTransform::Clone(void * pArg)
{
	CTransform*	pInstance = new CTransform(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTransform");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTransform::Free()
{
	__super::Free();

}
