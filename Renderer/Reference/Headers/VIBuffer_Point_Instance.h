#pragma once

#include "VIBuffer_Instancing.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Point_Instance final : public CVIBuffer_Instancing
{
private:
	CVIBuffer_Point_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Point_Instance(const CVIBuffer_Point_Instance& rhs);
	virtual ~CVIBuffer_Point_Instance() = default;

public:
	virtual HRESULT Initialize_Prototype(const INSTANCE_DESC& InstanceDesc);
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Update(_float fTimeDelta) override;

public:

public:
	static CVIBuffer_Point_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const INSTANCE_DESC& InstanceDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END