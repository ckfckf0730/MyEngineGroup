#include"PeraHeader.hlsli"

float4 ps(Output input) : SV_TARGET
{
	return tex.Sample(smp, input.uv) + float4(0.2,0.2,0.2,0.2);
}