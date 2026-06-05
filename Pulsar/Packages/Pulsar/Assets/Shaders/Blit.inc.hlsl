#ifndef _BLIT_INC_HLSL
#define _BLIT_INC_HLSL



void TriangleFullScreen(uint vertexID, out float2 uv, out float4 pos)
{
	uv = float2(vertexID & 2, (vertexID << 1) & 2);
	pos = float4(uv.x * 2.0f - 1.0f, -(uv.y * 2.0f - 1.0f), 0.1f, 1.0f);
}

void BlitVSMain(uint vertexID : SV_VertexID, out float4 position : SV_Position, out float2 texcoord : TEXCOORD0)
{
    TriangleFullScreen(vertexID, texcoord, position);
}

#endif