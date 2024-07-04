#pragma once

#include "GameObject.h"

BEGIN(Engine)

class ENGINE_DLL CLandObject abstract : public CGameObject
{
public:
	typedef struct tagLandObjectDesc
	{
		class CTransform*			pTerrainTransform = { nullptr };
		class CVIBuffer_Terrain*	pVIBuffer_Terrain = { nullptr };
	}LANDOBJECT_DESC;

protected:
	CLandObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLandObject(const CLandObject& rhs);
	virtual ~CLandObject() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void PriorityTick(_float fTimeDelta);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();

protected:
	Vec4 SetUp_OnTerrain(Vec4 vWorldPos);

private:
	class CTransform*			m_pTerrainTransform = { nullptr };
	class CVIBuffer_Terrain*	m_pVIBuffer_Terrain = { nullptr };

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END