#pragma once

#include "Base.h"

/* aiScene 뼈의 상태를 가지고 있어.  */
/* aiBone(뼈들을 메시 기준으로 다시 재구성하여 저장해놓은 데이터 + 이 뼈는 이 메시의 어떤 정점에게 몇 퍼나 영향을 주는가?! ) */
/* aiNode(뼈들의 상속관계를 표현하기위한 데이터 + 뼈들의 상태행렬) */
/* aiNodeAnim*/

BEGIN(Engine)

class ENGINE_DLL CBone final : public CBase
{
private:
	CBone();
	CBone(const CBone& rhs);
	virtual ~CBone() = default;

public:
	const char* Get_BoneName() const {
		return m_szName;
	}

	_float4x4 Get_CombinedTransformationMatrix() const {
		return m_CombinedTransformationMatrix;
	}

	void Set_TransformationMatrix(_fmatrix TransformationMatrix) {
		XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix);
	}

public:
	HRESULT Initialize(const aiNode* pAINode, _int iParentBoneIndex);
	HRESULT Update_CombinedTransformationMatrix(const vector<class CBone*>& Bones);


private:
	char			m_szName[MAX_PATH] = "";

	/* 부모기준으로 표현된 내 뼈만의 상태변환 행렬. */
	/* 애니메이터 분들이 저장해준 정보대로 갱신해야하는 행렬. */
	_float4x4		m_TransformationMatrix; 

	/* m_TransformationMatrix * 부모행렬(m_CombinedTransformationMatrix)*/
	_float4x4		m_CombinedTransformationMatrix;


	_int			m_iParentBoneIndex = -1;

public:
	static CBone* Create(const aiNode* pAINode, _int iParentBoneIndex);
	CBone* Clone();
	virtual void Free() override;
};

END