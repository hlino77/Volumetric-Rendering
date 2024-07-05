#pragma once

#include "Base.h"

/* 클라이엉ㄴ트에서 제작할 다양한 게임오브젝트들의 부모가된다. */

BEGIN(Engine)

class ENGINE_DLL CCloud : public CBase
{
private:
	/* 원형을 생성할 때 */
	CCloud(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	/* 사본을 생성할 때 */
	CCloud(const CCloud& rhs); /* 복사 생성자. */
	virtual ~CCloud() = default;


public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void PriorityTick(_float fTimeDelta);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_LightDepth() { return S_OK; }

protected:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	BoundingBox				m_tBoundingBox;

public:
	static CCloud* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END