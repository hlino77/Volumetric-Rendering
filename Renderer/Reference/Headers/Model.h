#pragma once

/* .FBX파일을 로드하여 데이터들을 내 구조에 맞게 정리한다. */
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CModel final : public CComponent
{
public:
	enum TYPE { TYPE_NONANIM, TYPE_ANIM, TYPE_END };
private:
	CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModel(const CModel& rhs);
	virtual ~CModel() = default;

public:
	_uint Get_NumMeshes() const {
		return m_iNumMeshes;
	}

	_int Get_BoneIndex(const char* pBoneName) const;
	class CBone* Get_BonePtr(const _char* pBoneName) const;
	_float4x4 Get_PivotMatrix() const {
		return m_PivotMatrix;
	}

public:
	virtual HRESULT Initialize_Prototype(TYPE eModelType, const char* pModelFilePath, _fmatrix PivotMatrix);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT SetUp_Animation(_bool isLoop, _uint iAnimationIndex);
	HRESULT Bind_BoneMatrices(class CShader* pShader, _uint iMeshIndex, const char* pConstantName);
	HRESULT Bind_MaterialTexture(class CShader* pShader, const char* pConstantName, _uint iMeshIndex, aiTextureType eType);
	HRESULT Play_Animation(_float fTimeDelta);
	HRESULT Render(_uint iMeshIndex);

private: /* .fbx파일을 열어서 읽어주는 역활 */
	Assimp::Importer			m_Importer; 

	/* m_Importer가 읽어준 데이터들을 보관한다. */
	const aiScene*				m_pAIScene = { nullptr };
private:
	TYPE						m_eModelType = { TYPE_END };

private:
	_uint						m_iNumMeshes = { 0 };
	vector<class CMesh*>		m_Meshes;	

private:
	_uint						m_iNumMaterials = { 0 };
	vector<MESH_MATERIAL>		m_Materials;

	_float4x4					m_PivotMatrix;

private:
	vector<class CBone*>		m_Bones;

private:
	_uint						m_iCurrentAnimIndex = { 0 };
	_uint						m_iNumAnimations = { 0 };
	vector<class CAnimation*>	m_Animations;

private:
	HRESULT Ready_Meshes();
	HRESULT Ready_Materials(const char* pModelFilePath);
	HRESULT Ready_Bones(const aiNode* pAINode, _int iParentBoneIndex);
	HRESULT Ready_Animations();

public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eModelType, const char* pModelFilePath, _fmatrix PivotMatrix = XMMatrixIdentity());
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END