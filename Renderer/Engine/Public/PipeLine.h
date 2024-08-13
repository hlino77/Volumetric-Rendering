#pragma once

#include "Base.h"

/* 뷰, 투영행렬을 보관한다. */
/* Tick함수안에서 뷰, 투영의 역행렬을 구해놓느다 .*/

BEGIN(Engine)

class CPipeLine final : public CBase
{
	DECLARE_SINGLETON(CPipeLine)
public:
	enum TRANSFORMSTATE { D3DTS_VIEW, D3DTS_PROJ, D3DTS_LIGHTVIEW, D3DTS_LIGHTPROJ, D3DTS_END };
private:
	CPipeLine();
	virtual ~CPipeLine() = default;

public:
	void Set_Transform(TRANSFORMSTATE eState, Matrix TransformMatrix);
	Matrix Get_Transform_Matrix(TRANSFORMSTATE eState) const;
	Matrix Get_Transform_Matrix_Inverse(TRANSFORMSTATE eState) const;
	Vec3 Get_CamPosition() const;

public:
	HRESULT Bind_TransformToShader(class CShader* pShader, const char* pConstantName, CPipeLine::TRANSFORMSTATE eState);	

public:
	HRESULT Initialize();
	void Tick();

private:
	Matrix			m_TransformMatrices[D3DTS_END];
	Matrix			m_TransformMatrices_Inverse[D3DTS_END];
	Vec3			m_vCamPosition = { 0.0f, 0.f, 0.f };

public:
	virtual void Free() override;
};

END