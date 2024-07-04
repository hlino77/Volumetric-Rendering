#pragma once

/* 모델의 특정 파츠를 의미하는 메시. */
/* 각 파츠(메시)마다 정점, 인덱스버퍼를 가진다. */
#include "VIBuffer.h"
#include "Model.h"

BEGIN(Engine)

class CMesh final : public CVIBuffer
{
private:
	CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMesh(const CMesh& rhs);
	virtual ~CMesh() = default;

public:
	_uint Get_MaterialIndex() const {
		return m_iMaterialIndex;
	}

public:
	virtual HRESULT Initialize_Prototype(const CModel* pModel, CModel::TYPE eModelType, const aiMesh* pAIMesh, _fmatrix PivotMatrix);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_BoneMatrices(class CShader* pShader, const vector<class CBone*>& Bones, const char* pConstantName, _fmatrix PivotMatrix);
private:
	_char				m_szName[MAX_PATH] = "";
	_uint				m_iMaterialIndex = { 0 };
	_uint				m_iNumBones = { 0 }; /* 이 메시를 구성하는 정점들은 몇개 뼈의 영향을 받는가. */
	vector<_uint>		m_Bones; /* 이 메시는 어떤 뼈들의 영향을 받는가?! 뼈들의 인덱스 == 모델클래스가 들고 있는 뼈배열의 인덱스  */
	vector<_float4x4>	m_OffsetMatrices;

private:
	HRESULT Ready_VertexBuffer_For_NonAnim(const aiMesh* pAIMesh, _fmatrix PivotMatrix);
	HRESULT Ready_VertexBuffer_For_Anim(const CModel* pModel, const aiMesh* pAIMesh);

public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CModel* pModel, CModel::TYPE eModelType, const aiMesh* pAIMesh, _fmatrix PivotMatrix);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END