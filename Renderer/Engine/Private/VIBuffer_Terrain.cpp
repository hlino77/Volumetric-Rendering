#include "..\Public\VIBuffer_Terrain.h"
#include "Transform.h"
#include "Frustum.h"

CVIBuffer_Terrain::CVIBuffer_Terrain(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer(pDevice, pContext)
	, m_pFrustum(CFrustum::GetInstance())
{
	Safe_AddRef(m_pFrustum);
}

CVIBuffer_Terrain::CVIBuffer_Terrain(const CVIBuffer_Terrain & rhs)
	: CVIBuffer(rhs)
	, m_iNumVerticesX(rhs.m_iNumVerticesX)
	, m_iNumVerticesZ(rhs.m_iNumVerticesZ)
	, m_pFrustum(rhs.m_pFrustum)
{
	Safe_AddRef(m_pFrustum);

}

HRESULT CVIBuffer_Terrain::Initialize_Prototype(const wstring& strHeightMapFilePath)
{
	/* 받아온 하이트맵 이미지의 정보를 읽어오자. */	


	_ulong		dwByte = 0;
	HANDLE		hFile = CreateFile(strHeightMapFilePath.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	BITMAPFILEHEADER		fh;
	BITMAPINFOHEADER		ih;

	ReadFile(hFile, &fh, sizeof(fh), &dwByte, nullptr);
	ReadFile(hFile, &ih, sizeof(ih), &dwByte, nullptr);

	_ulong*					pPixel = { nullptr };
	pPixel = new _ulong[ih.biWidth * ih.biHeight];

	ReadFile(hFile, pPixel, sizeof(_ulong) * ih.biWidth * ih.biHeight, &dwByte, nullptr);

	CloseHandle(hFile);

	m_iNumVerticesX = ih.biWidth;
	m_iNumVerticesZ = ih.biHeight;
	m_iStride = sizeof(VTXNORTEX); /* 정점하나의 크기 .*/
	m_iNumVertices = m_iNumVerticesX * m_iNumVerticesZ;

	m_iIndexStride = 4; /* 인덱스 하나의 크기. 2 or 4 */
	m_iNumIndices = (m_iNumVerticesX - 1) * (m_iNumVerticesZ - 1) * 2 * 3;
	m_eIndexFormat = m_iIndexStride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
	m_eTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_iNumVBs = 1;

#pragma region VERTEX_BUFFER
/*
	0x000000ff

	11111111 11101110 11101110 11101110
&	00000000 11111111 00000000 00000000
	00000000 11101110 00000000 00000000*/
	

	VTXNORTEX*		pVertices = new VTXNORTEX[m_iNumVertices];
	m_pVerticesPos = new Vec3[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXNORTEX) * m_iNumVertices);
	ZeroMemory(m_pVerticesPos, sizeof(Vec3) * m_iNumVertices);

	for (size_t i = 0; i < m_iNumVerticesZ; i++)
	{
		for (size_t j = 0; j < m_iNumVerticesX; j++)
		{
			_uint		iIndex = i * m_iNumVerticesX + j;

			m_pVerticesPos[iIndex] = pVertices[iIndex].vPosition = Vec3(j, (pPixel[iIndex] & 0x000000ff) / 10.f, i);
			pVertices[iIndex].vNormal = Vec3(0.f, 0.f, 0.f);
			pVertices[iIndex].vTexcoord = Vec2(j / (m_iNumVerticesX - 1.f), i / (m_iNumVerticesZ - 1.f));
		}
	}	

	Safe_Delete_Array(pPixel);

#pragma endregion

#pragma region INDEX_BUFFER
	_uint*		pIndices = new _uint[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_uint) * m_iNumIndices);

	_uint		iNumIndices = 0;

	for (size_t i = 0; i < m_iNumVerticesZ - 1; i++)
	{
		for (size_t j = 0; j < m_iNumVerticesX - 1; j++)
		{
			_uint		iIndex = i * m_iNumVerticesX + j;

			_uint		iIndices[4] = {
				iIndex + m_iNumVerticesX,		//5
				iIndex + m_iNumVerticesX + 1,	//6
				iIndex + 1,						//1
				iIndex							//0
			};

			Vec3		vSourDir, vDestDir, vNormal;

			pIndices[iNumIndices++] = iIndices[0];
			pIndices[iNumIndices++] = iIndices[1];
			pIndices[iNumIndices++] = iIndices[2];

			vSourDir = XMLoadFloat3(&pVertices[iIndices[1]].vPosition) -
				XMLoadFloat3(&pVertices[iIndices[0]].vPosition);

			vDestDir = XMLoadFloat3(&pVertices[iIndices[2]].vPosition) -
				XMLoadFloat3(&pVertices[iIndices[1]].vPosition);

			vNormal = XMVector3Normalize(XMVector3Cross(vSourDir, vDestDir));

			XMStoreFloat3(&pVertices[iIndices[0]].vNormal, 
				XMVector3Normalize(pVertices[iIndices[0]].vNormal + vNormal));
			XMStoreFloat3(&pVertices[iIndices[1]].vNormal,
				XMVector3Normalize(pVertices[iIndices[1]].vNormal + vNormal));
			XMStoreFloat3(&pVertices[iIndices[2]].vNormal,
				XMVector3Normalize(pVertices[iIndices[2]].vNormal + vNormal));

			pIndices[iNumIndices++] = iIndices[0];
			pIndices[iNumIndices++] = iIndices[2];
			pIndices[iNumIndices++] = iIndices[3];

			vSourDir = XMLoadFloat3(&pVertices[iIndices[2]].vPosition) -
				XMLoadFloat3(&pVertices[iIndices[0]].vPosition);

			vDestDir = XMLoadFloat3(&pVertices[iIndices[3]].vPosition) -
				XMLoadFloat3(&pVertices[iIndices[2]].vPosition);

			vNormal = XMVector3Normalize(XMVector3Cross(vSourDir, vDestDir));

			XMStoreFloat3(&pVertices[iIndices[0]].vNormal,
				XMVector3Normalize(pVertices[iIndices[0]].vNormal + vNormal));
			XMStoreFloat3(&pVertices[iIndices[2]].vNormal,
				XMVector3Normalize(pVertices[iIndices[2]].vNormal + vNormal));
			XMStoreFloat3(&pVertices[iIndices[3]].vNormal,
				XMVector3Normalize(pVertices[iIndices[3]].vNormal + vNormal));
		}
	}
#pragma endregion

	/* 정점버퍼와 인덱스 버퍼를 만드낟. */
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
	m_BufferDesc.ByteWidth = m_iStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT; /* 정적버퍼로 할당한다. (Lock, unLock 호출 불가)*/
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iStride;

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pVertices;

	if (FAILED(__super::Create_Buffer(&m_pVB)))
		return E_FAIL;

	/* 정점버퍼와 인덱스 버퍼를 만드낟. */
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
	m_BufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	m_BufferDesc.Usage = D3D11_USAGE_DYNAMIC; /* 정적버퍼로 할당한다. (Lock, unLock 호출 불가)*/
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = 0;	

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pIndices;

	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);
	Safe_Delete_Array(pIndices);

	return S_OK;
}



