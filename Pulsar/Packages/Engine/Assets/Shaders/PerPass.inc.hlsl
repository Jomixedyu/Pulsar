#ifndef _ENGINE_PER_PASS_INC
#define _ENGINE_PER_PASS_INC

// Per-Pass Constants (set 1 / space1)
// Bound once per render pass, shared by all draw calls in the pass.

struct CameraData
{
    float4x4 MatrixV;
    float4x4 InvMatrixV;
    float4x4 MatrixP;
    float4x4 InvMatrixP;
    float4x4 MatrixVP;
    float4x4 InvMatrixVP;
    float4   CamPosition;
    float    CamNear;
    float    CamFar;
    float2   Resolution;
    float4   _Padding1;
    float4   _Padding2;
    float4x4 _Padding3;
};

struct WorldData
{
    float4 WorldSpaceLightVector;
    float4 WorldSpaceLightColor;
    float4 SkyLightColor;
    float  TotalTime;
    float  DeltaTime;
    uint   LightParameterCount;
    float  _Padding0;
};

struct LightRenderingData
{
    float3 Position;
    float  CutOff;
    float3 Direction;
    float  Radius;
    float4 Color; // w intensity
    uint   Flags;
    float3 _Pad0;
};

struct LightsBufferData
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

ConstantBuffer<CameraData>      CameraBuffer : register(b0, space1);
ConstantBuffer<WorldData>       WorldBuffer  : register(b1, space1);
ConstantBuffer<LightsBufferData> LightBuffer : register(b2, space1);

// Legacy aliases for compatibility during migration
#define TargetBuffer CameraBuffer

#endif // _ENGINE_PER_PASS_INC
