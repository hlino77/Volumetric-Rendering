#pragma once

#include "Component_Manager.h"
#include "PipeLine.h"
#include "Input_Device.h"

/* 클라이언트개발자가 엔진의 기능을 이용하고자할 때 접촉하는 객체.  */
/* 클라이언트에 보여줘야할 함수들을 모두 정의하고 있는다. */

BEGIN(Engine)

class ENGINE_DLL CGameInstance final : public CBase
{
	DECLARE_SINGLETON(CGameInstance)
private:
	CGameInstance();
	virtual ~CGameInstance() = default;

public: /* For.GameInstance */
	HRESULT Initialize_Engine(_uint iNumLevels, HINSTANCE hInstance, const GRAPHIC_DESC& GraphicDesc, _Inout_ ID3D11Device** ppDevice, _Inout_ ID3D11DeviceContext** ppContext);
	void Tick(_float fTimeDelta);
	void Clear(_uint iLevelIndex);

public: /* For.Timer_Manager */
	_float Compute_TimeDelta(const wstring& strTimerTag);
	HRESULT	Add_Timer(const wstring& strTimerTag);

public: /* For.Graphic_Device */
	HRESULT Clear_BackBuffer_View(Vec4 vClearColor);	
	HRESULT Clear_DepthStencil_View();	
	HRESULT Present();

public: /* For.Input_Device */
	_byte	Get_DIKeyState(_ubyte byKeyID);
	_byte	Get_DIMouseState(CInput_Device::MOUSEKEYSTATE eMouse);
	_long	Get_DIMouseMove(CInput_Device::MOUSEMOVESTATE eMouseState);

public: /* For.Level_Manager */
	HRESULT Open_Level(_uint iLevelIndex, class CLevel* pNewLevel);

public: /* For.Object_Manager */
	HRESULT Add_Prototype(const wstring& strPrototypeTag, class CGameObject* pPrototype);
	HRESULT Add_GameObject(_uint iLevelIndex, const wstring& strLayerTag, const wstring& strPrototypeTag, void* pArg = nullptr);
	class CGameObject* Clone_GameObject(const wstring& strPrototypeTag, void* pArg = nullptr);

	class CGameObject* Get_GameObject(_uint iLevelIndex, const wstring& strLayerTag, _uint iIndex = 0);
	class CComponent* Get_Component(_uint iLevelIndex, const wstring& strLayerTag, const wstring& strComponentTag, _uint iIndex = 0);


public: /* For.Component_Manager */
	HRESULT Add_Prototype(_uint iLevelIndex, const wstring& strPrototypeTag, class CComponent* pPrototype);
	class CComponent* Clone_Component(_uint iLevelIndex, const wstring& strPrototypeTag, void* pArg = nullptr);

public: /* For.PipeLine */
	HRESULT Bind_TransformToShader(class CShader* pShader, const char* pConstantName, CPipeLine::TRANSFORMSTATE eState);
	Vec3 Get_CamPosition() const;

public: /* For.Light_Manager */
	const LIGHT_DESC* Get_LightDesc(_uint iLightIndex);
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);

public: /* For.Target_Manager */
	HRESULT Bind_SRV(class CShader* pShader, const wstring& strTargetTag, const _char* pConstantName);

public: /* For.Font_Manager */
	HRESULT Add_Font(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wstring& strFontTag, const wstring& strFontFilePath);
	HRESULT Render_Font(const wstring& strFontTag, const wstring & strText,
		const Vec2 & vPos,
		FXMVECTOR color = XMVectorZero(),
		float rotation = 0.f,
		XMFLOAT2 const & origin = Vec2(0.f, 0.f),
		float scale = 1.f);

public: /* For.Frustum */
	_bool isIn_Frustum_World(Vec3 vWorldPos, _float fRadius = 0.f);

private:
	class CTimer_Manager*			m_pTimer_Manager = { nullptr };
	class CGraphic_Device*			m_pGraphic_Device = { nullptr };
	class CInput_Device*			m_pInput_Device = { nullptr };
	class CLevel_Manager*			m_pLevel_Manager = { nullptr };
	class CObject_Manager*			m_pObject_Manager = { nullptr };
	class CComponent_Manager*		m_pComponent_Manager = { nullptr };
	class CPipeLine*				m_pPipeLine = { nullptr };
	class CLight_Manager*			m_pLight_Manager = { nullptr };
	class CTarget_Manager*			m_pTarget_Manager = { nullptr };
	class CFont_Manager*			m_pFont_Manager = { nullptr };
	class CFrustum*					m_pFrustum = { nullptr };

public:
	static void Release_Engine();
	virtual void Free() override;
};

END