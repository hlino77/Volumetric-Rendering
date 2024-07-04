#include "..\Public\PipeLine.h"
#include "Shader.h"

IMPLEMENT_SINGLETON(CPipeLine)

CPipeLine::CPipeLine()
{
}

void CPipeLine::Set_Transform(TRANSFORMSTATE eState, Matrix TransformMatrix)
{
	if (eState >= D3DTS_END)
		return;

	XMStoreFloat4x4(&m_TransformMatrices[eState], TransformMatrix);
}


Matrix CPipeLine::Get_Transform_Matrix(TRANSFORMSTATE eState) const
{
	if (eState >= D3DTS_END)
		return XMMatrixIdentity();

	return XMLoadFloat4x4(&m_TransformMatrices[eState]);
}


Matrix CPipeLine::Get_Transform_Matrix_Inverse(TRANSFORMSTATE eState) const
{
	if (eState >= D3DTS_END)
		return XMMatrixIdentity();

	return XMLoadFloat4x4(&m_TransformMatrices_Inverse[eState]);
}

Vec3 CPipeLine::Get_CamPosition() const
{
	return m_vCamPosition;
}


HRESULT CPipeLine::Bind_TransformToShader(CShader * pShader, const char * pConstantName, CPipeLine::TRANSFORMSTATE eState)
{	
	return pShader->Bind_Matrix(pConstantName, &m_TransformMatrices[eState]);	
}

HRESULT CPipeLine::Initialize()
{
	for (size_t i = 0; i < D3DTS_END; i++)
	{
		XMStoreFloat4x4(&m_TransformMatrices[i], XMMatrixIdentity());
		XMStoreFloat4x4(&m_TransformMatrices_Inverse[i], XMMatrixIdentity());
	}

	return S_OK;
}

void CPipeLine::Tick()
{
	for (size_t i = 0; i < D3DTS_END; i++)
	{
		m_TransformMatrices_Inverse[i] = m_TransformMatrices[i].Invert();
	}

	m_vCamPosition = m_TransformMatrices_Inverse->Translation();
}

void CPipeLine::Free()
{
	__super::Free();

}
