#pragma once

/* ����ü�� �ǹ��ϴ� ��ü. */
/* ����ü == ���������� �ǹ�. */
/* ���ʿ� ���������̽� ���� ����� ������ �� �ִ� ������ ���� ������ ������. */
/* �� �����Ӹ��� �� ������ ���� ���� �����̽��� ��ȯ���ѳ��´�. */

#include "Base.h"

BEGIN(Engine)

class CFrustum final : public CBase
{
	DECLARE_SINGLETON(CFrustum)
private:
	CFrustum();
	virtual ~CFrustum() = default;

public:
	HRESULT Initialize();
	void Tick();
	void Transform_ToLocalSpace(Matrix WorldMatrixInverse);
	_bool isIn_Frustum_World(Vec3 vWorldPos, _float fRadius);
	_bool isIn_Frustum_Local(Vec3 vLocalPos, _float fRadius);

private:
	class CPipeLine*		m_pPipeLine = { nullptr };
	Vec3			m_vOriginalPoints[8];
	Vec3			m_vPoints[8];

	Vec4			m_WorldPlanes[6];
	Vec4			m_LocalPlanes[6];

private:
	void Make_Planes(const Vec3* pPoints, Vec4* pPlanes);

public:
	virtual void Free() override;
};

END