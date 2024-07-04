#pragma once

/* CChannel: 특정 애니메이션이 사용ㅇ하는 뼈의 정보.  */
/* 시간에 따른 뼈의 변환 상태값들을 가지고 있는다. */
#include "Base.h"

BEGIN(Engine)

class CChannel : public CBase
{
private:
	CChannel();
	virtual ~CChannel() = default;

public:
	HRESULT Initialize(const class CModel* pModel, const aiNodeAnim* pAIChannel);
	void Update_TransformationMatrix(_uint* pCurrentKeyFrame, vector<class CBone*>& Bones, _float fTrackPosition);
	

private:
	/* 뼈의 이름. */
	_char						m_szName[MAX_PATH] = "";
	/* 시간에 따른 상태 값들을 보관한다. */
	_uint						m_iNumKeyFrames = { 0 };

	_uint						m_iBoneIndex = { 0 };

	/* KEYFRAME : 특정 시간에 표현해야할 이 뼈의 상태를 의미한ㄷ. */
	vector<KEYFRAME>			m_KeyFrames;

public:
	static CChannel* Create(const class CModel* pModel, const aiNodeAnim* pAIChannel);
	virtual void Free() override;
};

END