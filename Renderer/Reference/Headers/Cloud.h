#pragma once

#include "GameObject.h"

BEGIN(Engine)

class ENGINE_DLL CCloud : public CGameObject
{
private:
	struct CloudTarget
	{
		wstring szMRT;
		wstring szTarget;
	};


private:
	CCloud(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCloud(const CCloud& rhs);
	virtual ~CCloud() = default;


public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void PriorityTick(_float fTimeDelta);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();

private:
	HRESULT Ready_For_NoiseTexture3D();
	HRESULT Ready_Components();
	HRESULT Ready_RenderTargets();
	HRESULT Ready_UpdatePixel();
private:

	BoundingBox					m_tBoundingBox;

	ID3D11ShaderResourceView*	m_pShapeSRV = { nullptr };
	ID3D11ShaderResourceView*	m_pDetailSRV = { nullptr };

	class CShader* m_pShader = { nullptr };
	class CTexture* m_pBlueNoiseTexture = { nullptr };
	class CTexture* m_pCurlNoiseTexture = { nullptr };
	class CVIBuffer_Rect* m_pVIBuffer = { nullptr };
	class CRenderer* m_pRendererCom = { nullptr };

	Matrix					m_WorldMatrix, m_ViewMatrix, m_ProjMatrix;
	Matrix					m_PrevViewProj;

	class CTarget_Manager* m_pTarget_Manager = { nullptr };

	vector<CloudTarget> m_Targets;
	vector<_uint>	m_UpdatePixel;
	_uint m_iCurrUpdatePixel = 0;
	_uint m_iGridSize = 4;
	_uint m_iWinSizeX = 1280;
	_uint m_iWinSizeY = 720;

	bool m_bSwap = true;
public:
	virtual CGameObject* Clone(void* pArg) override;
	static CCloud* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END