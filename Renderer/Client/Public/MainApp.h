#pragma once

#include "Client_Defines.h"

/* Ŭ������ ����� ���. */
/* 
class CŬ�����̸�
{
private or protected:
	������. 
	�Ҹ���.
public: 
	Getter 
public:	 
	Setter 
public:
	�Լ�
protected:
	����
protected:
	�Լ�
private:
	����
private:
	�Լ�

public:
	���������� �Լ�. (Create, Clone)	
	���������� �Լ�. (Free)
};
*/

/* �� ������ �������� �ʱ�ȭ(Initialize), ����(Tick), ������(Render)�� �����Ѵ�. */

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
	/* �� ������ ����. */
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
	/* ��緹������ ���Ǵ� ������Ʈ ����. */
	HRESULT Ready_Prototype_Components(); 

	HRESULT Ready_Gara();

public:
	static CMainApp* Create();
	virtual void Free() override;
};

END

