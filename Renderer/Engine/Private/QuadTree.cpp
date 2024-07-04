#include "..\Public\QuadTree.h"
#include "Frustum.h"
#include "PipeLine.h"

CQuadTree::CQuadTree()
	: m_pPipeLine(CPipeLine::GetInstance())
{
	Safe_AddRef(m_pPipeLine);

}

HRESULT CQuadTree::Initialize(_uint iLT, _uint iRT, _uint iRB, _uint iLB)
{
	m_iCorners[CORNER_LT] = iLT;
	m_iCorners[CORNER_RT] = iRT;
	m_iCorners[CORNER_RB] = iRB;
	m_iCorners[CORNER_LB] = iLB;

	if (1 == iRT - iLT)
		return S_OK;

	m_iCenter = (iLT + iRB) >> 1;

	_uint		iLC, iTC, iRC, iBC;

	iLC = (iLT + iLB) >> 1;
	iTC = (iLT + iRT) >> 1;
	iRC = (iRT + iRB) >> 1;
	iBC = (iLB + iRB) >> 1;

	m_pChilds[CORNER_LT] = CQuadTree::Create(iLT, iTC, m_iCenter, iLC);
	m_pChilds[CORNER_RT] = CQuadTree::Create(iTC, iRT, iRC, m_iCenter);
	m_pChilds[CORNER_RB] = CQuadTree::Create(m_iCenter, iRC, iRB, iBC);
	m_pChilds[CORNER_LB] = CQuadTree::Create(iLC, m_iCenter, iBC, iLB);



	return S_OK;
}

void CQuadTree::Culling(CFrustum * pFrustum, const Vec3* pVerticesPos, _uint* pIndices, _uint* pNumIndices)
{
	if (nullptr == m_pChilds[CORNER_LT] || 
		true == isDraw(pVerticesPos))
	{
		_bool		isIn[4] = {
			pFrustum->isIn_Frustum_Local(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_LT]]), 0.f),
			pFrustum->isIn_Frustum_Local(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_RT]]), 0.f),
			pFrustum->isIn_Frustum_Local(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_RB]]), 0.f),
			pFrustum->isIn_Frustum_Local(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_LB]]), 0.f),
		};

		_bool		isDraw[NEIGHBOR_END] = { true, true, true, true }; 

		for (size_t i = 0; i < NEIGHBOR_END; i++)
		{
			if (nullptr != m_pNeighbors[i])
				isDraw[i] = m_pNeighbors[i]->isDraw(pVerticesPos);
		}

		if (true == isDraw[NEIGHBOR_LEFT] &&
			true == isDraw[NEIGHBOR_TOP] &&
			true == isDraw[NEIGHBOR_RIGHT] &&
			true == isDraw[NEIGHBOR_BOTTOM])
		{
			if (true == isIn[0] ||
				true == isIn[1] ||
				true == isIn[2])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
			}

			/* 왼쪽 아래 삼각형의 세 점중 하나가 절두체 안에 있냐 */
			if (true == isIn[0] ||
				true == isIn[2] ||
				true == isIn[3])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
			}

			return;
		}

		_uint		iLC, iTC, iRC, iBC;

		iLC = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_LB]) >> 1;
		iTC = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_RT]) >> 1;
		iRC = (m_iCorners[CORNER_RT] + m_iCorners[CORNER_RB]) >> 1;
		iBC = (m_iCorners[CORNER_LB] + m_iCorners[CORNER_RB]) >> 1;

		if (true == isIn[0] ||
			true == isIn[2] ||
			true == isIn[3])
		{
			if (false == isDraw[NEIGHBOR_LEFT])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iLC;

				pIndices[(*pNumIndices)++] = iLC;
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
			}
			else
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
			}

			if (false == isDraw[NEIGHBOR_BOTTOM])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iBC;

				pIndices[(*pNumIndices)++] = iBC;
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
			}
			else
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
			}
		}

		if (true == isIn[0] ||
			true == isIn[1] ||
			true == isIn[2])
		{
			if (false == isDraw[NEIGHBOR_RIGHT])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
				pIndices[(*pNumIndices)++] = iRC;
				pIndices[(*pNumIndices)++] = m_iCenter;

				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iRC;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
			}
			else
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
				pIndices[(*pNumIndices)++] = m_iCenter;
			}

			if (false == isDraw[NEIGHBOR_TOP])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = iTC;
				pIndices[(*pNumIndices)++] = m_iCenter;

				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iTC;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
			}
			else
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
				pIndices[(*pNumIndices)++] = m_iCenter;
			}
		}

		return;	
	}

	_float	fRadius = XMVectorGetX(XMVector3Length(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_LT]]) - XMLoadFloat3(&pVerticesPos[m_iCenter])));

	if (true == pFrustum->isIn_Frustum_Local(XMLoadFloat3(&pVerticesPos[m_iCenter]), fRadius))
	{
		for (size_t i = 0; i < CORNER_END; i++)
		{
			if (nullptr != m_pChilds[i])
				m_pChilds[i]->Culling(pFrustum, pVerticesPos, pIndices, pNumIndices);
		}
	}


}


