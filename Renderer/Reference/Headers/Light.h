#pragma once

#include "Base.h"

BEGIN(Engine)

class CLight final : public CBase
{
private:
	CLight();
	virtual ~CLight() = default;

public:
	const LIGHT_DESC* Get_LightDesc() const {
		return &m_LightDesc;
	} 

public:
	HRESULT Initialize(const LIGHT_DESC& LightDesc);
	HRESULT Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

	void	Set_LightPos(Vec3 vPos)
	{
		m_LightDesc.vLightPos = Vec4(vPos.x, vPos.y, vPos.z, 1.0f);
	}

	void	Set_TransLUT(ID3D11ShaderResourceView* pSRV)
	{
		m_LightDesc.pTransLUT = pSRV;
	}

	void	Set_AtmosphereBuffer(ID3D11Buffer* pBuffer)
	{
		m_LightDesc.pAtmosphereBuffer = pBuffer;
	}

private:
	LIGHT_DESC			m_LightDesc;

public:
	static CLight* Create(const LIGHT_DESC& LightDesc);
	virtual void Free() override;


};

END