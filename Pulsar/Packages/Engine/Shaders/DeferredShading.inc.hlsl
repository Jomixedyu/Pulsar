#ifndef _DEFERRED_SHADING_INC
#define _DEFERRED_SHADING_INC

struct OutPixelDeferred
{
    // rgb:basecolor a:materialId
    float4 GBufferA : SV_TARGET0;
    // rgb:normal
    float4 GBufferB : SV_TARGET1;
    // r:metallic g:roughness b:AO a:specular
    float4 GBufferC : SV_TARGET2;
    // rgb:shadowColor a:NdotL
    float4 GBufferD : SV_TARGET3;
    // rgb:worldTangent a:anisotropy
    float4 GBufferE : SV_TARGET4;
    // rgb:customData  a:selectiveMask
    float4 GBufferF : SV_TARGET5;
};


#endif