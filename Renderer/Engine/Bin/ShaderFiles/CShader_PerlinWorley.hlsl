
#include "Engine_Shader_Defines.hpp"



RWTexture3D<float4> OutputTexture;


[numthreads(8, 8, 8)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    // 예제: 단순히 색을 채움
    OutputTexture[DTid] = float4(DTid.x / 256.0, DTid.y / 256.0, DTid.z / 256.0, 1.0);
}


technique11 DefaultTechnique
{
	pass Compute0
	{
		VertexShader = NULL;
		GeometryShader = NULL;
		ComputeShader = compile cs_5_0 CSMain();
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = NULL;
	}

	
}