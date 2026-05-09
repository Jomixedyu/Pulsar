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

    // 从 LocalToWorldMatrix 提取世界位置和局部缩放
    float3 centerWS = RendererBuffer.LocalToWorldMatrix[3].xyz;
    float scaleX = length(RendererBuffer.LocalToWorldMatrix[0].xyz);
    float scaleY = length(RendererBuffer.LocalToWorldMatrix[1].xyz);

    // 从 InvMatrixV 提取相机在世界空间的 Right / Up 方向（单位化）
    float3 rightWS = normalize(CameraBuffer.InvMatrixV[0].xyz);
    float3 upWS    = normalize(CameraBuffer.InvMatrixV[1].xyz);

    // Billboard：用相机的 Right/Up 向量展开本地 quad，使平面始终平行于屏幕
    float3 posWS = centerWS
                 + rightWS * a.Position.x * scaleX
                 + upWS    * a.Position.y * scaleY;

    v.Position = mul(CameraBuffer.MatrixVP, float4(posWS, 1.0));
    v.TexCoord0 = a.TexCoord0;
    return v;
}

float4 PSMain(BillboardVaryings v) : SV_Target
{
    float4 tex = _BaseColorMap.Sample(Sampler__BaseColorMap, v.TexCoord0);
    float3 final = tex.rgb * _TintColor.rgb;
    return float4(final, tex.a * _TintColor.a);
}
