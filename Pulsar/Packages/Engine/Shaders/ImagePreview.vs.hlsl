#include "PostProcessing.inc.hlsl"


PPVSOutput main(uint VertexIndex : SV_VertexID)
{
	PPVSOutput output = (PPVSOutput)0;
	output.TexCoord0 = float2(VertexIndex & 2, (VertexIndex << 1) & 2);
	output.Position = float4(output.TexCoord0.x * 2.0f - 1.0f, -(output.TexCoord0.y * 2.0f - 1.0f), 0.1f, 1.0f);
	return output;
}
