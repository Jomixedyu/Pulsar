#include "Common.inc.hlsl"
#include "DeferredShading.inc.hlsl"


Texture2D _BaseColor : register(t1, space3);
Texture2D _MRA       : register(t2, space3);

SamplerState DefaultSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

OutPixelDeferred main(InPixelAssembly v2f)
{
    OutPixelDeferred o;

    o.GBufferA = _BaseColor.Sample(DefaultSampler, v2f.TexCoord0);
    o.GBufferB = float4(v2f.WorldNormal, 1);
    o.GBufferC = _MRA.Sample(DefaultSampler, v2f.TexCoord0);
    float NdotL = dot(v2f.WorldNormal, -WorldBuffer.WorldSpaceLightVector.xyz);
    o.GBufferD = float4(0,0,0, NdotL);

    return o;
}

