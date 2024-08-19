#pragma once

#include "Client_Defines.h"
#include "Level.h"

BEGIN(Engine)
class CGameObject;
class CSky;
END


BEGIN(Client)

class CLevel_GamePlay final : public CLevel
{
private:
	CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_GamePlay() = default;

public:
	virtual HRESULT Initialize() override;
	virtual HRESULT Tick(_float fTimeDelta) override;
	virtual HRESULT LateTick(_float fTimeDelta) override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_BackGround(const wstring& strLayerTag);

	HRESULT Ready_Layer_Camera(const wstring& strLayerTag);
	


	void	Update_Sky();
	void	Update_Performance();


private:
	_uint m_iFrameCount = 0;
	_float m_fTimer = 1.0f;

	CSky* m_pSky = nullptr;

public:
	static class CLevel_GamePlay* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END