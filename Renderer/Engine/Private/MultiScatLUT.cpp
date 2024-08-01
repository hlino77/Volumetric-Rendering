#include "..\Public\MultiScatLUT.h"
#include "GameInstance.h"



CMultiScatLUT::CMultiScatLUT(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CMultiScatLUT::CMultiScatLUT(const CMultiScatLUT & rhs)
	: m_pDevice(rhs.m_pDevice)
	, m_pContext(rhs.m_pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CMultiScatLUT::Update_MultiScatteringLUT(ID3D11Buffer** pAtmosConstant)
{
	m_pContext->CSSetUnorderedAccessViews(0, 1, &m_pUAV, nullptr);
	
	m_pContext->CSSetConstantBuffers(0, 1, pAtmosConstant);
	m_pContext->CSSetShader(m_pLUTShader, nullptr, 0);

	m_pContext->Dispatch(m_iLUTSize, m_iLUTSize, 1);

	ID3D11UnorderedAccessView* pNullUAV = nullptr;
	m_pContext->CSSetUnorderedAccessViews(0, 1, &pNullUAV, nullptr);

	return S_OK;
}

HRESULT CMultiScatLUT::Ready_Resources()
{
	if (FAILED(Ready_ComputeShader()))
	{
		return E_FAIL;
	}

	if (FAILED(Ready_2DTexture()))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CMultiScatLUT::Ready_ComputeShader()
{
	{
		ID3DBlob* pCSBlob = nullptr;
		ID3DBlob* pErrorBlob = nullptr;

		if (FAILED(D3DCompileFromFile(L"../Bin/ShaderFiles/CShader_TransmittanceLUT.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSTransLUT", "cs_5_0", 0, 0, &pCSBlob, &pErrorBlob)))
		{
			if (pErrorBlob)
			{
				pErrorBlob->Release();
			}

			return E_FAIL;
		}

		if (FAILED(m_pDevice->CreateComputeShader(pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), nullptr, &m_pLUTShader)))
		{
			pCSBlob->Release();
			return E_FAIL;
		}
		pCSBlob->Release();
	}

	return S_OK;
}

HRESULT CMultiScatLUT::Ready_2DTexture()
{
	{
		D3D11_TEXTURE2D_DESC tDesc = {};
		tDesc.Width = m_iLUTSize;
		tDesc.Height = m_iLUTSize;
		tDesc.MipLevels = 1;
		tDesc.ArraySize = 1;
		tDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		tDesc.SampleDesc = { 1, 0 };
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		tDesc.CPUAccessFlags = 0;
		tDesc.MiscFlags = 0;

		if (FAILED(m_pDevice->CreateTexture2D(&tDesc, nullptr, &m_pMultiScatLUTTexture)))
		{
			return E_FAIL;
		}
	}

	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC tUAVDesc = {};
		tUAVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		tUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		tUAVDesc.Texture2D.MipSlice = 0;

		if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pMultiScatLUTTexture, &tUAVDesc, &m_pUAV)))
		{
			return E_FAIL;
		}
	}
	

	{
		
		D3D11_SHADER_RESOURCE_VIEW_DESC tSRVDesc = {};
		tSRVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		tSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		tSRVDesc.Texture2D.MipLevels = 1;
		tSRVDesc.Texture2D.MostDetailedMip = 0;

		if (FAILED(m_pDevice->CreateShaderResourceView(m_pMultiScatLUTTexture, &tSRVDesc, &m_pSRV)))
		{
			return E_FAIL;
		}
	}

	

	return S_OK;
}



CMultiScatLUT* CMultiScatLUT::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMultiScatLUT* pInstance = new CMultiScatLUT(pDevice, pContext);


	if (FAILED(pInstance->Ready_Resources()))
	{
		Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

void CMultiScatLUT::Free()
{
	__super::Free();

	Safe_Release(m_pLUTShader);
	Safe_Release(m_pMultiScatLUTTexture);
	Safe_Release(m_pUAV);
	Safe_Release(m_pSRV);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
