#include "GPUTimer.h"

CGPUTimer::CGPUTimer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
{
	D3D11_QUERY_DESC queryDesc = {};
	queryDesc.Query = D3D11_QUERY_TIMESTAMP;
	m_pDevice->CreateQuery(&queryDesc, &m_pStartQuery);
	m_pDevice->CreateQuery(&queryDesc, &m_pEndQuery);

	D3D11_QUERY_DESC disjointDesc = {};
	disjointDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	m_pDevice->CreateQuery(&disjointDesc, &m_pDisjointQuery);
}

CGPUTimer::~CGPUTimer()
{
	Safe_Release(m_pDisjointQuery);
	Safe_Release(m_pEndQuery);
	Safe_Release(m_pStartQuery);
}

void CGPUTimer::Start()
{
	m_bStarted = true;
	m_pContext->Begin(m_pDisjointQuery);
	m_pContext->End(m_pStartQuery);
}

void CGPUTimer::End()
{
	m_pContext->End(m_pEndQuery);
	m_pContext->End(m_pDisjointQuery);
}

_float CGPUTimer::Compute_Time()
{
	if (m_bStarted == false)
	{
		return 0.0f;
	}

	m_bStarted = false;

	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
	while (m_pContext->GetData(m_pDisjointQuery, &disjointData, sizeof(disjointData), 0) != S_OK) {}

	UINT64 startTime = 0;
	while (m_pContext->GetData(m_pStartQuery, &startTime, sizeof(startTime), 0) != S_OK) {}

	UINT64 endTime = 0;
	while (m_pContext->GetData(m_pEndQuery, &endTime, sizeof(endTime), 0) != S_OK) {}

	if (!disjointData.Disjoint) {
		UINT64 delta = endTime - startTime;
		return (double(delta) / double(disjointData.Frequency));
	}

	return 0.0f;
}