void CQuadTree::Make_Neighbors()
{
	if (nullptr == m_pChilds[CORNER_LT]->m_pChilds[CORNER_LT])
		return;

	m_pChilds[CORNER_LT]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pChilds[CORNER_RT];
	m_pChilds[CORNER_LT]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pChilds[CORNER_LB];

	m_pChilds[CORNER_RT]->m_pNeighbors[NEIGHBOR_LEFT] = m_pChilds[CORNER_LT];
	m_pChilds[CORNER_RT]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pChilds[CORNER_RB];

	m_pChilds[CORNER_RB]->m_pNeighbors[NEIGHBOR_LEFT] = m_pChilds[CORNER_LB];
	m_pChilds[CORNER_RB]->m_pNeighbors[NEIGHBOR_TOP] = m_pChilds[CORNER_RT];

	m_pChilds[CORNER_LB]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pChilds[CORNER_RB];
	m_pChilds[CORNER_LB]->m_pNeighbors[NEIGHBOR_TOP] = m_pChilds[CORNER_LT];

	if (nullptr != m_pNeighbors[NEIGHBOR_RIGHT])
	{
		m_pChilds[CORNER_RT]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pNeighbors[NEIGHBOR_RIGHT]->m_pChilds[CORNER_LT];
		m_pChilds[CORNER_RB]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pNeighbors[NEIGHBOR_RIGHT]->m_pChilds[CORNER_LB];
	}

	if (nullptr != m_pNeighbors[NEIGHBOR_BOTTOM])
	{
		m_pChilds[CORNER_LB]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pNeighbors[NEIGHBOR_BOTTOM]->m_pChilds[CORNER_LT];
		m_pChilds[CORNER_RB]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pNeighbors[NEIGHBOR_BOTTOM]->m_pChilds[CORNER_RT];
	}

	if (nullptr != m_pNeighbors[NEIGHBOR_LEFT])
	{
		m_pChilds[CORNER_LT]->m_pNeighbors[NEIGHBOR_LEFT] = m_pNeighbors[NEIGHBOR_LEFT]->m_pChilds[CORNER_RT];
		m_pChilds[CORNER_LB]->m_pNeighbors[NEIGHBOR_LEFT] = m_pNeighbors[NEIGHBOR_LEFT]->m_pChilds[CORNER_RB];
	}
	if (nullptr != m_pNeighbors[NEIGHBOR_TOP])
	{
		m_pChilds[CORNER_LT]->m_pNeighbors[NEIGHBOR_TOP] = m_pNeighbors[NEIGHBOR_TOP]->m_pChilds[CORNER_LB];
		m_pChilds[CORNER_RT]->m_pNeighbors[NEIGHBOR_TOP] = m_pNeighbors[NEIGHBOR_TOP]->m_pChilds[CORNER_RB];
	}

	for (size_t i = 0; i < CORNER_END; i++)
	{
		m_pChilds[i]->Make_Neighbors();
	}	
}

_bool CQuadTree::isDraw(const Vec3 * pVerticesPos)
{
	_float		fCamDistance = XMVectorGetX(XMVector3Length(m_pPipeLine->Get_CamPosition() - pVerticesPos[m_iCenter]));
	_float		fWidth = m_iCorners[CORNER_RT] - m_iCorners[CORNER_LT];

	if (fCamDistance * 0.2f >= fWidth)
		return true;

	return false;
}

CQuadTree * CQuadTree::Create(_uint iLT, _uint iRT, _uint iRB, _uint iLB)
{
	CQuadTree*	pInstance = new CQuadTree();

	if (FAILED(pInstance->Initialize(iLT, iRT, iRB, iLB)))
	{
		MSG_BOX("Failed to Created : CQuadTree");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CQuadTree::Free()
{
	Safe_Release(m_pPipeLine);

	for (size_t i = 0; i < CORNER_END; i++)	
		Safe_Release(m_pChilds[i]);
}
