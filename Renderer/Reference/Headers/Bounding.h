#pragma once

/* AABB, OBB, SPHERE 부모가 되는 클래스다. */
#include "Collider.h"

BEGIN(Engine)

class CBounding abstract : public CBase
{
public:
	typedef struct tagBoundingDesc
	{
		_float3			vCenter;
	}BOUNDING_DESC;
protected:
	CBounding();
	virtual ~CBounding() = default;

public:
	virtual HRESULT Initialize(const BOUNDING_DESC* pDesc);
	virtual void Update(_fmatrix TransformMatrix) = 0;

public:
	virtual _bool isCollision(CCollider::TYPE eType, CBounding* pBouding) = 0;

protected:
	_bool				m_isColl = { false };

#ifdef _DEBUG
public:
	virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* pBatch) = 0;
#endif

public:
	virtual void Free();
};

END