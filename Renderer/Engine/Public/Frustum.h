#pragma once

/* 절두체를 의미하는 객체. */
/* 절두체 == 여섯개면을 의미. */
/* 최초엔 투영스페이스 상의 평면을 구성할 수 있는 여덟개 점의 정보를 가진다. */
/* 매 프레임마다 이 여덟개 점을 월드 스페이스로 변환시켜놓는다. */

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