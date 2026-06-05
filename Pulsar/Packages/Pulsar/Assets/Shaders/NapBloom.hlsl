#include "Pulsar/Assets/Shaders/Blit.inc.hlsl"
#include "Pulsar/Assets/Shaders/PostProcessing.inc.hlsl"

struct BloomParams
{
    float2 TexelSize;
    float2 Direction;
    float  Threshold;
    float  Intensity;
    int    SampleMode;
    int    _Padding0;
    int    _Padding1;
    int    _Padding2;
};
ConstantBuffer<BloomParams> BloomParamBuffer : register(b8, space1);

Texture2D _BloomLayer0 : register(t4, space1);
SamplerState Sampler__BloomLayer0 : register(s4, space1);
Texture2D _BloomLayer1 : register(t5, space1);
SamplerState Sampler__BloomLayer1 : register(s5, space1);
Texture2D _BloomLayer2 : register(t6, space1);
SamplerState Sampler__BloomLayer2 : register(s6, space1);
Texture2D _BloomLayer3 : register(t7, space1);
SamplerState Sampler__BloomLayer3 : register(s7, space1);

static const float2 CORE0[6] = {
    float2(-2.2226281, 0.1334844),
    float2(-4.0952849, 0.0057047),
    float2(-0.437803, 0.501892),
    float2(1.320767, 0.3234969),
    float2(3.147974, 0.0348785),
    float2(5, 0.0005436),
};

static const float2 CORE1[9] = {
    float2(-5.2274981, 0.0151298),
    float2(-7.1588202, 0.0009649),
    float2(-3.3147621, 0.1009583),
    float2(-1.417412, 0.2889),
    float2(0.4722446, 0.3564036),
    float2(2.364548, 0.1897708),
    float2(4.268898, 0.0434656),
    float2(6.1908078, 0.0042536),
    float2(8, 0.0001532),
};

static const float2 CORE2[16] = {
    float2(-12.2933798, 0.0009471),
    float2(-14.26509, 0.0001463),
    float2(-10.3233604, 0.0046463),
    float2(-8.3548632, 0.0172796),
    float2(-6.3876772, 0.0487266),
    float2(-4.4215422, 0.1042022),
    float2(-2.456162, 0.1690129),
    float2(-0.4912108, 0.207937),
    float2(1.473654, 0.1940565),
    float2(3.4387779, 0.1373738),
    float2(5.4044962, 0.0737621),
    float2(7.3711209, 0.0300379),
    float2(9.338933, 0.0092757),
    float2(11.3081703, 0.0021717),
    float2(13.2790203, 0.0003854),
    float2(15, 3.8788519e-005),
};

static const float2 CORE3[20] = {
    float2(-16.3224392, 0.0003934),
    float2(-18.3031006, 8.2805367e-005),
    float2(-14.3424101, 0.0015638),
    float2(-12.3629599, 0.0052015),
    float2(-10.3840103, 0.0144784),
    float2(-8.4055147, 0.033726),
    float2(-6.4273849, 0.0657471),
    float2(-4.449542, 0.1072673),
    float2(-2.4719019, 0.1464697),
    float2(-0.4943747, 0.1673879),
    float2(1.48313, 0.1601027),
    float2(3.4607019, 0.1281654),
    float2(5.4384332, 0.0858689),
    float2(7.416409, 0.0481489),
    float2(9.3947134, 0.0225949),
    float2(11.3734198, 0.0088735),
    float2(13.3526201, 0.0029162),
    float2(15.3323498, 0.000802),
    float2(17.3126907, 0.0001846),
    float2(19, 2.509824e-005),
};

float4 DownsamplePSMain(float4 position : SV_Position, float2 texcoord : TEXCOORD0) : SV_Target
{
    float2 halfPixel = BloomParamBuffer.TexelSize;
    float4 a = PP_InColor.Sample(Sampler_PP_InColor, texcoord + float2(-halfPixel.x, -halfPixel.y));
    float4 b = PP_InColor.Sample(Sampler_PP_InColor, texcoord + float2( halfPixel.x, -halfPixel.y));
    float4 c = PP_InColor.Sample(Sampler_PP_InColor, texcoord + float2(-halfPixel.x,  halfPixel.y));
    float4 d = PP_InColor.Sample(Sampler_PP_InColor, texcoord + float2( halfPixel.x,  halfPixel.y));

    float4 color = (a + b + c + d) * 0.25;
    color.rgb = max(color.rgb - BloomParamBuffer.Threshold, 0);
    return color;
}

float4 GaussFilterPSMain(float4 position : SV_Position, float2 texcoord : TEXCOORD0) : SV_Target
{
    float4 color = 0;
    float2 dir = BloomParamBuffer.Direction;
    float2 texel = BloomParamBuffer.TexelSize;
    int mode = BloomParamBuffer.SampleMode;

    if (mode == 0)
    {
        [unroll]
        for (int i = 0; i < 6; i++)
            color += PP_InColor.Sample(Sampler_PP_InColor, texcoord + dir * CORE0[i].x * texel) * CORE0[i].y;
    }
    else if (mode == 1)
    {
        [unroll]
        for (int i = 0; i < 9; i++)
            color += PP_InColor.Sample(Sampler_PP_InColor, texcoord + dir * CORE1[i].x * texel) * CORE1[i].y;
    }
    else if (mode == 2)
    {
        [unroll]
        for (int i = 0; i < 16; i++)
            color += PP_InColor.Sample(Sampler_PP_InColor, texcoord + dir * CORE2[i].x * texel) * CORE2[i].y;
    }
    else if (mode == 3)
    {
        [unroll]
        for (int i = 0; i < 20; i++)
            color += PP_InColor.Sample(Sampler_PP_InColor, texcoord + dir * CORE3[i].x * texel) * CORE3[i].y;
    }

    return color;
}

float4 CombinePSMain(float4 position : SV_Position, float2 texcoord : TEXCOORD0) : SV_Target
{
    float3 bloom = _BloomLayer0.Sample(Sampler__BloomLayer0, texcoord).rgb * 0.29688;
    bloom += _BloomLayer1.Sample(Sampler__BloomLayer1, texcoord).rgb * 0.29688;
    bloom += _BloomLayer2.Sample(Sampler__BloomLayer2, texcoord).rgb * 0.25781;
    bloom += _BloomLayer3.Sample(Sampler__BloomLayer3, texcoord).rgb * 0.14844;

    const float threshould = 0.3;
    float3 bloomSup = pow(abs(bloom) / threshould, 1.0 / 3.0) - (1-threshould);
    bloom = lerp(bloom, bloomSup, bloom >= threshould);
    bloom *= BloomParamBuffer.Intensity;

    float4 original = PP_InColor.Sample(Sampler_PP_InColor, texcoord);
    float3 color = original.rgb + bloom.rgb;

    float alpha = original.a;
    return float4(color, alpha);
}
