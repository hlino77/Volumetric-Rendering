#pragma once

#include "Component.h"

/* 1. ȭ�鿡 �׷������ϴ� ��ü���� �׸��� ������� ��Ƽ� �����Ѵ�. */
/* 2. �����ϰ� �ִ� ��ü���� ������(��ο���)�� �����Ѵ�. */

BEGIN(Engine)

class ENGINE_DLL CRenderer final : public CComponent
{
public:
	/* RG_NONBLEND : ���� �׷����� Blend������Ʈ���� ���� ������ ���� �ݵ�� �������� �ֵ��� ���� �׷����Ѵ�. */
	/* RG_BLEND : �������ϰ� �׷����� ��ü�鵵 �ݵ�� �ָ��ִ� ����� �׸���. */
	enum RENDERGROUP { RG_PRIORITY, RG_SHADOW, RG_NONLIGHT, RG_NONBLEND, RG_BLEND, RG_UI, RG_END };
private:
	CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);	
	CRenderer(const CRenderer& rhs) = delete;
	virtual ~CRenderer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Add_RenderGroup(RENDERGROUP eRenderGroup, class CGameObject* pGameObject);
	HRESULT Draw_RenderObjects();

#ifdef _DEBUG
public:
	HRESULT Add_Debug(class CComponent* pDebug) {
		m_RenderDebug.push_back(pDebug);
		Safe_AddRef(pDebug);
		return S_OK;
	}

#endif


private:
	list<class CGameObject*>			m_RenderObjects[RG_END];
	
	class CTarget_Manager*				m_pTarget_Manager = { nullptr };
	class CLight_Manager*				m_pLight_Manager = { nullptr };

private:
	class CVIBuffer_Rect*		m_pVIBuffer = { nullptr };
	class CShader*				m_pShader = { nullptr };
	class CShader*				m_pVolumeRenderShader = { nullptr };

	class CCloud*				m_pCloud = { nullptr };

	Matrix					m_WorldMatrix, m_ViewMatrix, m_ProjMatrix;

	float m_fTest = 0.0f;

#ifdef _DEBUG
private:
	list<class CComponent*>				m_RenderDebug;
#endif

private:
	HRESULT Render_Priority();
	HRESULT Render_NonLight();
	HRESULT Render_LightDepth(); 
	HRESULT Render_NonBlend();
	HRESULT Render_LightAcc();
	HRESULT Render_Volume();
	HRESULT Render_Deferred();
	HRESULT Render_Blend();
	HRESULT Render_UI();

#ifdef _DEBUG
private:
	HRESULT Render_Debug();

#endif

public:
	static CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};

END