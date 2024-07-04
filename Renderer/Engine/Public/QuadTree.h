#pragma once

#include "Base.h"

/* 지형을 4분할하는 관계로 저장한다. */
/* 최초 Create를 통해 생성되는 QuadTree 내 지형 전체에 대한 정보를 생성한다. */

BEGIN(Engine)

class CQuadTree final : public CBase
{
public:	
	enum CORNER { CORNER_LT, CORNER_RT, CORNER_RB, CORNER_LB, CORNER_END };
	enum NEIGHBOR { NEIGHBOR_LEFT, NEIGHBOR_TOP, NEIGHBOR_RIGHT, NEIGHBOR_BOTTOM, NEIGHBOR_END };
private:
	CQuadTree();
	virtual ~CQuadTree() = default;

public:
	HRESULT Initialize(_uint iLT, _uint iRT, _uint iRB, _uint iLB);
	void Culling(class CFrustum* pFrustum, const Vec3* pVerticesPos, _uint* pIndices, _uint* pNumIndices);
	void Make_Neighbors();

private:
	_uint					m_iCorners[CORNER_END] = { 0 };
	_uint					m_iCenter = { 0 };
	_float					m_fRadius = { 0.f };
	
	class CQuadTree*		m_pChilds[CORNER_END] = { nullptr, nullptr, nullptr, nullptr };
	class CPipeLine*		m_pPipeLine = { nullptr };

	class CQuadTree*		m_pNeighbors[NEIGHBOR_END] = { nullptr, nullptr, nullptr, nullptr };

private:

	_bool isDraw(const Vec3* pVerticesPos);

public:
	static CQuadTree* Create(_uint iLT, _uint iRT, _uint iRB, _uint iLB);
	virtual void Free() override;
};

END