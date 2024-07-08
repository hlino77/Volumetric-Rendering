
#include "Engine_Shader_Defines.hpp"



RWTexture3D<float4> OutputTexture;

#define UI0 1597334673U
#define UI1 3812015801U
#define UI2 uint2(UI0, UI1)
#define UI3 uint3(UI0, UI1, 2798796415U)
#define UIF (1.0f / float(0xffffffffU))


float3 hash33(float3 p)
{
	uint3 q = uint3(int3(p)) * UI3;
	q = (q.x ^ q.y ^ q.z) * UI3;
	return -1.0f + 2.0f * float3(q) * UIF;
}

float remap(float x, float a, float b, float c, float d)
{
    return (((x - a) / (b - a)) * (d - c)) + c;
}

float gradientNoise(float3 x, float freq)
{
     // grid
    float3 p = floor(x);
    float3 w = frac(x);
    
    // quintic interpolant
    float3 u = w * w * w * (w * (w * 6.0f - 15.0f) + 10.0f);

    // gradients
    float3 ga = hash33(fmod(p + float3(0.0f, 0.0f, 0.0f), freq));
    float3 gb = hash33(fmod(p + float3(1.0f, 0.0f, 0.0f), freq));
    float3 gc = hash33(fmod(p + float3(0.0f, 1.0f, 0.0f), freq));
    float3 gd = hash33(fmod(p + float3(1.0f, 1.0f, 0.0f), freq));
    float3 ge = hash33(fmod(p + float3(0.0f, 0.0f, 1.0f), freq));
    float3 gf = hash33(fmod(p + float3(1.0f, 0.0f, 1.0f), freq));
    float3 gg = hash33(fmod(p + float3(0.0f, 1.0f, 1.0f), freq));
    float3 gh = hash33(fmod(p + float3(1.0f, 1.0f, 1.0f), freq));
    
    // projections
    float va = dot(ga, w - float3(0.0f, 0.0f, 0.0f));
    float vb = dot(gb, w - float3(1.0f, 0.0f, 0.0f));
    float vc = dot(gc, w - float3(0.0f, 1.0f, 0.0f));
    float vd = dot(gd, w - float3(1.0f, 1.0f, 0.0f));
    float ve = dot(ge, w - float3(0.0f, 0.0f, 1.0f));
    float vf = dot(gf, w - float3(1.0f, 0.0f, 1.0f));
    float vg = dot(gg, w - float3(0.0f, 1.0f, 1.0f));
    float vh = dot(gh, w - float3(1.0f, 1.0f, 1.0f));
    
    // interpolation
    return va + 
           u.x * (vb - va) + 
           u.y * (vc - va) + 
           u.z * (ve - va) + 
           u.x * u.y * (va - vb - vc + vd) + 
           u.y * u.z * (va - vc - ve + vg) + 
           u.z * u.x * (va - vb - ve + vf) + 
           u.x * u.y * u.z * (-va + vb + vc - vd + ve - vf - vg + vh);
}

float worleyNoise(float3 uv, float freq)
{  
    float3 id = floor(uv);
    float3 p = frac(uv);
    
    float minDist = 10000.0;
    for (int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            for(int z = -1; z <= 1; ++z)
            {
                float3 offset = float3(x, y, z);
                float3 Input = id + offset;
                
                float3 h = hash33(float3(fmod(Input.x, freq), fmod(Input.y, freq), fmod(Input.z, freq))) * 0.5f + 0.5f;
                h += offset;
                float3 d = p - h;
                minDist = min(minDist, dot(d, d));
            }
        }
    }
    
    // inverted worley noise
    return 1.0f - minDist;
}


float perlinfbm(float3 p, float freq, int octaves)
{
    float G = exp2(-0.85f);
    float amp = 1.0f;
    float noise = 0.0f;
    for (int i = 0; i < octaves; ++i)
    {
        noise += amp * gradientNoise(p * freq, freq);
        freq *= 2.0f;
        amp *= G;
    }
    
    return noise;
}

float worleyFbm(float3 p, float freq)
{
    return worleyNoise(p * freq, freq) * 0.625f +
        	 worleyNoise(p * freq * 2.0f, freq * 2.0f) * 0.25f +
        	 worleyNoise(p * freq * 4.0f, freq * 4.0f) * 0.125f;
}


[numthreads(8, 8, 8)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    
    float3 uv = float3(DTid.x / 128.f, DTid.y / 128.f, DTid.z / 128.f);

    float4 col = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float freq = 4.0f;
    
    float pfbm = lerp(1.0f, perlinfbm(uv, 4.0f, 7), 0.5f);
    pfbm = abs(pfbm * 2.0f - 1.0f); // billowy perlin noise
    
    col.g += worleyFbm(uv, freq);
    col.x += remap(pfbm, 0.0, 1.0, col.g, 1.0); // perlin-worley
    
    OutputTexture[DTid] = col;
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