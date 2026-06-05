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

    // 正确提取世界位置
    float3 centerWS = mul(RenderObjectBuffer.LocalToWorldMatrix, float4(0, 0, 0, 1)).xyz;
    // 正确提取各轴缩放：取 column 0 / column 1 的长度（HLSL M[i] 是 row，column 要跨行取）
    float scaleX = length(float3(RenderObjectBuffer.LocalToWorldMatrix[0].x,
                                 RenderObjectBuffer.LocalToWorldMatrix[1].x,
                                 RenderObjectBuffer.LocalToWorldMatrix[2].x));
    float scaleY = length(float3(RenderObjectBuffer.LocalToWorldMatrix[0].y,
                                 RenderObjectBuffer.LocalToWorldMatrix[1].y,
                                 RenderObjectBuffer.LocalToWorldMatrix[2].y));

    // 从 InvMatrixV（camera world transform）提取 Right / Up 列向量
    float3 rightWS = normalize(float3(CameraBuffer.InvMatrixV[0].x,
                                      CameraBuffer.InvMatrixV[1].x,
                                      CameraBuffer.InvMatrixV[2].x));
    float3 upWS    = normalize(float3(CameraBuffer.InvMatrixV[0].y,
                                      CameraBuffer.InvMatrixV[1].y,
                                      CameraBuffer.InvMatrixV[2].y));

    // 兼容 XY 平面和 XZ 平面的 mesh（默认 Plane 是 XZ 平面，y=0）
    float2 localOffset = float2(a.Position.x,
                                abs(a.Position.y) > 0.0001 ? a.Position.y : a.Position.z);

    // Billboard：用相机的 Right/Up 向量展开本地 quad，使平面始终平行于屏幕
    float3 posWS = centerWS
                 + rightWS * localOffset.x * scaleX
                 + upWS    * localOffset.y * scaleY;

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
