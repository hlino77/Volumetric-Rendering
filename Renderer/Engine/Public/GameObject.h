#pragma once

#include "Base.h"

/* 클라이엉ㄴ트에서 제작할 다양한 게임오브젝트들의 부모가된다. */

BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{
protected:
	/* 원형을 생성할 때 */
	CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	/* 사본을 생성할 때 */
	CGameObject(const CGameObject& rhs); /* 복사 생성자. */
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
	/* 특정 컴포넌트의 검색을 용이하게 하기 위해서. */
	/* 검색을 고려하는 이유?! : 타 객체가 현재 객체의 정보(컴포넌트)를 참조하고자 하는 경우가 빈번하다. */
	map<const wstring, class CComponent*>		m_Components;

protected:
	HRESULT Add_Component(_uint iLevelIndex, const wstring& strPrototypeTag, const wstring& strComponentTag, _Inout_ CComponent** ppOut, void* pArg = nullptr);


	

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END