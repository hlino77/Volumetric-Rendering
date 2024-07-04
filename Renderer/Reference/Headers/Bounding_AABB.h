#pragma once

#include "Bounding.h"

BEGIN(Engine)

class CBounding_AABB final : public CBounding
{
public:
	typedef struct tagBoundingAABBDesc : public CBounding::BOUNDING_DESC
	{
		_float3		vExtents;
	}BOUNDING_AABB_DESC;
private:
	CBounding_AABB();
	virtual ~CBounding_AABB() = default;

public:
	const BoundingBox* Get_Bouding() const {
		return m_pAABB;
	}


public:
	virtual HRESULT Initialize(const BOUNDING_DESC* pDesc) override;
	virtual void Update(_fmatrix TransformMatrix) override;

	virtual _bool isCollision(CCollider::TYPE eType, CBounding* pBouding);

#ifdef _DEBUG
public:
	virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* pBatch) override;
#endif

private:
	BoundingBox*			m_pAABB_Original = { nullptr };
	BoundingBox*			m_pAABB = { nullptr };

public:
	static CBounding_AABB* Create(const BOUNDING_DESC* pDesc);
	virtual void Free() override;
};

END