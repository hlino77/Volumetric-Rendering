#include "..\Public\Frustum.h"
#include "PipeLine.h"

IMPLEMENT_SINGLETON(CFrustum)

CFrustum::CFrustum()
	: m_pPipeLine(CPipeLine::GetInstance())
{
	Safe_AddRef(m_pPipeLine);
}

HRESULT CFrustum::Initialize()
{
	m_vOriginalPoints[0] = Vec3(-1.f, 1.f, 0.f);
	m_vOriginalPoints[1] = Vec3(1.f, 1.f, 0.f);
	m_vOriginalPoints[2] = Vec3(1.f, -1.f, 0.f);
	m_vOriginalPoints[3] = Vec3(-1.f, -1.f, 0.f);

	m_vOriginalPoints[4] = Vec3(-1.f, 1.f, 1.f);
	m_vOriginalPoints[5] = Vec3(1.f, 1.f, 1.f);
	m_vOriginalPoints[6] = Vec3(1.f, -1.f, 1.f);
	m_vOriginalPoints[7] = Vec3(-1.f, -1.f, 1.f);
	
	return S_OK;
}

void CFrustum::Tick()
{
	
	Matrix		ProjMatrixInv = m_pPipeLine->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_PROJ);
	Matrix		ViewMatrixInv = m_pPipeLine->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_VIEW);

	/* 여덟개 점을 월드까지 변환시키자. */
	for (size_t i = 0; i < 8; i++)
	{
		XMStoreFloat3(&m_vPoints[i], XMVector3TransformCoord(XMLoadFloat3(&m_vOriginalPoints[i]), ProjMatrixInv));
		XMStoreFloat3(&m_vPoints[i], XMVector3TransformCoord(XMLoadFloat3(&m_vPoints[i]), ViewMatrixInv));
	}

	/* 월드 상의 평면 여섯개를 구성하자. */
	Make_Planes(m_vPoints, m_WorldPlanes);
}

void CFrustum::Transform_ToLocalSpace(Matrix WorldMatrixInverse)
{
	Vec3		vLocalPoints[8];

	for (size_t i = 0; i < 8; i++)
	{
		/* 월드상에 정보를 담고 있는 점 여덟개에 지형의 월드역행려을 곱해서 지형의 로컬로 옮긴다. */
		XMStoreFloat3(&vLocalPoints[i], XMVector3TransformCoord(XMLoadFloat3(&m_vPoints[i]), WorldMatrixInverse));
	}

	/* 로컬 상의 평면 여섯개를 구성하자. */
	Make_Planes(vLocalPoints, m_LocalPlanes);
}

_bool CFrustum::isIn_Frustum_World(Vec3 vWorldPos, _float fRadius)
{
	/* 0 < ax + by + cz + d */

	/* Plane		: a b c d */
	/* vWorldPos	: x y z 1 */
	for (size_t i = 0; i < 6; i++)
	{
		if (fRadius < XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_WorldPlanes[i]), vWorldPos)))
			return false;		
	}

	return true;
}

_bool CFrustum::isIn_Frustum_Local(Vec3 vLocalPos, _float fRadius)
{
	for (size_t i = 0; i < 6; i++)
	{
		if (fRadius < XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_LocalPlanes[i]), vLocalPos)))
			return false;
	}

	return true;
}

void CFrustum::Make_Planes(const Vec3 * pPoints, Vec4 * pPlanes)
{
	XMStoreFloat4(&pPlanes[0], XMPlaneFromPoints(XMLoadFloat3(&pPoints[1]), XMLoadFloat3(&pPoints[5]), XMLoadFloat3(&pPoints[6])));
	XMStoreFloat4(&pPlanes[1], XMPlaneFromPoints(XMLoadFloat3(&pPoints[4]), XMLoadFloat3(&pPoints[0]), XMLoadFloat3(&pPoints[3])));
	XMStoreFloat4(&pPlanes[2], XMPlaneFromPoints(XMLoadFloat3(&pPoints[4]), XMLoadFloat3(&pPoints[5]), XMLoadFloat3(&pPoints[1])));
	XMStoreFloat4(&pPlanes[3], XMPlaneFromPoints(XMLoadFloat3(&pPoints[3]), XMLoadFloat3(&pPoints[2]), XMLoadFloat3(&pPoints[6])));
	XMStoreFloat4(&pPlanes[4], XMPlaneFromPoints(XMLoadFloat3(&pPoints[5]), XMLoadFloat3(&pPoints[4]), XMLoadFloat3(&pPoints[7])));
	XMStoreFloat4(&pPlanes[5], XMPlaneFromPoints(XMLoadFloat3(&pPoints[0]), XMLoadFloat3(&pPoints[1]), XMLoadFloat3(&pPoints[2])));
}

void CFrustum::Free()
{
	Safe_Release(m_pPipeLine);
}
