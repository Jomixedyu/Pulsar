#include "PostProcessing.inc.hlsl"

#define FLAGS_GAMMA 0x01
#define FLAGS_EnableCheckerBackground 0x02
#define FLAGS_CHANNEL_R 0x04
#define FLAGS_CHANNEL_G 0x08
#define FLAGS_CHANNEL_B 0x10
#define FLAGS_CHANNEL_A 0x20
#define FLAGS_NORMALMAP 0x40

TextureCube _Image : register(t0, USER_DESCSET);
// TextureCube _Image : register(t1, USER_DESCSET);
SamplerState _ImageSampler : register(s0, USER_DESCSET);

float4 main(PPVSOutput v2f) : SV_TARGET
{
    float4 output = float4(0,0,0,1);
    
    float u = v2f.TexCoord0.x;
    float v = v2f.TexCoord0.y;

   // 将UV空间划分为4列3行
    int faceCol = floor(u * 4.0);
    int faceRow = floor(v * 3.0);
    
    // 计算面内局部UV坐标
    float localU = frac(u * 4.0);
    float localV = frac(v * 3.0);
    
    // 转换为立方体贴图坐标系（-1到1）
    float2 sphereUV = float2(localU * 2.0 - 1.0, 
                            (1.0 - localV) * 2.0 - 1.0);
    
    float3 dir = float3(0, 0, 0);
    
    // 顶行（+Y）
    if (faceRow == 0) {
        if (faceCol == 1) {  // 中间列
            dir = float3(sphereUV.x, 1.0, -sphereUV.y);
        }
        else return float4(0,0,0,1);
    }
    // 中间行（前/后/左/右）
    else if (faceRow == 1) {
        if (faceCol == 0) {      // 左面 (-X)
            dir = float3(-1.0, sphereUV.y, sphereUV.x);
        }
        else if (faceCol == 1) { // 前面 (+Z)
            dir = float3(sphereUV.x, sphereUV.y, 1.0);
        }
        else if (faceCol == 2) { // 右面 (+X)
            dir = float3(1.0, sphereUV.y, -sphereUV.x);
        }
        else if (faceCol == 3) { // 后面 (-Z)
            dir = float3(-sphereUV.x, sphereUV.y, -1.0);
        }
    }
    // 底行（-Y）
    else if (faceRow == 2) {
        if (faceCol == 1) {  // 中间列
            dir = float3(sphereUV.x, -1.0, sphereUV.y);
        }
        else return float4(0,0,0,1);
    }
    
    // 无效区域返回黑色
    if (dir.x == 0 && dir.y == 0 && dir.z == 0)
        return float4(0,0,0,1);
    
    // 采样立方体贴图
    float4 color = _Image.Sample(_ImageSampler, normalize(dir));


    return color;
}