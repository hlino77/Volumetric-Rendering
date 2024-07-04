#pragma once

#include "Base.h"

/* aiScene ���� ���¸� ������ �־�.  */
/* aiBone(������ �޽� �������� �ٽ� �籸���Ͽ� �����س��� ������ + �� ���� �� �޽��� � �������� �� �۳� ������ �ִ°�?! ) */
/* aiNode(������ ��Ӱ��踦 ǥ���ϱ����� ������ + ������ �������) */
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

	/* �θ�������� ǥ���� �� ������ ���º�ȯ ���. */
	/* �ִϸ����� �е��� �������� ������� �����ؾ��ϴ� ���. */
	_float4x4		m_TransformationMatrix; 

	/* m_TransformationMatrix * �θ����(m_CombinedTransformationMatrix)*/
	_float4x4		m_CombinedTransformationMatrix;


	_int			m_iParentBoneIndex = -1;

public:
	static CBone* Create(const aiNode* pAINode, _int iParentBoneIndex);
	CBone* Clone();
	virtual void Free() override;
};

END