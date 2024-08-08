#pragma once

#include "Base.h"

BEGIN(Engine)



class ENGINE_DLL CAerialLUT : public CBase
{
private:
	CAerialLUT(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAerialLUT(const CAerialLUT& rhs); 
	virtual ~CAerialLUT() = default;

private:
	struct GlobalParams
	{
		Matrix matProjInv;
		Matrix matViewInv;
		Vec3 vCamPosition;
		float fPadding1 = 0.0f;
		Vec3 vSunPos;
		float fPadding2 = 0.0f;
	} m_tGlobalParams;


public:
	HRESULT Update_AerialLUT(ID3D11Buffer** pAtmosConstant, ID3D11ShaderResourceView** pTransLUT, const Vec3& vLightDir, ID3D11ShaderResourceView** pMultiScatLUT);

	ID3D11ShaderResourceView* Get_SRV() { return m_pSRV; }
private:
	HRESULT Ready_Resources();
	HRESULT Ready_3DTexture();
	void	Update_Params(const Vec3& vSunPos);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

	ID3D11Texture3D* m_pAerialLUTTexture = { nullptr };

	ID3D11UnorderedAccessView* m_pUAV = { nullptr };
	ID3D11ShaderResourceView* m_pSRV = { nullptr };

	ID3D11ComputeShader* m_pLUTShader = { nullptr };

	int m_iLUTSize = 32;

	D3D11_VIEWPORT m_AerialLUTViewPortDesc;

	ID3D11Buffer* m_pGlobalContantBuffer = { nullptr };
public:
	static CAerialLUT* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END