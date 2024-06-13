#ifndef _ENGINE_MATERIALATTRIBUTES_INC
#define _ENGINE_MATERIALATTRIBUTES_INC

struct MaterialAttributes
{
    float3 BaseColor;
    float  Metallic;
    float  Specular;
    float  Roughness;
    float  Anisotropy;
    float3 EmissiveColor;
    float  Opacity;
    float  OpacityMask;
    float3 Normal;
    float3 Tangent;
    float3 SubsurfaceColor;
    float  AmbientOcclusion;
    uint   ShadingModel;
};

#endif