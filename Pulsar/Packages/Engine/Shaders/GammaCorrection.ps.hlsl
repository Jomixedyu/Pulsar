#include "PostProcessing.inc.hlsl"


float4 main(in PPVSOutput v2f) : SV_TARGET
{
	float4 color = PP_InColor.Sample(ColorSampler, v2f.TexCoord0);
	return float4(pow(color.rgb, 1/2.2), color.a);
}

