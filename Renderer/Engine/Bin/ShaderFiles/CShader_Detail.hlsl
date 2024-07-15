
#include "Engine_Shader_Defines.hpp"
#include "Noise.hlsl"


RWTexture3D<float4> OutputTexture;



[numthreads(8, 8, 8)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    
    float3 vUV = float3(DTid.x / 32.f, DTid.y / 32.f, DTid.z / 32.f);
    vUV += float3(1.0f, 1.0f, 1.0f);

    float fFreq = 8.0f;
    
    float fWorley1 = worleyFbm(vUV, fFreq);
    float fWorley2 = worleyFbm(vUV, fFreq * 2.0f);
    float fWorley3 = worleyFbm(vUV, fFreq * 4.0f);

    OutputTexture[DTid] = float4(fWorley1, fWorley2, fWorley3, 0.0f);
}


