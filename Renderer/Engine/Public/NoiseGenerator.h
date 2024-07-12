#pragma once

#include "Base.h"

BEGIN(Engine)



class ENGINE_DLL CNoiseGenerator : public CBase
{
private:
	CNoiseGenerator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNoiseGenerator(const CNoiseGenerator& rhs); 
	virtual ~CNoiseGenerator() = default;


public:
	ID3D11ShaderResourceView* Generate_ShapeNoise();
	ID3D11ShaderResourceView* Generate_DetailNoise();

private:
	HRESULT Ready_Resources();
	HRESULT Ready_ComputeShader();
	HRESULT Ready_3DTexture();

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

	ID3D11Texture3D* m_pShapeTexture3D = { nullptr };
	ID3D11ComputeShader* m_pShapeShader = { nullptr };

	ID3D11Texture3D* m_pDetailTexture3D = { nullptr };
	ID3D11ComputeShader* m_pDetailShader = { nullptr };

public:
	static CNoiseGenerator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END