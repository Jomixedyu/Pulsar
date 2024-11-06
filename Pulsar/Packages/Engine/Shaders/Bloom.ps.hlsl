#include "PostProcessing.inc.hlsl"

cbuffer Properties : register(b0, space3)
{
	float _Luminance;
}

float Luminace(float4 color)
{
	return 0.2125 * color.r + 0.7154 * color.g + 0.0721 * color.b;
}

float4 main(in PPVSOutput v2f) : SV_TARGET
{
	float4 color = PP_InColor.Sample(ColorSampler, v2f.TexCoord0);
	float luminace = Luminace(color);
	float val = saturate(luminace - _Luminance);
	return color * val;
}

