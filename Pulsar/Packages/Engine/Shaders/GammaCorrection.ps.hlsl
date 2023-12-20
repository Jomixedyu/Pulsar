
[[vk::combinedImageSampler]]
Texture2D SceneColor : register(t0);
[[vk::combinedImageSampler]]
SamplerState state : register(s0);

struct VSOutput
{
	float4 Pos : SV_POSITION;
	float2 UV : TEXCOORD0;
};

float4 main(in VSOutput v2f) : SV_TARGET
{
    return SceneColor.Sample(state, v2f.UV) * 2;
}