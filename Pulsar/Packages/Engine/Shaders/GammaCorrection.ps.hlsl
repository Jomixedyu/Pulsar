#include "PostProcessing.inc.hlsl"



float4 main(in PPVSOutput v2f) : SV_TARGET
{
	return PP_InColor.Sample(DefaultSampler, v2f.TexCoord0);
}

