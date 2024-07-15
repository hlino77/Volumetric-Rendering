
#include "Engine_Shader_Defines.hpp"
#include "Noise.hlsl"


RWTexture3D<float4> OutputTexture;



[numthreads(8, 8, 8)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    float3 vUV = float3(DTid.x / 128.f, DTid.y / 128.f, DTid.z / 128.f);
    vUV += float3(1.0f, 1.0f, 1.0f);
    
    float fFreq = 4.0f; 
    
    float fPfbm = lerp(1.0f, perlinfbm(vUV, 4.0f, 7), 0.5f);
    fPfbm = abs(fPfbm * 2.0f - 1.0f); // billowy perlin noise

    float fWorley1 = worleyFbm(vUV, fFreq);
    float fWorley2 = worleyFbm(vUV, fFreq * 2.0f);
    float fWorley3 = worleyFbm(vUV, fFreq * 4.0f);
    float fPerlinWorley = remap(fPfbm, 0.0f, 1.0f, fWorley1, 1.0f); // perlin-worley
   
    OutputTexture[DTid] = float4(fPerlinWorley, fWorley1, fWorley2, fWorley3);

    //if (vUV.x == 0.0f)
    //{
        //OutputTexture[DTid] = float4(0.0f, 0.0f, 0.0f, 0.0f);
    //}
}


