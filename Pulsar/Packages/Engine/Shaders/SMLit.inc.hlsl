#ifndef _SMLit_INC
#define _SMLit_INC

#include "MaterialAttributes.inc.hlsl"
#include "Common.inc.hlsl"
#include "MeshRenderer.inc.hlsl"

const float PI = 3.14159265358979;

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

const float3 lightPositions[3] = 
{
    float3(1.5, 1.5, -1.5),
    float3(2, 2, -2),
    float3(1.25, 1.25, -1.25),
    float3(4.5, 4.5, -4.5)
};
const float3 lightColors[3] = 
{
    float3(1, 1, 1),
    float3(1, 1, 1),
    float3(1, 1, 1),
    float3(1, 1, 1),
};



float3 kS_CookTorrance(float3 N, float3 V, float3 L, float D, float G, float3 F)
{
    float3 numerator  = D * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    float3 specular = numerator / denominator;
    return specular;
}

float3 kD_Lambert(float3 kS, float3 albedo, float3 metallic)
{
    return ((float3(1.0) - kS) * (1.0 - metallic)) * (albedo / PI);
}


float4 ShadingModel_Lit(
    MaterialAttributes attr, InPixelAssembly v2f
)
{
    float3 N = normalize(v2f.WorldNormal);
    float3 V = normalize(TargetBuffer.CamPosition.xyz - v2f.WorldPosition.xyz);

    float3 F0 = float3(0.04); 
    F0 = lerp(F0, attr.BaseColor, attr.Metallic);
    
    float3 Lo = float3(0.0);

    //direction light
    {
        float3 L = -WorldBuffer.WorldSpaceLightVector;
        float3 H = normalize(V + L);
        float3 dirLightRadiance = WorldBuffer.WorldSpaceLightColor.xyz * WorldBuffer.WorldSpaceLightColor.w;

        float  D = DistributionGGX(N, H, attr.Roughness);   
        float  G = GeometrySmith(N, V, L, attr.Roughness);      
        float3 F = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

        // Cook-Torrance BRDF
        float3 kS_cookTorrance = kS_CookTorrance(N, V, L, D, G, F);
        float3 kD_lambert = kD_Lambert(F, attr.BaseColor, attr.Metallic); 
        float3 BRDF = kD_lambert + kS_cookTorrance;

        float NdotL = max(dot(N, L), 0.0);        
        
        
        Lo += BRDF * dirLightRadiance * NdotL;
    }
    
    for(int i = 0; i < 4; ++i) 
    {

        float3 L = normalize(lightPositions[i] - v2f.WorldPosition.xyz);
        float3 H = normalize(V + L);
        float  distance = length(lightPositions[i] - v2f.WorldPosition.xyz);
        float  attenuation = 1.0 / (distance * distance);
        float3 radiance = lightColors[i] * attenuation;

        float  D = DistributionGGX(N, H, attr.Roughness);   
        float  G = GeometrySmith(N, V, L, attr.Roughness);      
        float3 F = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

        // Cook-Torrance BRDF
        float3 kS_cookTorrance = kS_CookTorrance(N, V, L, D, G, F);
        float3 kD_lambert = kD_Lambert(F, attr.BaseColor, attr.Metallic); 
        float3 BRDF = kD_lambert + kS_cookTorrance;

        float NdotL = max(dot(N, L), 0.0);        
        
        Lo += BRDF * radiance * NdotL;
    }
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    float3 ambient = float3(0.03) * attr.BaseColor * attr.AmbientOcclusion;

    float3 color = ambient + Lo;

    // HDR tonemapping
    // color = color / (color + float3(1.0));
    // gamma correct
    // color = pow(color, float3(1.0/2.2)); 

    return float4(color, 1.0);
}

#endif //_SMLit_INC