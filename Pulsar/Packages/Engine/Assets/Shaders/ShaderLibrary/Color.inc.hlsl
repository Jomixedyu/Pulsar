
#ifndef PI
#define PI 3.1415926535897932384626
#endif

namespace color
{
    float3 HsvToRgb(float3 c)
    {
        float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * lerp(K.xxx, saturate(p - K.xxx), c.y);
    }

    float3 RgbToHsv(float3 c)
    {
        float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        float4 p = lerp(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g));
        float4 q = lerp(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r));
        float d = q.x - min(q.w, q.y);
        float e = 1.0e-10;
        return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }

    float3 HslToRgb(float3 HSL)
    {
        float R = abs(HSL.x * 6 - 3) - 1;
        float G = 2 - abs(HSL.x * 6 - 2);
        float B = 2 - abs(HSL.x * 6 - 4);
        float3 RGB = saturate(float3(R, G, B));
        float C = (1 - abs(2 * HSL.z - 1)) * HSL.y;
        return (RGB - 0.5) * C + HSL.z;
    }

    float3 RgbToYCoCg(float3 rgb)
    {
        float Y = (0.25 * rgb.r) + (0.5 * rgb.g) + (0.25 * rgb.b);
        float Co = (0.5 * rgb.r) + (0.0 * rgb.g) - (0.5 * rgb.b);
        float Cg = (-0.25 * rgb.r) + (0.5 * rgb.g) - (0.25 * rgb.b);
        return float3(Y, Co, Cg);
    }

    float3 YCoCgToRgb(float3 YCoCg)
    {
        float Y = YCoCg.r;
        float Co = YCoCg.g;
        float Cg = YCoCg.b;
        return float3(
            Y + Co - Cg,
            Y + Cg,
            Y - Co - Cg);
    }

    float3 RgbToHsl(float3 RGB)
    {
        float Epsilon = 1e-10;
        float4 P = (RGB.g < RGB.b) ? float4(RGB.bg, -1.0, 2.0 / 3.0) : float4(RGB.gb, 0.0, -1.0 / 3.0);
        float4 Q = (RGB.r < P.x) ? float4(P.xyw, RGB.r) : float4(RGB.r, P.yzx);
        float C = Q.x - min(Q.w, Q.y);
        float H = abs((Q.w - Q.y) / (6 * C + Epsilon) + Q.z);
        float L = Q.x - C * 0.5;
        float S = C / (1 - abs(L * 2 - 1) + Epsilon);
        return float3(H, S, L);
    }


    
    namespace __detail__
    {
        float3 RotateAboutAxis(float4 NormalizedRotationAxisAndAngle, float3 PositionOnAxis, float3 Position)
        {
            // Project Position onto the rotation axis and find the closest point on the axis to Position
            float3 ClosestPointOnAxis = PositionOnAxis + NormalizedRotationAxisAndAngle.xyz * dot(
                NormalizedRotationAxisAndAngle.xyz, Position - PositionOnAxis);
            // Construct orthogonal axes in the plane of the rotation
            float3 UAxis = Position - ClosestPointOnAxis;
            float3 VAxis = cross(NormalizedRotationAxisAndAngle.xyz, UAxis);
            float CosAngle;
            float SinAngle;
            sincos(NormalizedRotationAxisAndAngle.w, SinAngle, CosAngle);
            // Rotate using the orthogonal axes
            float3 R = UAxis * CosAngle + VAxis * SinAngle;
            // Reconstruct the rotated world space position
            float3 RotatedPosition = ClosestPointOnAxis + R;
            // Convert from position to a position offset
            return RotatedPosition - Position;
        }
        float3 RotateAboutAxis(
            float3 NormalizedRotationAxis,
            float RotationAngle,
            float3 PivotPoint,
            float3 Position)
        {
            return RotateAboutAxis(float4(NormalizedRotationAxis, RotationAngle), PivotPoint, Position);
        }
    }

    float3 HueShift(float shift, float3 color)
    {
        return __detail__::RotateAboutAxis(
            float4(normalize(float3(1, 1, 1)), shift * PI * 2),
            float3(0, 0, 0),
            color) + color;
    }
    
    float3 Desaturate(float3 value, float3 saturation)
    {
        // Saturation = Colorfulness / Brightness.
        // https://munsell.com/color-blog/difference-chroma-saturation/
        float  mean = (value.r + value.g + value.b) * 0.33333333;
        float3 dev  = value - mean;

        return mean + dev * saturation;
    }
    
    float3 CheapModulation(float3 color, float hueShift, float sat, float lum)
    {
        return Desaturate(HueShift(hueShift, color), sat)  * lum;
    }

    float3 CheapContrast(float3 i,  float contrast)
    {
        return saturate(lerp(0 - contrast, 1 + contrast, i));
    }
    
    float3 Pallette(float3 x, float3 yoffset)
    {
        float3 y = (float)1.0 - x * x;
        return saturate(y - yoffset);
    }

    float3 Spectral(float w)
    {
        float x = saturate(w * (float)0.0033333 - (float)1.33333);
        const float3 c1 = float3(3.54585104f, 2.93225262f, 2.41593945f);
        const float3 x1 = float3(0.69549072f, 0.49228336f, 0.27699880f);
        const float3 y1 = float3(0.02312639f, 0.15225084f, 0.52607955f);
        const float3 c2 = float3(3.90307140f, 3.21182957f, 3.96587128f);
        const float3 x2 = float3(0.11748627f, 0.86755042f, 0.66077860f);
        const float3 y2 = float3(0.84897130f, 0.88445281f, 0.73949448f);
        return Pallette(c1 * (x - x1), y1) + Pallette(c2 * (x - x2), y2);
    }
    
    float3 IridescenceColor(float3 viewDirWS, float3 normalWS, float offset, float intensity)
    {
        float3 normal = normalWS;
            	
        float f = dot(normal, viewDirWS);

        float d = offset * (float)100.0;
        float u = (float)2.66666 * d * f;
            	
        float3 color = 0;
        [unroll]
        for (int i = 1; i < 7; i++)
        {
            float w = u / (i + (float)0.5);
            color.rgb += Spectral(w);
        }
        return saturate(color);
    }
}
