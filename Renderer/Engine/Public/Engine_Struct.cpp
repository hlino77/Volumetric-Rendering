#include "Engine_Defines.h"

namespace Engine
{
	const D3D11_INPUT_ELEMENT_DESC VTXPOS::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* ��ġ���Ϳ� ���� ����. */		
	};
	

	const D3D11_INPUT_ELEMENT_DESC VTXPOSTEX::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* ��ġ���Ϳ� ���� ����. */
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* �ؽ���� ���Ϳ� ���� ����. */
	};

	const D3D11_INPUT_ELEMENT_DESC VTXPOINT::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* ��ġ���Ϳ� ���� ����. */
		{ "PSIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* �ؽ���� ���Ϳ� ���� ����. */
	};

	const D3D11_INPUT_ELEMENT_DESC VTXCUBE::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* ��ġ���Ϳ� ���� ����. */
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* �ؽ���� ���Ϳ� ���� ����. */
	};	

	const D3D11_INPUT_ELEMENT_DESC VTXNORTEX::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* ��ġ���Ϳ� ���� ����. */
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* �븻���Ϳ� ���� ����. */
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* �ؽ���� ���Ϳ� ���� ����. */
	};

	const D3D11_INPUT_ELEMENT_DESC VTXMESH::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* ��ġ���Ϳ� ���� ����. */
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* �븻���Ϳ� ���� ����. */
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* �ؽ���� ���Ϳ� ���� ����. */
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* �ؽ���� ���Ϳ� ���� ����. */
	};

	const D3D11_INPUT_ELEMENT_DESC VTXANIMMESH::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* ��ġ���Ϳ� ���� ����. */
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* �븻���Ϳ� ���� ����. */
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* �ؽ���� ���Ϳ� ���� ����. */
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* �ؽ���� ���Ϳ� ���� ����. */
		{ "BLENDINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* ���� �ε����� */
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* ���� ����µ� */
	};

	const D3D11_INPUT_ELEMENT_DESC VTX_POINT_INSTANCE::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* ��ġ���Ϳ� ���� ����. */
		{ "PSIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* �ؽ���� ���Ϳ� ���� ����. */

		{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* �ν��Ͻ� ���� ����(���)�� Right. */
		{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* �ν��Ͻ� ���� ����(���)�� Up.*/
		{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* �ν��Ͻ� ���� ����(���)�� Look. */
		{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* �ν��Ͻ� ���� ����(���)�� Translation. */
	};

	

	const D3D11_INPUT_ELEMENT_DESC VTX_RECT_INSTANCE::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* ��ġ���Ϳ� ���� ����. */		
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* �ؽ���� ���Ϳ� ���� ����. */

		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* �ν��Ͻ� ���� ����(���)�� Right. */
		{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* �ν��Ͻ� ���� ����(���)�� Up.*/
		{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* �ν��Ͻ� ���� ����(���)�� Look. */
		{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* �ν��Ͻ� ���� ����(���)�� Translation. */
	};

	const D3D11_INPUT_ELEMENT_DESC VTX_MODEL_INSTANCE::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* ��ġ���Ϳ� ���� ����. */
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* �븻���Ϳ� ���� ����. */
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* �ؽ���� ���Ϳ� ���� ����. */
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* �ؽ���� ���Ϳ� ���� ����. */

		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* �ν��Ͻ� ���� ����(���)�� Right. */
		{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* �ν��Ͻ� ���� ����(���)�� Up.*/
		{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* �ν��Ͻ� ���� ����(���)�� Look. */
		{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* �ν��Ͻ� ���� ����(���)�� Translation. */
	};



	
	AtmosphereProperties AtmosphereProperties::ToStdUnit() const
	{
		AtmosphereProperties ret = *this;
		ret.fScatterRayleigh = 1e-6f * ret.fScatterRayleigh;
		ret.fHDensityRayleigh = 1e3f * ret.fHDensityRayleigh;

		ret.fScatterMie = 1e-6f * ret.fScatterMie;
		ret.fExtinctionMie = 1e-6f * ret.fExtinctionMie;
		ret.fHDensityMie = 1e3f * ret.fHDensityMie;

		ret.fAbsorbOzone = 1e-6f * ret.fAbsorbOzone;
		ret.vOzone = 1e3f * ret.vOzone;

		ret.fAtmosphereRadius = 1e3f * ret.fAtmosphereRadius;
		ret.fPlanetRadius = 1e3f * ret.fPlanetRadius;
		return ret;
	}
}

