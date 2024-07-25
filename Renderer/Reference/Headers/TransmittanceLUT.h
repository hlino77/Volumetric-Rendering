#pragma once

#include "Base.h"

BEGIN(Engine)



class ENGINE_DLL CTransmittanceLUT : public CBase
{
private:
	CTransmittanceLUT(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTransmittanceLUT(const CTransmittanceLUT& rhs); 
	virtual ~CTransmittanceLUT() = default;


public:
	ID3D11ShaderResourceView* Generate_TransmittanceLUT(const AtmosphereProperties& tAtmos);

private:
	HRESULT Ready_Resources();
	HRESULT Ready_ComputeShader();
	HRESULT Ready_2DTexture();

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

	ID3D11Texture2D* m_pTransLUTTexture = { nullptr };
	ID3D11ComputeShader* m_pLUTShader = { nullptr };

public:
	static CTransmittanceLUT* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END