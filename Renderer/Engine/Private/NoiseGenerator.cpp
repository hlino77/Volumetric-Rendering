#include "..\Public\NoiseGenerator.h"
#include "GameInstance.h"



CNoiseGenerator::CNoiseGenerator(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CNoiseGenerator::CNoiseGenerator(const CNoiseGenerator & rhs)
	: m_pDevice(rhs.m_pDevice)
	, m_pContext(rhs.m_pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

ID3D11ShaderResourceView* CNoiseGenerator::Generate_Perlin_Worley()
{
	if (FAILED(Ready_ComputeShader()))
	{
		return nullptr;
	}
	
	if (FAILED(Ready_3DTexture()))
	{
		return nullptr;
	}

	ID3D11UnorderedAccessView* pUAV = nullptr;
	D3D11_UNORDERED_ACCESS_VIEW_DESC tUAVDesc = {};
	tUAVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	tUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
	tUAVDesc.Texture3D.MipSlice = 0;
	tUAVDesc.Texture3D.FirstWSlice = 0;
	tUAVDesc.Texture3D.WSize = 128;

	if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pTexture3D, &tUAVDesc, &pUAV)))
	{
		return nullptr;
	}

	m_pContext->CSSetUnorderedAccessViews(0, 1, &pUAV, nullptr);
	m_pContext->CSSetShader(m_pNoiseShader, nullptr, 0);

	XMUINT3 ThreadInput(128 / 8.0f, 128 / 8.0f, 128 / 8.0f);
	m_pContext->Dispatch(ThreadInput.x, ThreadInput.y, ThreadInput.z);

	ID3D11UnorderedAccessView* pNullUAV = nullptr;
	m_pContext->CSSetUnorderedAccessViews(0, 1, &pNullUAV, nullptr);

	pUAV->Release();
	Safe_Release(m_pNoiseShader);


	ID3D11ShaderResourceView* pSRV = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC tSRVDesc = {};
	tSRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	tSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	tSRVDesc.Texture3D.MostDetailedMip = 0;
	tSRVDesc.Texture3D.MipLevels = 1;

	if (FAILED(m_pDevice->CreateShaderResourceView(m_pTexture3D, &tSRVDesc, &pSRV)))
	{
		return nullptr;
	}

	return pSRV;
}

HRESULT CNoiseGenerator::Ready_ComputeShader()
{
	ID3DBlob* pCSBlob = nullptr;
	ID3DBlob* pErrorBlob = nullptr;

	if (FAILED(D3DCompileFromFile(L"../Bin/ShaderFiles/CShader_PerlinWorley.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", 0, 0, &pCSBlob, &pErrorBlob)))
	{
		if (pErrorBlob)
		{
			pErrorBlob->Release();
		}

		return E_FAIL;
	}

	if (FAILED(m_pDevice->CreateComputeShader(pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), nullptr, &m_pNoiseShader)))
	{
		pCSBlob->Release();
		return E_FAIL;
	}
	pCSBlob->Release();

	return S_OK;
}

HRESULT CNoiseGenerator::Ready_3DTexture()
{
	D3D11_TEXTURE3D_DESC tDesc = {};
	tDesc.Width = 128;
	tDesc.Height = 128;
	tDesc.Depth = 128;
	tDesc.MipLevels = 1;
	tDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	tDesc.Usage = D3D11_USAGE_DEFAULT;
	tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	if (FAILED(m_pDevice->CreateTexture3D(&tDesc, nullptr, &m_pTexture3D)))
	{
		return E_FAIL;
	}

	return S_OK;
}



CNoiseGenerator* CNoiseGenerator::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNoiseGenerator* pInstance = new CNoiseGenerator(pDevice, pContext);

	return pInstance;
}

void CNoiseGenerator::Free()
{
	__super::Free();

	Safe_Release(m_pTexture3D);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
