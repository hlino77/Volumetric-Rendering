#pragma once

#include "GameObject.h"

BEGIN(Engine)

class ENGINE_DLL CSky : public CGameObject
{
private:
	CSky(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSky(const CSky& rhs);
	virtual ~CSky() = default;


public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void PriorityTick(_float fTimeDelta);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();

private:
	HRESULT Ready_For_LUT();
	HRESULT Ready_Components();
	HRESULT Ready_RenderTargets();
	HRESULT Ready_AtmosphereBuffer();
	HRESULT	Ready_Sun();


	void	Update_Sun(_float fTimeDelta);
private:
	ID3D11ShaderResourceView*	m_pTransLUTSRV = { nullptr };

	class CMultiScatLUT* m_pMultiScatLUT = { nullptr };
	class CAerialLUT* m_pAerialLUT = { nullptr };

	class CShader* m_pShader = { nullptr };
	class CVIBuffer_Rect* m_pVIBuffer = { nullptr };
	class CRenderer* m_pRendererCom = { nullptr };

	Matrix					m_WorldMatrix, m_ViewMatrix, m_ProjMatrix;

	_uint m_iWinSizeX = 1280;
	_uint m_iWinSizeY = 720;

	_uint m_iSkyLUTX = 192;
	_uint m_iSkyLUTY = 108;
	D3D11_VIEWPORT		m_SkyLUTViewPortDesc;

	D3D11_VIEWPORT		m_MultiScatLUTViewPortDesc;

	class CTarget_Manager* m_pTarget_Manager = { nullptr };

	AtmosphereProperties m_tUnitAtmo;

	ID3D11Buffer* m_pAtmosphereBuffer = { nullptr };


	Vec3					m_vSunPos;
	Vec3					m_vLightDir;

	//Test
	_bool					m_bKeyPress = false;
	_float					m_fTest = 1.0f;
public:
	virtual CGameObject* Clone(void* pArg) override;
	static CSky* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END