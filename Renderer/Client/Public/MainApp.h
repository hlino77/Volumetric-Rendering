#pragma once

#include "Client_Defines.h"


#include "Base.h"

BEGIN(Engine)
class CRenderer;
class CGameInstance;
END

BEGIN(Client)

class CMainApp final : public CBase
{
private:
	CMainApp();
	virtual ~CMainApp() = default;

public:
	/* 내 게임의 시작. */
	HRESULT Initialize();
	void Tick(_float fTimeDelta);
	HRESULT Render();

private:
	CGameInstance*			m_pGameInstance = { nullptr };

	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	CRenderer*				m_pRenderer = { nullptr };

#ifdef _DEBUG
private:
	_tchar					m_szFPS[MAX_PATH] = TEXT("");
	_uint					m_iRenderCount = { 0 };
	_float					m_fTimeAcc = { 0 };

#endif




private:
	HRESULT Open_Level(LEVELID eLevelID);
	HRESULT Ready_Prototype_Components(); 

public:
	static CMainApp* Create();
	virtual void Free() override;
};

END

