#pragma once

/* CChannel: Ư�� �ִϸ��̼��� ��뤷�ϴ� ���� ����.  */
/* �ð��� ���� ���� ��ȯ ���°����� ������ �ִ´�. */
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
	/* ���� �̸�. */
	_char						m_szName[MAX_PATH] = "";
	/* �ð��� ���� ���� ������ �����Ѵ�. */
	_uint						m_iNumKeyFrames = { 0 };

	_uint						m_iBoneIndex = { 0 };

	/* KEYFRAME : Ư�� �ð��� ǥ���ؾ��� �� ���� ���¸� �ǹ��Ѥ�. */
	vector<KEYFRAME>			m_KeyFrames;

public:
	static CChannel* Create(const class CModel* pModel, const aiNodeAnim* pAIChannel);
	virtual void Free() override;
};

END