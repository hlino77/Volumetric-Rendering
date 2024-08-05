#include "..\Public\AerialLUT.h"
#include "GameInstance.h"



CAerialLUT::CAerialLUT(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CAerialLUT::CAerialLUT(const CAerialLUT & rhs)
	: m_pDevice(rhs.m_pDevice)
	, m_pContext(rhs.m_pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}


HRESULT CAerialLUT::Update_AerialLUT(ID3D11Buffer** pAtmosConstant, ID3D11ShaderResourceView** pTransLUT, const Vec3& vLightDir)
{
	Update_Params(vLightDir);

	m_pContext->CSSetUnorderedAccessViews(0, 1, &m_pUAV, nullptr);
	m_pContext->CSSetShaderResources(0, 1, pTransLUT);

	m_pContext->CSSetConstantBuffers(0, 1, pAtmosConstant);
	m_pContext->CSSetConstantBuffers(1, 1, &m_pGlobalContantBuffer);

	m_pContext->CSSetShader(m_pLUTShader, nullptr, 0);

	XMUINT3 ThreadInput(m_iLUTSize / 8.0f, m_iLUTSize / 8.0f, m_iLUTSize / 8.0f);
	m_pContext->Dispatch(ThreadInput.x, ThreadInput.y, ThreadInput.z);

	ID3D11UnorderedAccessView* pNullUAV = nullptr;
	m_pContext->CSSetUnorderedAccessViews(0, 1, &pNullUAV, nullptr);

	return S_OK;
}

HRESULT CAerialLUT::Ready_Resources()
{
	if (FAILED(Ready_3DTexture()))
	{
		return E_FAIL;
	}

	{
		ID3DBlob* pCSBlob = nullptr;
		ID3DBlob* pErrorBlob = nullptr;

		if (FAILED(D3DCompileFromFile(L"../Bin/ShaderFiles/CShader_AerialLUT.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", 0, 0, &pCSBlob, &pErrorBlob)))
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


	D3D11_BUFFER_DESC tBufferDesc;
	ZeroMemory(&tBufferDesc, sizeof(D3D11_BUFFER_DESC));
	tBufferDesc.ByteWidth = sizeof(GlobalParams);
	tBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA tData;
	ZeroMemory(&tData, sizeof(D3D11_SUBRESOURCE_DATA));
	tData.pSysMem = &m_tGlobalParams;

	if (FAILED(m_pDevice->CreateBuffer(&tBufferDesc, &tData, &m_pGlobalContantBuffer)))
	{
		return E_FAIL;
	}

	return S_OK;
}



HRESULT CAerialLUT::Ready_3DTexture()
{
	{
		D3D11_TEXTURE3D_DESC tDesc = {};
		tDesc.Width = m_iLUTSize;
		tDesc.Height = m_iLUTSize;
		tDesc.Depth = m_iLUTSize;
		tDesc.MipLevels = 1;
		tDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		tDesc.CPUAccessFlags = 0;
		tDesc.MiscFlags = 0;

		if (FAILED(m_pDevice->CreateTexture3D(&tDesc, nullptr, &m_pAerialLUTTexture)))
		{
			return E_FAIL;
		}
	}

	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC tUAVDesc = {};
		tUAVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		tUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
		tUAVDesc.Texture3D.MipSlice = 0;
		tUAVDesc.Texture3D.FirstWSlice = 0;
		tUAVDesc.Texture3D.WSize = 32;

		if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pAerialLUTTexture, &tUAVDesc, &m_pUAV)))
		{
			return E_FAIL;
		}
	}


	{
		
		D3D11_SHADER_RESOURCE_VIEW_DESC tSRVDesc = {};
		tSRVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		tSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		tSRVDesc.Texture3D.MipLevels = 1;
		tSRVDesc.Texture3D.MostDetailedMip = 0;

		if (FAILED(m_pDevice->CreateShaderResourceView(m_pAerialLUTTexture, &tSRVDesc, &m_pSRV)))
		{
			return E_FAIL;
		}
	}



	return S_OK;
}

void CAerialLUT::Update_Params(const Vec3& vLightDir)
{
	CPipeLine* pPipeLine = GET_INSTANCE(CPipeLine);

	m_tGlobalParams.matViewInv = pPipeLine->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_VIEW).Transpose();
	m_tGlobalParams.matProjInv = pPipeLine->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_PROJ).Transpose();
	m_tGlobalParams.vCamPosition = pPipeLine->Get_CamPosition();

	RELEASE_INSTANCE(CPipeLine);

	m_tGlobalParams.vLightDir = vLightDir;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	m_pContext->Map(m_pGlobalContantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &m_tGlobalParams, sizeof(GlobalParams));
	m_pContext->Unmap(m_pGlobalContantBuffer, 0);
}



CAerialLUT* CAerialLUT::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAerialLUT* pInstance = new CAerialLUT(pDevice, pContext);


	if (FAILED(pInstance->Ready_Resources()))
	{
		Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

void CAerialLUT::Free()
{
	__super::Free();

	Safe_Release(m_pLUTShader);
	Safe_Release(m_pAerialLUTTexture);
	Safe_Release(m_pUAV);
	Safe_Release(m_pSRV);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
