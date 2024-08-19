#pragma once

#include "Base.h"

/* ���ӳ��� ���� ��ü���� ���������� ������ü���� �����Ѵ�.(����) */
/* ���ӳ��� ���� ��ü���� �����Ѵ�.(�纻) */


BEGIN(Engine)

class CObject_Manager final : public CBase
{
	DECLARE_SINGLETON(CObject_Manager)
private:
	CObject_Manager();
	virtual ~CObject_Manager() = default;

public:
	class CGameObject* Get_GameObject(_uint iLevelIndex, const wstring& strLayerTag, _uint iIndex = 0);
	class CGameObject* Find_GameObject(_uint iLevelIndex, const wstring& strLayerTag, const wstring& strName);

	class CComponent* Get_Component(_uint iLevelIndex, const wstring& strLayerTag, const wstring& strComponentTag, _uint iIndex = 0);

public:
	HRESULT Reserve_Manager(_uint iNumLevels);
	HRESULT Add_Prototype(const wstring& strPrototypeTag, class CGameObject* pPrototype);
	HRESULT Add_GameObject(_uint iLevelIndex, const wstring& strLayerTag, const wstring& strPrototypeTag, void* pArg);
	class CGameObject* Clone_GameObject(const wstring& strPrototypeTag, void* pArg);
	void PriorityTick(_float fTimeDelta);
	void Tick(_float fTimeDelta);
	void LateTick(_float fTimeDelta);
	void Clear(_uint iLevelIndex);
private:
	/* ������ü���� �������� �����ұ�?! */
	map<const wstring, class CGameObject*>			m_Prototypes;

private:
	/* �纻��ü���� �������� �׷�(CLayer)��� �����Ѵ�. */
	_uint											m_iNumLevels = { 0 };
	map<const wstring, class CLayer*>*				m_pLayers = { nullptr };
	typedef map<const wstring, class CLayer*>		LAYERS;

private:
	class CGameObject* Find_Prototype(const wstring& strPrototypeTag);
	class CLayer* Find_Layer(_uint iLevelIndex, const wstring& strLayerTag);

public:
	virtual void Free() override;
};

END