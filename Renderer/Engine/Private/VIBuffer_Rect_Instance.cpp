#include "..\Public\VIBuffer_Rect_Instance.h"

CVIBuffer_Rect_Instance::CVIBuffer_Rect_Instance(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer_Instancing(pDevice, pContext)
{
}

CVIBuffer_Rect_Instance::CVIBuffer_Rect_Instance(const CVIBuffer_Rect_Instance & rhs)
	: CVIBuffer_Instancing(rhs)
{

}

HRESULT CVIBuffer_Rect_Instance::Initialize_Prototype(const INSTANCE_DESC& InstanceDesc)
{
	/* 인스턴스용 데이터. 셋. */
	m_iNumInstance = InstanceDesc.iNumInstance;
	m_iNumIndicesPerInstance = 6;

	/* 그리기용 정점을 생성하는 파트. */
	m_iStride = sizeof(VTXPOSTEX); /* 정점하나의 크기 .*/
	m_iNumVertices = 4;

	m_iIndexStride = 2; /* 인덱스 하나의 크기. 2 or 4 */
	m_iNumIndices = 6 * m_iNumInstance;
	m_eIndexFormat = m_iIndexStride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
	m_eTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_iNumVBs = 2;

#pragma region VERTEX_BUFFER

	/* 정점버퍼와 인덱스 버퍼를 만드낟. */
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	// m_BufferDesc.ByteWidth = 정점하나의 크기(Byte) * 정점의 갯수;
	m_BufferDesc.ByteWidth = m_iStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT; /* 정적버퍼로 할당한다. (Lock, unLock 호출 불가)*/
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iStride;

	VTXPOSTEX*		pVertices = new VTXPOSTEX[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXPOSTEX) * m_iNumVertices);

	pVertices[0].vPosition = Vec3(-0.5f, 0.5f, 0.f);
	pVertices[0].vTexcoord = Vec2(0.f, 0.f);

	pVertices[1].vPosition = Vec3(0.5f, 0.5f, 0.f);
	pVertices[1].vTexcoord = Vec2(1.f, 0.f);

	pVertices[2].vPosition = Vec3(0.5f, -0.5f, 0.f);
	pVertices[2].vTexcoord = Vec2(1.f, 1.f);

	pVertices[3].vPosition = Vec3(-0.5f, -0.5f, 0.f);
	pVertices[3].vTexcoord = Vec2(0.f, 1.f);

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pVertices;

	if (FAILED(__super::Create_Buffer(&m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

#pragma endregion

#pragma region INDEX_BUFFER

	/* 정점버퍼와 인덱스 버퍼를 만드낟. */
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
	m_BufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT; /* 정적버퍼로 할당한다. (Lock, unLock 호출 불가)*/
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = 0;

	_ushort*		pIndices = new _ushort[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_ushort) * m_iNumIndices);

	_uint		iNumIndices = 0;

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		pIndices[iNumIndices++] = 0;
		pIndices[iNumIndices++] = 1;
		pIndices[iNumIndices++] = 2;

		pIndices[iNumIndices++] = 0;
		pIndices[iNumIndices++] = 2;
		pIndices[iNumIndices++] = 3;
	}	

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pIndices;

	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);
#pragma endregion

	/* 인스턴스 버퍼의 생성에 필요한 초기 정보를 미리 채워놓는다. */
	if (FAILED(__super::Initialize_Prototype(InstanceDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVIBuffer_Rect_Instance::Initialize(void * pArg)
{
#pragma region INSTANCE_BUFFER

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

#pragma endregion

	return S_OK;
}

HRESULT CVIBuffer_Rect_Instance::Update(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource = {};

	

	/* 파티클의 움직임을 부여한다. */
	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		m_pTimeAccs[i] += fTimeDelta;

		((VTXINSTANCE*)SubResource.pData)[i].vTranslation.y += m_pSpeeds[i] * fTimeDelta;

		if (m_pTimeAccs[i] >= m_pLifeTimes[i])
		{
			((VTXINSTANCE*)SubResource.pData)[i].vTranslation.y = m_pVertices[i].vTranslation.y;
			m_pTimeAccs[i] = 0.f;
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);

	return S_OK;
}

CVIBuffer_Rect_Instance * CVIBuffer_Rect_Instance::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const INSTANCE_DESC& InstanceDesc)
{
	CVIBuffer_Rect_Instance*	pInstance = new CVIBuffer_Rect_Instance(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(InstanceDesc)))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Rect_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CVIBuffer_Rect_Instance::Clone(void * pArg)
{
	CVIBuffer_Rect_Instance*	pInstance = new CVIBuffer_Rect_Instance(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CVIBuffer_Rect_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CVIBuffer_Rect_Instance::Free()
{
	__super::Free();


}
