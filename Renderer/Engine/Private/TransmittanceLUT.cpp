#include "..\Public\TransmittanceLUT.h"
#include "GameInstance.h"



CTransmittanceLUT::CTransmittanceLUT(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CTransmittanceLUT::CTransmittanceLUT(const CTransmittanceLUT & rhs)
	: m_pDevice(rhs.m_pDevice)
	, m_pContext(rhs.m_pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

ID3D11ShaderResourceView* CTransmittanceLUT::Generate_TransmittanceLUT(const AtmosphereProperties& tAtmos)
{
	ID3D11UnorderedAccessView* pUAV = nullptr;
	D3D11_UNORDERED_ACCESS_VIEW_DESC tUAVDesc = {};
	tUAVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	tUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	tUAVDesc.Texture2D.MipSlice = 0;

	if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pTransLUTTexture, &tUAVDesc, &pUAV)))
	{
		return nullptr;
	}

	m_pContext->CSSetUnorderedAccessViews(0, 1, &pUAV, nullptr);
	
	ID3D11Buffer* pContantBuffer = nullptr;

	{
		D3D11_BUFFER_DESC tBufferDesc;
  		ZeroMemory(&tBufferDesc, sizeof(D3D11_BUFFER_DESC));
		tBufferDesc.ByteWidth = sizeof(AtmosphereProperties);
		tBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		tBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		D3D11_SUBRESOURCE_DATA tData;
		ZeroMemory(&tData, sizeof(D3D11_SUBRESOURCE_DATA));
		tData.pSysMem = &tAtmos;
		
		if (FAILED(m_pDevice->CreateBuffer(&tBufferDesc, &tData, &pContantBuffer)))
		{
			return nullptr;
		}
	}

	m_pContext->CSSetConstantBuffers(0, 1, &pContantBuffer);
	m_pContext->CSSetShader(m_pLUTShader, nullptr, 0);


	const int iThreadGroupCountX = m_iLUTSizeX / 16;
	const int iThreadGroupCountY = m_iLUTSizeY / 16;

	m_pContext->Dispatch(iThreadGroupCountX, iThreadGroupCountY, 1);

	ID3D11UnorderedAccessView* pNullUAV = nullptr;
	m_pContext->CSSetUnorderedAccessViews(0, 1, &pNullUAV, nullptr);

	pUAV->Release();
	Safe_Release(m_pLUTShader);
	Safe_Release(pContantBuffer);

	ID3D11ShaderResourceView* pSRV = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC tSRVDesc = {};
	tSRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	tSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	tSRVDesc.Texture2D.MipLevels = 1;
	tSRVDesc.Texture2D.MostDetailedMip = 0;

	if (FAILED(m_pDevice->CreateShaderResourceView(m_pTransLUTTexture, &tSRVDesc, &pSRV)))
	{
		return nullptr;
	}

	return pSRV;
}

HRESULT CTransmittanceLUT::Ready_Resources()
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

HRESULT CTransmittanceLUT::Ready_ComputeShader()
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

HRESULT CTransmittanceLUT::Ready_2DTexture()
{
	{
		D3D11_TEXTURE2D_DESC tDesc = {};
		tDesc.Width = m_iLUTSizeX;
		tDesc.Height = m_iLUTSizeY;
		tDesc.MipLevels = 1;
		tDesc.ArraySize = 1;
		tDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		tDesc.SampleDesc = { 1, 0 };
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		tDesc.CPUAccessFlags = 0;
		tDesc.MiscFlags = 0;

		if (FAILED(m_pDevice->CreateTexture2D(&tDesc, nullptr, &m_pTransLUTTexture)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}



CTransmittanceLUT* CTransmittanceLUT::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTransmittanceLUT* pInstance = new CTransmittanceLUT(pDevice, pContext);


	if (FAILED(pInstance->Ready_Resources()))
	{
		Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

void CTransmittanceLUT::Free()
{
	__super::Free();

	Safe_Release(m_pTransLUTTexture);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
