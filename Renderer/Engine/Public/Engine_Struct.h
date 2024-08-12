#ifndef Engine_Struct_h__
#define Engine_Struct_h__

namespace Engine
{
	typedef struct tagGraphicDesc
	{
		enum WINMODE { WINMODE_FULL, WINMODE_WIN, WINMODE_END };
		HWND		hWnd;
		WINMODE		eWinMode;
		unsigned int	iWinSizeX, iWinSizeY;
	}GRAPHIC_DESC;

	typedef struct tagLightDesc
	{
		enum TYPE { LIGHT_DIRECTIONAL, LIGHT_POINT, LIGHT_SUN, LIGHT_END };

		TYPE		eLightType = { LIGHT_END };

		Vec4	vLightPos;

		Vec4	vLightDir;

		Vec4	vDiffuse;
		Vec4	vAmbient;
		Vec4	vSpecular;

		ID3D11ShaderResourceView* pTransLUT = nullptr;
		ID3D11ShaderResourceView* pMultiScatLUT = nullptr;

		ID3D11Buffer* pAtmosphereBuffer = nullptr;
	}LIGHT_DESC;

	typedef struct tagMeshMaterial
	{
		class CTexture*		pTextures[AI_TEXTURE_TYPE_MAX];
	}MESH_MATERIAL;

	typedef struct tagKeyFrame
	{
		float			fTime;
		Vec3		vScale;
		Vec4		vRotation;
		Vec4		vTranslation;
	}KEYFRAME;


	typedef struct ENGINE_DLL tagVertex_Volume
	{
		_uint		iVertexID;
		_uint		iInstanceID;

		static const unsigned int				iNumElements = 2;
		static const D3D11_INPUT_ELEMENT_DESC	Elements[iNumElements];
	}VTXVOLUME;

	typedef struct ENGINE_DLL tagVertex_Position
	{
		Vec3		vPosition;		

		static const unsigned int				iNumElements = 1;
		static const D3D11_INPUT_ELEMENT_DESC	Elements[iNumElements];
	}VTXPOS;

	typedef struct ENGINE_DLL tagVertex_Position_Texcoord
	{
		Vec3		vPosition;
		Vec2		vTexcoord;

		static const unsigned int				iNumElements = 2;
		static const D3D11_INPUT_ELEMENT_DESC	Elements[iNumElements];

	}VTXPOSTEX;

	typedef struct ENGINE_DLL tagVertex_Position_Size
	{
		Vec3		vPosition;
		Vec2		vPSize;		

		static const unsigned int				iNumElements = 2;
		static const D3D11_INPUT_ELEMENT_DESC	Elements[iNumElements];

	}VTXPOINT;



	typedef struct ENGINE_DLL tagVertex_Position_Cube_Texcoord
	{
		Vec3		vPosition;
		Vec3		vTexcoord;

		static const unsigned int				iNumElements = 2;
		static const D3D11_INPUT_ELEMENT_DESC	Elements[iNumElements];

	}VTXCUBE;

	typedef struct ENGINE_DLL tagVertex_Position_Normal_Texcoord
	{
		Vec3		vPosition;
		Vec3		vNormal;
		Vec2		vTexcoord;

		static const unsigned int				iNumElements = 3;
		static const D3D11_INPUT_ELEMENT_DESC	Elements[iNumElements];

	}VTXNORTEX;

	typedef struct ENGINE_DLL tagVertex_Mesh
	{
		Vec3		vPosition;
		Vec3		vNormal;
		Vec2		vTexcoord;
		Vec3		vTangent;

		static const unsigned int				iNumElements = 4;
		static const D3D11_INPUT_ELEMENT_DESC	Elements[iNumElements];

	}VTXMESH;

	typedef struct ENGINE_DLL tagVertex_Animation_Mesh
	{
		Vec3		vPosition;
		Vec3		vNormal;
		Vec2		vTexcoord;
		Vec3		vTangent;

		/* 이 정점이 상태를 받아와야할 뼈들의 인덱스 */
		XMUINT4		vBlendIndices;
		Vec4		vBlendWeights;

		static const unsigned int				iNumElements = 6;
		static const D3D11_INPUT_ELEMENT_DESC	Elements[iNumElements];

	}VTXANIMMESH;

	/* 인스턴스 버퍼를 만들기 위해. */
	typedef struct ENGINE_DLL tagVertex_Instance
	{
		Vec4		vRight;
		Vec4		vUp;
		Vec4		vLook;
		Vec4		vTranslation;
	
		/* 셰이더에 이러한 정점이 잘 들어가냐? 라는 검증을 위해 */
		/* 이 정점 뿐만아니라 그리기용 정점도 같이 받아야되지. */
		/*static const unsigned int				iNumElements = 4;
		static const D3D11_INPUT_ELEMENT_DESC	Elements[iNumElements];*/
	}VTXINSTANCE;

	/* 그리기용 정점 + 인스턴스용 정점에 대한 정보가 합쳐져서 셰이더에 검증처리를 하기위해. */

	typedef struct ENGINE_DLL tagVertex_Point_Instance
	{
		static const unsigned int				iNumElements = 6;
		static const D3D11_INPUT_ELEMENT_DESC	Elements[iNumElements];
	}VTX_POINT_INSTANCE;

	typedef struct ENGINE_DLL tagVertex_Rect_Instance
	{		
		static const unsigned int				iNumElements = 6;
		static const D3D11_INPUT_ELEMENT_DESC	Elements[iNumElements];
	}VTX_RECT_INSTANCE;

	typedef struct ENGINE_DLL tagVertex_Model_Instance
	{
		static const unsigned int				iNumElements = 8;
		static const D3D11_INPUT_ELEMENT_DESC	Elements[iNumElements];
	}VTX_MODEL_INSTANCE;





	struct alignas(16) AtmosphereProperties
	{
		Vec4	fScatterRayleigh = { 5.802f, 13.558f, 33.1f, 0.0f };
		float	fHDensityRayleigh = 8;

		float	fScatterMie = 3.996f;
		float	fPhaseMieG = 0.8f;
		float	fExtinctionMie = 4.4f;
		float	fHDensityMie = 1.2f;

		float	fPlanetRadius = 6360;
		float	fAtmosphereRadius = 6460;

		float	fSunIlluminance = 100000.0f;

 		Vec4	fAbsorbOzone = { 0.65f, 1.881f, 0.085f, 0.0f };
		Vec4	vOzone = { 25.0f, 8.0f, 0.0f, 0.0f };

		float	fMultiScatFactor = 1.0f;

		AtmosphereProperties ToStdUnit() const;
	};

	
}


#endif // Engine_Struct_h__
