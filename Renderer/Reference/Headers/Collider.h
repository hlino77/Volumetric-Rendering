#pragma once

/* �浹ü�� ǥ���ϴ� ������Ʈ��. */
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CCollider final : public CComponent
{
public:
	enum TYPE { TYPE_AABB, TYPE_OBB, TYPE_SPHERE, TYPE_END };
private:
	CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCollider(const CCollider& rhs);
	virtual ~CCollider() = default;

public:
	virtual HRESULT Initialize_Prototype(TYPE eColliderType);
	virtual HRESULT Initialize(void* pArg);	

public:
	void Update(_fmatrix TransformMatrix);

	/* �浹�Ѵ�!! */
	_bool isCollision(CCollider* pTargetCollider);
	/*_bool isCollision_AABB();
	_bool isCollision_OBB();*/

#ifdef _DEBUG
public:
	virtual HRESULT Render();
#endif

private:
	TYPE							m_eColliderType = { TYPE_END };
	class CBounding*				m_pBounding = { nullptr };

#ifdef _DEBUG
private:
	/* Vertex, Index Buffer�� �����ϰ� �׸��� ����� �����ϴ� ��ü.  */
	PrimitiveBatch<DirectX::VertexPositionColor>*		m_pBatch = { nullptr };
	BasicEffect*										m_pEffect = { nullptr };	
	ID3D11InputLayout*									m_pInputLayout = { nullptr };
	ID3D11DepthStencilState*							m_pDSState = { nullptr };
	
#endif

public:
	static CCollider* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eColliderType);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END