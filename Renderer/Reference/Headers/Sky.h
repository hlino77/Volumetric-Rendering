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
private:
	ID3D11ShaderResourceView*	m_pTransLUTSRV = { nullptr };

	class CShader* m_pShader = { nullptr };
	class CVIBuffer_Rect* m_pVIBuffer = { nullptr };
	class CRenderer* m_pRendererCom = { nullptr };

	Matrix					m_WorldMatrix, m_ViewMatrix, m_ProjMatrix;

	_uint m_iWinSizeX = 1280;
	_uint m_iWinSizeY = 720;

	class CTarget_Manager* m_pTarget_Manager = { nullptr };

	AtmosphereProperties m_tAtmo;
	AtmosphereProperties m_tUnitAtmo;
public:
	virtual CGameObject* Clone(void* pArg) override;
	static CSky* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END