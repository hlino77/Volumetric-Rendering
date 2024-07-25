
#define THREAD_GROUP_SIZE_X 16
#define THREAD_GROUP_SIZE_Y 16

#define STEP_COUNT 1000

RWTexture2D<float4> Transmittance;

cbuffer AtmosphereParams : register(b0)
{
	float3	fScatterRayleigh;
	float	fHDensityRayleigh;

	float	fScatterMie;
	float	fAsymmetryMie;
	float	fAbsorbMie;
	float	fHDensityMie;

	float3	fAbsorbOzone;
	float	fOzoneCenterHeight;

	float	fOzoneThickness;

	float	fEarthRadius;
	float	fAtmosphereRadius;
	float	fPadding;
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


float3 GetTransmittance(float fHeight)
{
	float3 fRayleigh = fScatterRayleigh * exp(-fHeight / fHDensityRayleigh);
	float fMie = (fScatterMie + fAbsorbMie) * exp(-fHeight / fHDensityMie);
	float3 fOzone = fAbsorbOzone * max(0.0f, 1 - 0.5 * abs(fHeight - fOzoneCenterHeight) / fOzoneThickness);
	return fRayleigh + fMie + fOzone;
}



[numthreads(THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1)]
void CSTransLUT(int3 iThreadIdx : SV_DispatchThreadID)
{
	int iWidth, iHeight;
	Transmittance.GetDimensions(iWidth, iHeight);
	if (iThreadIdx.x >= iWidth || iThreadIdx.y >= iHeight)
		return;

	float fTheta = asin(lerp(-1.0, 1.0, float(iThreadIdx.y) / iHeight));
	float fHeight = lerp(
		0.0, fAtmosphereRadius - fEarthRadius, (iThreadIdx.x + 0.5) / iWidth);

	float2 vOrigin = float2(0, fEarthRadius + fHeight);
	float2 vDir = float2(cos(fTheta), sin(fTheta));

	float fLength = 0;
	if (!findClosestIntersectionWithCircle(vOrigin, vDir, fEarthRadius, fLength))
		findClosestIntersectionWithCircle(vOrigin, vDir, fAtmosphereRadius, fLength);

	float2 vEnd = vOrigin + fLength * vDir;

	float3 vSum;
	for (int i = 0; i < STEP_COUNT; ++i)
	{
		float2 vCurr = lerp(vOrigin, vEnd, (i + 0.5) / STEP_COUNT);
		float fCurrHeight = length(vCurr) - fEarthRadius;
		float3 vResult = GetTransmittance(fCurrHeight);
		vSum += vResult;
	}

	float3 vResult = exp(-vSum * (fLength / STEP_COUNT));
	Transmittance[iThreadIdx.xy] = float4(vResult, 1);
}