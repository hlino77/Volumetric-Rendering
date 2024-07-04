#pragma once

#include "VIBuffer_Instancing.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Rect_Instance final : public CVIBuffer_Instancing
{
private:
	CVIBuffer_Rect_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Rect_Instance(const CVIBuffer_Rect_Instance& rhs);
	virtual ~CVIBuffer_Rect_Instance() = default;

public:
	virtual HRESULT Initialize_Prototype(const INSTANCE_DESC& InstanceDesc);
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Update(_float fTimeDelta) override;

public:

public:
	static CVIBuffer_Rect_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const INSTANCE_DESC& InstanceDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END