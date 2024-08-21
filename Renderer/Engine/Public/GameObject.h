#pragma once

#include "Base.h"

/* Ŭ���̾���Ʈ���� ������ �پ��� ���ӿ�����Ʈ���� �θ𰡵ȴ�. */

BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{
protected:
	/* ������ ������ �� */
	CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	/* �纻�� ������ �� */
	CGameObject(const CGameObject& rhs); /* ���� ������. */
	virtual ~CGameObject() = default;

public:
	class CComponent* Find_Component(const wstring& strComponentTag);

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void PriorityTick(_float fTimeDelta);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual void AfterRenderTick();
	virtual HRESULT Render();
	virtual HRESULT Render_LightDepth() { return S_OK; }

	wstring Get_ObjectName() { return m_strName; }

protected:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };


	wstring					m_strName;
protected:
	/* Ư�� ������Ʈ�� �˻��� �����ϰ� �ϱ� ���ؼ�. */
	/* �˻��� ����ϴ� ����?! : Ÿ ��ü�� ���� ��ü�� ����(������Ʈ)�� �����ϰ��� �ϴ� ��찡 ����ϴ�. */
	map<const wstring, class CComponent*>		m_Components;

protected:
	HRESULT Add_Component(_uint iLevelIndex, const wstring& strPrototypeTag, const wstring& strComponentTag, _Inout_ CComponent** ppOut, void* pArg = nullptr);


	

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END