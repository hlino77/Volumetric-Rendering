#pragma once

#include "Base.h"

BEGIN(Engine)

class CLight_Manager final : public CBase
{
	DECLARE_SINGLETON(CLight_Manager)
private:
	CLight_Manager();
	virtual ~CLight_Manager() = default;

public:
	const LIGHT_DESC* Get_LightDesc(_uint iLightIndex);

public:
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	HRESULT Add_Sun(const LIGHT_DESC& LightDesc);

	HRESULT Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

	void	Set_SunPos(Vec3 vPos);
	void	Set_TransLUT(ID3D11ShaderResourceView* pSRV);
	void	Set_MultiScatLUT(ID3D11ShaderResourceView* pSRV);
	void	Set_AtmosphereBuffer(ID3D11Buffer* pBuffer);

private:
	list<class CLight*>				m_Lights;
	class CLight* m_pSunLight = nullptr;

public:
	virtual void Free() override;
};

END