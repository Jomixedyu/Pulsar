#include "DeferredShading.inc.hlsl"


Texture2D _GBufferA : register(t0, space2);
Texture2D _GBufferB : register(t1, space2);
Texture2D _GBufferC : register(t2, space2);
Texture2D _GBufferD : register(t3, space2);
Texture2D _GBufferE : register(t4, space2);

SamplerState _GBufferASampler : register(s0, space2);
SamplerState _GBufferBSampler : register(s1, space2);
SamplerState _GBufferCSampler : register(s2, space2);
SamplerState _GBufferDSampler : register(s3, space2);
SamplerState _GBufferESampler : register(s4, space2);



float4 LightingPass(MaterialAttributes mat)
{
    float4 color;

    [branch]
    if (mat.ShadingModel & SHADING_MODEL_UNLIT)
    {
        color = mat.EmissiveColor;
    }

    [branch]
    if (mat.ShadingModel & SHADING_MODEL_DEFAULTPBR)
    {
        // todo..
    }

    return color;
}

float4 main() : SV_TARGET
{
    MaterialAttributes attr;

    return LightingPass(attr);
}