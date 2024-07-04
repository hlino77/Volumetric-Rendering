#pragma once

#include "Client_Defines.h"
#include "Camera.h"

BEGIN(Client)

class CCamera_Debug final : public CCamera
{
public:
	typedef struct tagCamera_Debug_Desc : public CCamera::CAMERA_DESC
	{
		_float			fMouseSensitive = { 0.0f };		
	}CAMERA_DEBUG_DESC;
private:
	CCamera_Debug(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera_Debug(const CCamera_Debug& rhs);
	virtual ~CCamera_Debug() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;

private:
	
	_float			m_fMouseSensitive = { 0.0f };

public:
	static CCamera_Debug* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END