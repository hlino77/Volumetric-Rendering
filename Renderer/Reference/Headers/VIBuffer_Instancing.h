#pragma once

/* �ν��Ͻ��� ���� �ټ� ���� �׷����� Ŭ�������� �θ�. */
/* Instance : �� �ϳ�. */
#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Instancing abstract : public CVIBuffer
{
public:
	typedef struct tagInstanceDesc
	{
		Vec3			vCenter;
		Vec3			vRange;		
		_float			fSpeedMin = 0.f, fSpeedMax = 0.f;		
		_float			fScaleMin, fScaleMax;
		_float			fLifeTimeMin, fLifeTimeMax;
		_uint			iNumInstance = 0;
	}INSTANCE_DESC;
protected:
	CVIBuffer_Instancing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Instancing(const CVIBuffer_Instancing& rhs);
	virtual ~CVIBuffer_Instancing() = default;

public:
	virtual HRESULT Initialize_Prototype(const INSTANCE_DESC& InstanceDesc);
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Update(_float fTimeDelta) { return S_OK; }
	virtual HRESULT Render() override;

protected:
	_uint					m_iStrideInstance = { 0 };
	_uint					m_iNumInstance = { 0 };
	_uint					m_iNumIndicesPerInstance = { 0 };
	ID3D11Buffer*			m_pVBInstance = { nullptr };
	VTXINSTANCE*			m_pVertices = { nullptr };

	_float*					m_pSpeeds = { nullptr };
	_float*					m_pLifeTimes = { nullptr };

	_float*					m_pTimeAccs = { nullptr };

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END