
#define THREAD_GROUP_SIZE_X 16
#define THREAD_GROUP_SIZE_Y 16

#define STEP_COUNT 1000



float			g_fATest = 5.0f;

RWTexture2D<float4> Transmittance;

cbuffer AtmosphereParams : register(b0)
{
	float3	ScatterRayleigh;
	float	HDensityRayleigh;

	float	ScatterMie;
	float	AsymmetryMie;
	float	AbsorbMie;
	float	HDensityMie;

	float3	AbsorbOzone;
	float	OzoneCenterHeight;

	float	OzoneThickness;

	float	fEarthRadius;
	float	fAtmosphereRadius;
	float	fPadding;
}




float3 getSigmaT(float h)
{
	float3 rayleigh = ScatterRayleigh * exp(-h / HDensityRayleigh);
	float mie = (ScatterMie + AbsorbMie) * exp(-h / HDensityMie);
	float3 ozone = AbsorbOzone * max(
		0.0f, 1 - 0.5 * abs(h - OzoneCenterHeight) / OzoneThickness);
	return rayleigh + mie + ozone;
}

bool findClosestIntersectionWithCircle(
	float2 o, float2 d, float R, out float t)
{
	float A = dot(d, d);
	float B = 2 * dot(o, d);
	float C = dot(o, o) - R * R;
	float delta = B * B - 4 * A * C;
	if (delta < 0)
		return false;
	t = (-B + (C <= 0 ? sqrt(delta) : -sqrt(delta))) / (2 * A);
	return (C <= 0) | (B <= 0);
}


[numthreads(THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1)]
void CSTransLUT(int3 threadIdx : SV_DispatchThreadID)
{
	int width, height;
	Transmittance.GetDimensions(width, height);
	if (threadIdx.x >= width || threadIdx.y >= height)
		return;

	float theta = asin(lerp(-1.0, 1.0, (threadIdx.y + 0.5) / height));
	float h = lerp(
		0.0, fAtmosphereRadius - fEarthRadius, (threadIdx.x + 0.5) / width);

	float2 o = float2(0, fEarthRadius + h);
	float2 d = float2(cos(theta), sin(theta));

	float t = 0;
	if (!findClosestIntersectionWithCircle(o, d, fEarthRadius, t))
		findClosestIntersectionWithCircle(o, d, fAtmosphereRadius, t);

	float2 end = o + t * d;

	float3 sum;
	for (int i = 0; i < STEP_COUNT; ++i)
	{
		float2 pi = lerp(o, end, (i + 0.5) / STEP_COUNT);
		float hi = length(pi) - fEarthRadius;
		float3 sigma = getSigmaT(hi);
		sum += sigma;
	}

	float3 result = exp(-sum * (t / STEP_COUNT));
	Transmittance[threadIdx.xy] = float4(result, 1);
}