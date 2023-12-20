
struct OutPixelDeferred
{
    // rgb:albedo
    float4 Albedo       : SV_TARGET0;
    float4 WorldNormal  : SV_TARGET1;
    // r:metallic g:roughness b:AO a:specular
    float4 MRAS         : SV_TARGET2;
    // rgb:tangent; a:anisotropy
    float4 WorldTangent : SV_TARGET3;

};