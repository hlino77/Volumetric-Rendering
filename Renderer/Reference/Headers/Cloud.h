#pragma once

#include "Base.h"

BEGIN(Engine)

class ENGINE_DLL CCloud : public CBase
{
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

	ID3D11ShaderResourceView* Get_SRV()
	{
		return m_pShapeSRV;
	}
private:
	HRESULT Ready_For_NoiseTexture3D();


private:

	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	BoundingBox					m_tBoundingBox;

	ID3D11ShaderResourceView*	m_pShapeSRV = { nullptr };
	ID3D11ShaderResourceView*	m_pDetailSRV = { nullptr };

	float						m_fOffset = 0.0f;

public:
	static CCloud* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END