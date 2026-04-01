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

struct LightShaderParameter
{
    float4 WorldPosition;
    float4 DirectionAndFalloffExponent;
    float4 Color;
    float2 SpotAngles;
    float2 SourceAndSoftSourceRadius;
};

struct LightsBufferData
{
    LightShaderParameter Lights[64];
};

ConstantBuffer<CameraData>      CameraBuffer : register(b0, space1);
ConstantBuffer<WorldData>       WorldBuffer  : register(b1, space1);
ConstantBuffer<LightsBufferData> LightBuffer : register(b2, space1);


#endif // _ENGINE_PER_PASS_INC
