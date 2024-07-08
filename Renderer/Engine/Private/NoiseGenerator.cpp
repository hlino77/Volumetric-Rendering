#include "..\Public\NoiseGenerator.h"
#include "GameInstance.h"



CNoiseGenerator::CNoiseGenerator(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
}

CNoiseGenerator::CNoiseGenerator(const CNoiseGenerator & rhs)
{

}



CNoiseGenerator* CNoiseGenerator::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNoiseGenerator* pInstance = new CNoiseGenerator(pDevice, pContext);

	return pInstance;
}

void CNoiseGenerator::Free()
{
	__super::Free();
}
