#pragma once

#include "Component.h"

/* 객체의 월드 스페이스로의 변환을 위한 행렬을 보관한다. */
/* 객체의 월드 스페이스로의 변환을 위한 행렬 == 월드행렬. == 월드상태에서의 Right, Up, Look, Position의 정보를 각 행에 저장한다. */

BEGIN(Engine)

class ENGINE_DLL CTransform final : public CComponent
{
public:
	enum STATE { STATE_RIGHT, STATE_UP, STATE_LOOK, STATE_POSITION, STATE_END };

public:
	typedef struct tagTransformDesc
	{
		_float		fSpeedPerSec = { 0.f };
		_float		fRotationRadianPerSec = { 0.0f };
	}TRANSFORM_DESC;
private:
	CTransform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTransform(const CTransform& rhs);
	virtual ~CTransform() = default;

public:

	Vec3 Get_State(STATE eState) {
		return Vec3(m_WorldMatrix.m[eState]);
	}
	Vec3 Get_Scaled();

	Matrix Get_WorldMatrix() const {
		return m_WorldMatrix;
	}

	Matrix Get_WorldMatrix_Inverse() const {
		return m_WorldMatrix.Invert();
	}

	void Set_State(STATE eState, Vec3 vState);
	void Set_Scaling(const Vec3& vScale); 

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_ShaderResources(class CShader* pShader, const char* pConstantName);

public:
	void Go_Straight(_float fTimeDelta);
	void Go_Backward(_float fTimeDelta);
	void Go_Left(_float fTimeDelta);
	void Go_Right(_float fTimeDelta);
	void Fix_Rotation(Vec3 vAxis, _float fRadian);
	void Turn(Vec3 vAxis, _float fTimeDelta);
	void LookAt(Vec3 vPoint);
	void Chase(Vec3 vPoint, _float fTimeDelta, _float fMargin = 0.1f);

private:
	Matrix			m_WorldMatrix;

	
	_float				m_fSpeedPerSec = { 0.0f };
	_float				m_fRotationRadianPerSec = { 0.0f };
	


public:
	static CTransform* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END