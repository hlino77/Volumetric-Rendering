#pragma once

#include "Base.h"

/* 사본 객체들을 리스트에 보관한다. */

BEGIN(Engine)

class CLayer final : public CBase
{
private:
	CLayer();
	virtual ~CLayer() = default;

public:
	class CGameObject* Get_GameObject(_uint iIndex);
	class CGameObject* Get_GameObject(const wstring& strName);

	class CComponent* Get_Component(const wstring& strComponentTag, _uint iIndex);

public:
	HRESULT Initialize();
	HRESULT Add_GameObject(class CGameObject* pGameObject);
	void PriorityTick(_float fTimeDelta);
	void Tick(_float fTimeDelta);
	void LateTick(_float fTimeDelta);

private:
	list<class CGameObject*>	m_GameObjects;

public:
	static CLayer* Create();
	virtual void Free() override;
};

END