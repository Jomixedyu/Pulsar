#ifndef _SHADERLIBRARY_MAPPING_HLSL_
#define _SHADERLIBRARY_MAPPING_HLSL_


#include "Math.inc.hlsl"

float map_ReconstructNormalZ(float2 normalRG)
{
    return sqrt(saturate(1.0 - dot(normalRG, normalRG)));
}

float3 map_DecodeNormalRG(float2 normalRG)
{
    return float3(normalRG, map_ReconstructNormalZ(normalRG));
}

float3 map_DecodeNormalMapRG(float2 normalMapSample)
{
    float2 normalRG = normalMapSample * 2.0 - 1.0;
    return map_DecodeNormalRG(normalRG);
}

float2 map_MatCapMapping(float3 normalVS)
{
    return normalVS.xy * 0.5 + 0.5;
}

// float4 map_TriPlanarMapping(Texture2D tex, SamplerState sampler, float3 posWS, float3 normalWS)
// {
//     half3 weight = pow(normalWS, 1);
//     half3 uvWeight = weight / (weight.x + weight.y + weight.z);
//     half4 a = tex.Sample(sampler, posWS.xy) * uvWeight.z;
//     half4 b = tex.Sample(sampler, posWS.xz) * uvWeight.y;
//     half4 c = tex.Sample(sampler, posWS.zy) * uvWeight.x;
//     return a + b + c;
// }

float2 map_SphericalMapping_Level2(float3 v)
{
    const float2 invAtan = float2(0.1591, 0.3183);
    float2 uv = float2(math_ATan2_Level2(v.z, v.x), math_ASin_Level2(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}
float2 map_SphericalMapping_Level1(float3 v)
{
    const float2 invAtan = float2(0.1591, 0.3183);
    float2 uv = float2(math_ATan2_Level1(v.z, v.x), math_ASin_Level1(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}
float2 map_SphericalMapping_Level0(float3 v)
{
    const float2 invAtan = float2(0.1591, 0.3183);
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}
float2 map_SphericalMapping(float3 v)
{
    return map_SphericalMapping_Level1(v);
}

float3 map_InvSphericalMapping_Level0(float2 uv)
{
    float phi = (uv.x - 0.5) * 2.0 * 3.14159265359;
    float theta = (uv.y - 0.5) * 3.14159265359;
    float cosTheta = cos(theta);
    float3 direction;
    direction.x = cosTheta * cos(phi);
    direction.y = sin(theta);
    direction.z = cosTheta * sin(phi);
    return normalize(direction);
}
float3 map_InvSphericalMapping_Level1(float2 uv)
{
    float phi = (uv.x - 0.5) * 2.0 * 3.14159265359;
    float theta = (uv.y - 0.5) * 3.14159265359;
    float cosTheta = math_Cos_Level1(theta);
    float3 direction;
    direction.x = cosTheta * math_Cos_Level1(phi);
    direction.y = math_Sin_Level1(theta);
    direction.z = cosTheta * math_Sin_Level1(phi);
    return normalize(direction);
}
float3 map_InvSphericalMapping(float2 uv)
{
    return map_InvSphericalMapping_Level1(uv);
}

float3 map_InvOctahedralMapping(float2 uv)
{
    float2 octCoord = uv * 2.0 - 1.0;
    float z = 1.0 - abs(octCoord.x) - abs(octCoord.y);
    float3 direction = float3(octCoord, z);
    if (z < 0.0)
    {
        direction.xy = (1.0 - abs(octCoord).yx) * sign(octCoord);
    }
    return normalize(direction);
}

float2 map_OctahedralMapping(float3 normDir)
{
    float2 octCoord = normDir.xy / dot(1, abs(normDir));
    if (normDir.z < 0.0)
    {
        octCoord = (1.0 - abs(octCoord).yx) * sign(octCoord);
    }
    return octCoord * 0.5 + 0.5;
}

#endif
