#pragma once

#include "Bounding.h"

BEGIN(Engine)

class CBounding_Sphere final : public CBounding
{
public:
	typedef struct tagBoundingSphereDesc : public CBounding::BOUNDING_DESC
	{
		_float		fRadius;		
	}BOUNDING_SPHERE_DESC;

private:
	CBounding_Sphere();
	virtual ~CBounding_Sphere() = default;
public:
	const BoundingSphere* Get_Bouding() const {
		return m_pSphere;
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
	BoundingSphere*			m_pSphere_Original = { nullptr };
	BoundingSphere*			m_pSphere = { nullptr };

public:
	static CBounding_Sphere* Create(const BOUNDING_DESC* pDesc);
	virtual void Free() override;
};

END