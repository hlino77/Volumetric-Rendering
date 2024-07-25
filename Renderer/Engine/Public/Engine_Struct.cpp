#include "Engine_Defines.h"

namespace Engine
{
	const D3D11_INPUT_ELEMENT_DESC VTXPOS::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* 위치벡터에 대한 묘사. */		
	};
	

	const D3D11_INPUT_ELEMENT_DESC VTXPOSTEX::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* 위치벡터에 대한 묘사. */
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* 텍스쿠드 벡터에 대한 묘사. */
	};

	const D3D11_INPUT_ELEMENT_DESC VTXPOINT::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* 위치벡터에 대한 묘사. */
		{ "PSIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* 텍스쿠드 벡터에 대한 묘사. */
	};

	const D3D11_INPUT_ELEMENT_DESC VTXCUBE::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* 위치벡터에 대한 묘사. */
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* 텍스쿠드 벡터에 대한 묘사. */
	};	

	const D3D11_INPUT_ELEMENT_DESC VTXNORTEX::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* 위치벡터에 대한 묘사. */
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* 노말벡터에 대한 묘사. */
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* 텍스쿠드 벡터에 대한 묘사. */
	};

	const D3D11_INPUT_ELEMENT_DESC VTXMESH::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* 위치벡터에 대한 묘사. */
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* 노말벡터에 대한 묘사. */
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* 텍스쿠드 벡터에 대한 묘사. */
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* 텍스쿠드 벡터에 대한 묘사. */
	};

	const D3D11_INPUT_ELEMENT_DESC VTXANIMMESH::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* 위치벡터에 대한 묘사. */
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* 노말벡터에 대한 묘사. */
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* 텍스쿠드 벡터에 대한 묘사. */
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* 텍스쿠드 벡터에 대한 묘사. */
		{ "BLENDINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* 뼈의 인덱스들 */
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* 뼈의 영향력들 */
	};

	const D3D11_INPUT_ELEMENT_DESC VTX_POINT_INSTANCE::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* 위치벡터에 대한 묘사. */
		{ "PSIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* 텍스쿠드 벡터에 대한 묘사. */

		{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* 인스턴스 버퍼 정점(행렬)의 Right. */
		{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* 인스턴스 버퍼 정점(행렬)의 Up.*/
		{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* 인스턴스 버퍼 정점(행렬)의 Look. */
		{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* 인스턴스 버퍼 정점(행렬)의 Translation. */
	};

	

	const D3D11_INPUT_ELEMENT_DESC VTX_RECT_INSTANCE::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* 위치벡터에 대한 묘사. */		
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* 텍스쿠드 벡터에 대한 묘사. */

		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* 인스턴스 버퍼 정점(행렬)의 Right. */
		{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* 인스턴스 버퍼 정점(행렬)의 Up.*/
		{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* 인스턴스 버퍼 정점(행렬)의 Look. */
		{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* 인스턴스 버퍼 정점(행렬)의 Translation. */
	};

	const D3D11_INPUT_ELEMENT_DESC VTX_MODEL_INSTANCE::Elements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* 위치벡터에 대한 묘사. */
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }, /* 노말벡터에 대한 묘사. */
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* 텍스쿠드 벡터에 대한 묘사. */
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },	/* 텍스쿠드 벡터에 대한 묘사. */

		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* 인스턴스 버퍼 정점(행렬)의 Right. */
		{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* 인스턴스 버퍼 정점(행렬)의 Up.*/
		{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* 인스턴스 버퍼 정점(행렬)의 Look. */
		{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	/* 인스턴스 버퍼 정점(행렬)의 Translation. */
	};



	
	AtmosphereProperties AtmosphereProperties::toStdUnit() const
	{
		AtmosphereProperties ret = *this;
		ret.scatterRayleigh = 1e-6f * ret.scatterRayleigh;
		ret.hDensityRayleigh = 1e3f * ret.hDensityRayleigh;
		ret.scatterMie = 1e-6f * ret.scatterMie;
		ret.absorbMie = 1e-6f * ret.absorbMie;
		ret.hDensityMie = 1e3f * ret.hDensityMie;
		ret.absorbOzone = 1e-6f * ret.absorbOzone;
		ret.ozoneCenterHeight = 1e3f * ret.ozoneCenterHeight;
		ret.ozoneThickness = 1e3f * ret.ozoneThickness;
		ret.fAtmosphereRadius = 1e3f * ret.fAtmosphereRadius;
		ret.fPlanetRadius = 1e3f * ret.fPlanetRadius;
		return ret;
	}
}

