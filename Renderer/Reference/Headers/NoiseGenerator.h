#pragma once

#include "Base.h"

BEGIN(Engine)



class ENGINE_DLL CNoiseGenerator : public CBase
{
private:
	CNoiseGenerator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNoiseGenerator(const CNoiseGenerator& rhs); 
	virtual ~CNoiseGenerator() = default;



private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };



public:
	static CNoiseGenerator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END