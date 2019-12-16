//--------------------------------------------------------------------------------------
// File: fractal.hlsl
//
// This is a compute shader that draws a Mandelbrot fractal.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

Texture2D ColorMapTexture : register(t0);
SamplerState ColorMapSampler : register(s0);
RWTexture2D<float4> OutputTexture : register(u0);


cbuffer cb0 : register(b6)
{
	float4 g_MaxThreadIter;
	float4 g_Window;
}

//SamplerState ColorMapSampler : register(s0)
//{
//	Filter = MIN_MAG_LINEAR_MIP_POINT;
//	//AddressU = WRAP;
//	//AddressV = WRAP;
//	AddressU = Clamp;
//	AddressV = Clamp;
//	AddressW = Clamp;
//	BorderColor = float4(1, 1, 1, 1);
//};


[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float4 MaxThreadIter = float4(1280, 720, 300, 0);
	float4 Window = float4(4.0f, 2.25f, -0.65f, 0.0f);

	float2 WindowLocal = ((float2)DTid.xy / MaxThreadIter.xy) * float2(1, -1) + float2(-0.5f, 0.5f);
	float2 coord = WindowLocal.xy * Window.xy + Window.zw;

	uint maxiter = (uint)MaxThreadIter.z * 4;
	uint iter = 0;
	float2 constant = coord;
	float2 sq;
	do
	{
		float2 newvalue;
		sq = coord * coord;
		newvalue.x = sq.x - sq.y;
		newvalue.y = 2 * coord.y * coord.x;
		coord = newvalue + constant;
		iter++;
	} while (iter < maxiter && (sq.x + sq.y) < 4.0);

	float colorIndex = frac((float)iter / MaxThreadIter.z);
	float4 SampledColor = ColorMapTexture.SampleLevel(ColorMapSampler, float2(colorIndex, 0), 0);

	OutputTexture[DTid.xy] = SampledColor;
}


technique11 Compute
{
	pass P0
	{
		SetVertexShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, main()));
		SetGeometryShader(NULL);
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetPixelShader(NULL);
	}
}
