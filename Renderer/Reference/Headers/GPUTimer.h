#pragma once
#include "Engine_Defines.h"

BEGIN(Engine)

class CGPUTimer
{
public:
	CGPUTimer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	~CGPUTimer();

	void	Start();
	void	End();

	_float	Compute_Time();

	_bool	m_bStarted = false;
private:
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pContext = nullptr;

	ID3D11Query* m_pStartQuery = nullptr;
	ID3D11Query* m_pEndQuery = nullptr;
	ID3D11Query* m_pDisjointQuery = nullptr;
};

END