#pragma once

#include "Component.h"

/* CVIBuffer : Vertices + Indices */
/* 정점과 인덱스를 할당하고 초기화한다. */
/* 렌더함수에서  이 두 버퍼를 이용하여 그린다.  */
BEGIN(Engine)

class ENGINE_DLL CVIBuffer abstract : public CComponent
{
protected:
	CVIBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer(const CVIBuffer& rhs);
	virtual ~CVIBuffer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Render();

protected:
	ID3D11Buffer*				m_pVB = { nullptr };
	ID3D11Buffer*				m_pIB = { nullptr };	

	Vec3*						m_pVerticesPos = { nullptr };

	D3D11_BUFFER_DESC			m_BufferDesc;
	D3D11_SUBRESOURCE_DATA		m_InitialData;
	_uint						m_iStride = { 0 }; /* 정점하나의 크기(Byte) */
	_uint						m_iNumVertices = { 0 };
	_uint						m_iIndexStride = { 0 };
	_uint						m_iNumIndices = { 0 };
	DXGI_FORMAT					m_eIndexFormat;
	D3D11_PRIMITIVE_TOPOLOGY	m_eTopology;
	_uint						m_iNumVBs = { 0 };

protected:
	HRESULT Create_Buffer(_Inout_ ID3D11Buffer** ppOut);

	

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END