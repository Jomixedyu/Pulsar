#ifndef _SHADERLIBRARY_MAPPING_HLSL_
#define _SHADERLIBRARY_MAPPING_HLSL_


#include "Math.inc.hlsl"

namespace map
{
    float2 MatCapMapping(float3 normalWS)
    {
        float3 normalVS = TransformWorldToViewDir(normalWS);
        return normalVS.xy * 0.5 + 0.5;
    }

    float4 TriPlanarMapping(shl::ISample2D sampleable, float3 posWS, float3 normalWS)
    {
        half3 weight = pow(normalWS, 1);
        half3 uvWeight = weight / (weight.x + weight.y + weight.z);
        half4 a = sampleable.Sample(posWS.xy) * uvWeight.z;
        half4 b = sampleable.Sample(posWS.xz) * uvWeight.y;
        half4 c = sampleable.Sample(posWS.zy) * uvWeight.x;
        return a + b + c;
    }
    

    float2 SphericalMapping_Level2(float3 v)
    {
        const float2 invAtan = float2(0.1591, 0.3183);
        float2 uv = float2(math::ATan2_Level2(v.z, v.x), math::ASin_Level2(v.y));
        uv *= invAtan;
        uv += 0.5;
        return uv;
    }
    float2 SphericalMapping_Level1(float3 v)
    {
        const float2 invAtan = float2(0.1591, 0.3183);
        float2 uv = float2(math::ATan2_Level1(v.z, v.x), math::ASin_Level1(v.y));
        uv *= invAtan;
        uv += 0.5;
        return uv;
    }
    float2 SphericalMapping_Level0(float3 v)
    {
        const float2 invAtan = float2(0.1591, 0.3183);
        float2 uv = float2(atan2(v.z, v.x), asin(v.y));
        uv *= invAtan;
        uv += 0.5;
        return uv;
    }
    float2 SphericalMapping(float3 v)
    {
        return SphericalMapping_Level1(v);
    }
    
    float3 InvSphericalMapping_Level0(float2 uv)
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
    float3 InvSphericalMapping_Level1(float2 uv)
    {
        float phi = (uv.x - 0.5) * 2.0 * 3.14159265359;
        float theta = (uv.y - 0.5) * 3.14159265359;
                
        float cosTheta = math::Cos_Level1(theta);
        float3 direction;
        direction.x = cosTheta * math::Cos_Level1(phi);
        direction.y = math::Sin_Level1(theta);
        direction.z = cosTheta * math::Sin_Level1(phi);

        return normalize(direction);
    }
    float3 InvSphericalMapping(float2 uv)
    {
        return InvSphericalMapping_Level1(uv);
    }

    
    float3 InvOctahedralMapping(float2 uv)
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

    float2 OctahedralMapping(float3 normDir)
    {
        float2 octCoord = normDir.xy / dot(1, abs(normDir));

        if (normDir.z < 0.0)
        {
            octCoord = (1.0 - abs(octCoord).yx) * sign(octCoord);
        }

        return octCoord * 0.5 + 0.5;
    }
}

#endif
