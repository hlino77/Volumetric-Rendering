#pragma once

#include "Base.h"

/* 여러개의 폰트(글씨체마다) 객체를 가진다. */

BEGIN(Engine)

class CFont_Manager final : public CBase
{
	DECLARE_SINGLETON(CFont_Manager)
private:
	CFont_Manager();
	virtual ~CFont_Manager() = default;

public:
	HRESULT Add_Font(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wstring& strFontTag, const wstring& strFontFilePath);
	HRESULT Render(const wstring& strFontTag, const wstring & strText,
		const Vec2 & vPos,
		FXMVECTOR color,
		float rotation,
		XMFLOAT2 const & origin,
		float scale);

private:
	map<const wstring, class CCustomFont*>			m_Fonts;

private:
	class CCustomFont* Find_Font(const wstring& strFontTag);

public:
	virtual void Free() override;
};

END