HRESULT CVIBuffer_Terrain::Initialize(void * pArg)
{
	return S_OK;
}

Vec4 CVIBuffer_Terrain::SetUp_OnTerrain(CTransform * pTerrainTransform, Vec4 vWorldPos)
{
	/*vWorldPos *  지형 월드 행렬의 역 => vWorldPos의 위치를 지형의 로컬 스페이스 상의 정보로 변환한다. */
	Vec3		vLocalPos = XMVector3TransformCoord(vWorldPos, pTerrainTransform->Get_WorldMatrix_Inverse());
		
	/* 현재 플레이어가 존재하고 있는 네모 영역의 왼쪽 하단 정점의 인덱스를 구했다. */
	_uint		iIndex = _uint(XMVectorGetZ(vLocalPos)) * m_iNumVerticesX + _uint(XMVectorGetX(vLocalPos));

	_uint		iIndices[4] = {
		iIndex + m_iNumVerticesX, 
		iIndex + m_iNumVerticesX + 1,
		iIndex + 1, 
		iIndex
	};

	_float		fWidth = XMVectorGetX(vLocalPos) - m_pVerticesPos[iIndices[0]].x;
	_float		fDepth = m_pVerticesPos[iIndices[0]].z - XMVectorGetZ(vLocalPos);

	Vec4		vPlane;
	
	/* 오른쪽 위 삼각형 */
	if (fWidth >= fDepth)
	{
		vPlane = XMPlaneFromPoints(XMLoadFloat3(&m_pVerticesPos[iIndices[0]]),
			XMLoadFloat3(&m_pVerticesPos[iIndices[1]]),
			XMLoadFloat3(&m_pVerticesPos[iIndices[2]]));			
	}
	/* 왼쪽 하단 삼각형 */
	else
	{
		vPlane = XMPlaneFromPoints(XMLoadFloat3(&m_pVerticesPos[iIndices[0]]),
			XMLoadFloat3(&m_pVerticesPos[iIndices[2]]),
			XMLoadFloat3(&m_pVerticesPos[iIndices[3]]));
	}	

	/* 평면 방정식에 a, b, c, d 다 구했다. */
	/* 내 위치 x, y, z */
	// ax + by + cz + d = 0
	// y = (-ax - cz - d) / b

	/* 평면 상에 존재할 수 있도록하는 y를 구하자. */
	/* 평면상에 존재하면 되는 것이기 때문에 x, z는 변할 이유가 없다. */
	_float	fY = ((-XMVectorGetX(vPlane) * XMVectorGetX(vLocalPos) - (XMVectorGetZ(vPlane) * XMVectorGetZ(vLocalPos)) - XMVectorGetW(vPlane))) / XMVectorGetY(vPlane);

	vLocalPos = XMVectorSetY(vLocalPos, fY);

	return XMVector3TransformCoord(vLocalPos, pTerrainTransform->Get_WorldMatrix());	
}

void CVIBuffer_Terrain::Culling(CTransform * pTerrainTransform)
{
	m_pFrustum->Transform_ToLocalSpace(pTerrainTransform->Get_WorldMatrix_Inverse());
	_uint			iNumIndices = 0;


}

CVIBuffer_Terrain * CVIBuffer_Terrain::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const wstring& strHeightMapFilePath)
{
	CVIBuffer_Terrain*	pInstance = new CVIBuffer_Terrain(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(strHeightMapFilePath)))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Terrain");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CVIBuffer_Terrain::Clone(void * pArg)
{
	CVIBuffer_Terrain*	pInstance = new CVIBuffer_Terrain(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CVIBuffer_Terrain");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CVIBuffer_Terrain::Free()
{
	__super::Free();

	Safe_Release(m_pFrustum);

}
