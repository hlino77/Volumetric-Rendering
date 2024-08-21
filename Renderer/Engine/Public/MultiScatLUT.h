#pragma once

#include "Base.h"

BEGIN(Engine)



class ENGINE_DLL CMultiScatLUT : public CBase
{
private:
	CMultiScatLUT(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMultiScatLUT(const CMultiScatLUT& rhs); 
	virtual ~CMultiScatLUT() = default;


public:
	HRESULT Update_MultiScatteringLUT(ID3D11Buffer** pAtmosConstant, ID3D11ShaderResourceView** pTransLUT);

	ID3D11ShaderResourceView* Get_SRV() { return m_pSRV; }
private:
	HRESULT Ready_Resources();
	HRESULT Ready_ComputeShader();
	HRESULT Ready_2DTexture();

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

	ID3D11Texture2D* m_pMultiScatLUTTexture = { nullptr };
	ID3D11ComputeShader* m_pLUTShader = { nullptr };

	ID3D11UnorderedAccessView* m_pUAV = { nullptr };
	ID3D11ShaderResourceView* m_pSRV = { nullptr };

	int m_iLUTSize = 32;

public:
	static CMultiScatLUT* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END