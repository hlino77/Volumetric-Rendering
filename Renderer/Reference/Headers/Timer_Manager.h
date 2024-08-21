#pragma once

#include "Base.h"

BEGIN(Engine)

class CTimer_Manager : public CBase
{
	DECLARE_SINGLETON(CTimer_Manager)

private:
	CTimer_Manager();
	virtual ~CTimer_Manager() = default;

public:
	void	Initialize_GPUTimer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	_float	Compute_TimeDelta(const wstring& strTimerTag);

public:
	HRESULT			Add_Timer(const wstring& strTimerTag);

	//GPU Timer
	HRESULT			Add_GPUTimer(const wstring& strTimerTag);
	void			Start_GPUTimer(const wstring& strTimerTag);
	void			End_GPUTimer(const wstring& strTimerTag);
	_float			Compute_GPUTimer(const wstring& strTimerTag);


private:
	map<const wstring, class CTimer*>		m_Timers;
	map<const wstring, class CGPUTimer*>	m_GPUTimers;
private:
	class CTimer* Find_Timer(const wstring& strTimerTag) const;

	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pContext = nullptr;
public:
	virtual void	Free(void);
};

END