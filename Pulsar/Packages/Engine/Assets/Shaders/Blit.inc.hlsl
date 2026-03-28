


struct Varyings
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

void TriangleFullScreen(uint vertexID, inout float2 uv, inout float4 pos)
{
	uv = float2(vertexID & 2, (vertexID << 1) & 2);
	pos = float4(uv.x * 2.0f - 1.0f, -(uv.y * 2.0f - 1.0f), 0.1f, 1.0f);
}

Varyings VSMain(uint vertexID : SV_VertexID)
{
    Varyings output;
    TriangleFullScreen(vertexID, output.TexCoord, output.Position);

    return output;
}