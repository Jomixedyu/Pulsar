#include "Common.inc.hlsl"
#include "MeshRenderer.inc.hlsl"

float4 _TintColor;

Texture2D _BaseColorMap;
SamplerState Sampler__BaseColorMap;

struct BillboardVaryings
{
    float4 Position : SV_POSITION;
    float2 TexCoord0 : TEXCOORD0;
};

BillboardVaryings VSMain(StandardAttributes a)
{
    BillboardVaryings v = (BillboardVaryings)0;

    // 世界中心
    float3 centerWS = mul(RendererBuffer.LocalToWorldMatrix, float4(0, 0, 0, 1)).xyz;

    // 兼容 XY 平面和 XZ 平面的 mesh（默认 Plane 是 XZ 平面，y=0）
    float2 localOffset = float2(a.Position.x,
                                abs(a.Position.y) > 0.0001 ? a.Position.y : a.Position.z);

    // 把 model matrix 的 scale 当作 clip-space 大小的缩放因子
    float scale = length(float3(RendererBuffer.LocalToWorldMatrix[0].x,
                                RendererBuffer.LocalToWorldMatrix[1].x,
                                RendererBuffer.LocalToWorldMatrix[2].x));
    float clipScale = scale * 0.06;

    // 先投影中心点到 clip-space
    float4 centerCS = mul(CameraBuffer.MatrixVP, float4(centerWS, 1.0));

    // 在 clip-space 中加固定偏移（乘以 w 以抵消透视除法）
    float2 offsetCS = localOffset * clipScale;
    float4 posCS = centerCS;
    posCS.xy += offsetCS * centerCS.w;

    v.Position = posCS;
    v.TexCoord0 = a.TexCoord0;
    return v;
}

float4 PSMain(BillboardVaryings v) : SV_Target
{
    float4 tex = _BaseColorMap.Sample(Sampler__BaseColorMap, v.TexCoord0);
    float3 final = tex.rgb * _TintColor.rgb;
    return float4(final, tex.a * _TintColor.a);
}
