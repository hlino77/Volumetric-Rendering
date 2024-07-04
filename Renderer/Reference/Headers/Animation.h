#pragma once

#include "Base.h"

BEGIN(Engine)

class CAnimation final : public CBase
{
private:
	CAnimation();
	CAnimation(const CAnimation& rhs);
	virtual ~CAnimation() = default;

public:
	void Set_Loop(_bool isLoop) {
		m_isLoop = isLoop;
	}

public:
	HRESULT Initialize(const class CModel* pModel, const aiAnimation* pAIAnimation);
	void Update_TransformationMatrix(vector<class CBone*>& Bones,_float fTimeDelta);
	void Reset();

private:
	_float							m_fDuration = { 0.f };
	_float							m_fTickPerSecond = { 0.0f };
	_float							m_fTrackPosition = { 0.0f }; /* ���� �ִϸ��̼��� ��� ��ġ. */
	_bool							m_isFinished = { false };
	_bool							m_isLoop = { false };


	/* �� �ִϸ��̼��� �����ؾ��ϴ� ������ ������.  */
	_char							m_szName[MAX_PATH] = "";
	_uint							m_iNumChannels = { 0 };
	vector<class CChannel*>			m_Channels;
	vector<_uint>					m_iCurrentKeyFrames;

public:
	static CAnimation* Create(const class CModel* pModel, const aiAnimation* pAIAnimation);
	CAnimation* Clone();
	virtual void Free() override;
};

END