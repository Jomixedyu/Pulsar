#include "PostProcessing.inc.hlsl"


PPVSOutput main(uint VertexIndex : SV_VertexID)
{
	PPVSOutput output = (PPVSOutput)0;
	output.TexCoord0 = float2((VertexIndex << 1) & 2, VertexIndex & 2);
	output.Position = float4(output.TexCoord0 * 2.0f - 1.0f, 0.0f, 1.0f);
	return output;
}
