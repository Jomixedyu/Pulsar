#ifndef _ENGINE_LIGHT_INC
#define _ENGINE_LIGHT_INC


struct LightRenderingData // 64
{
    float3 Position;
    float  CutOff;
    float3 Direction;
    float  Radius;
    float4 Color; // w intensity
    uint   Flags;
    float3 _Pad0;
};

struct LightsRenderingBufferData // 4096
{
    LightRenderingData Lights[63];
    uint PointCount;
    uint SpotCount;
    uint AreaCount;
    uint Flags;
    float4 _Pad0;
    float4 _Pad1;
    float4 _Pad2;
};

ConstantBuffer<LightsRenderingBufferData> LightBuffer : register(c0, space4);

#endif