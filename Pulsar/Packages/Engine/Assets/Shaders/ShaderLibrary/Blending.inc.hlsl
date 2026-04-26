


namespace blending
{
    namespace __detail__
    {
        float3 RGB2HSV(float3 c)
        {
            const float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            float4 p = lerp(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g));
            float4 q = lerp(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r));
            float d = q.x - min(q.w, q.y);
            const float e = 1.0e-4;
            return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }

        float3 HSV2RGB(float3 c)
        {
            const float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * lerp(K.xxx, saturate(p - K.xxx), c.y);
        }
    
        float GetLuminosity(float3 c)
        {
            return 0.3 * c.r + 0.59 * c.g + 0.11 * c.b;
        }
    
        float3 SetLuminosity(float3 c, float lum)
        {
            float d = lum - GetLuminosity(c);
            c.rgb += float3(d,d,d);

            // clip back into legal range
            lum = GetLuminosity(c);
            float cMin = min(c.r, min(c.g, c.b));
            float cMax = max(c.r, max(c.g, c.b));

            if(cMin < 0)
                c = lerp(float3(lum,lum,lum), c, lum / (lum - cMin));

            if(cMax > 1)
                c = lerp(float3(lum,lum,lum), c, (1 - lum) / (cMax - lum));

            return c;
        }
    }

    /**
     * 颜色
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 Color(float3 base, float3 blend)
    {
        float3 mix = __detail__::RGB2HSV(blend);
        mix.b = __detail__::RGB2HSV(base).b;
        return __detail__::HSV2RGB(mix);
    }

    /**
     * 颜色加深
     * @param base 
     * @param blend 
     * @return 
     */
    inline float ColorBurn(float base, float blend)
    {
        [flatten]
        if (base >= 1.0)
            return 1.0;
        else if (blend <= 0.0)
            return 0.0;
        else    
            return 1.0 - (1.0 - base) / blend;
    }

    /**
     * 颜色加深
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 ColorBurn(float3 base, float3 blend)
    {
        return float3(
            ColorBurn(base.r, blend.r),
            ColorBurn(base.g, blend.g),
            ColorBurn(base.b, blend.b));
    }

    /**
     * 颜色减淡
     * @param base 
     * @param blend 
     * @return 
     */
    float ColorDodge(float base, float blend)
    {
        [flatten]
        if (base <= 0.0)
            return 0.0;
        if (blend >= 1.0)
            return 1.0;
        else
            return min(1.0, base / (1.0-blend));
    }
    /**
     * 颜色减淡
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 ColorDodge(float3 base, float3 blend)
    {
        return float3(
            ColorDodge(base.r, blend.r),
            ColorDodge(base.g, blend.g),
            ColorDodge(base.b, blend.b)
        );
    }

    /**
     * 变暗
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 Darken(float3 base, float3 blend)
    {
        return min(base, blend);
    }
    
    /**
     * 深色
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 DarkerColor(float3 base, float3 blend)
    {
        return lerp(blend, base, step(__detail__::GetLuminosity(base), __detail__::GetLuminosity(blend)));
    }

    /**
     * 差值
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 Difference(float3 base, float3 blend)
    {
        return abs(base - blend);
    }

    /**
     * 划分
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 Divide(float3 base, float3 blend)
    {
        return base.rgb / blend.rgb;
    }

    /**
     * 排除
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 Exclusion(float3 base, float3 blend)
    {
        return base + blend - 2 * base * blend;
    }

    /**
     * 强光
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 HardLight(float3 base, float3 blend)
    {
        return lerp(
            1 - 2 * (1 - base) * (1 - blend),
            2 * base * blend,
            step(blend, 0.5)
        );
    }

    /**
     * 实色混合
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 HardMix(float3 base, float3 blend)
    {
        return floor(base.rgb + blend.rgb);
    }

    /**
     * 色相
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 Hue(float3 base, float3 blend)
    {
        float3 mix = __detail__::RGB2HSV(base);
        mix.r = __detail__::RGB2HSV(blend).r;
        return __detail__::HSV2RGB(mix);
    }

    /**
     * 变亮
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 Lighten(float3 base, float3 blend)
    {
        return max(base.rgb, blend.rgb);
    }

    /**
     * 浅色
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 LighterColor(float3 base, float3 blend)
    {
        return lerp(
            blend,
            base,
            step(__detail__::GetLuminosity(blend), __detail__::GetLuminosity(base))
        );
    }

    /**
     * 线性加深
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 LinearBurn(float3 base, float3 blend)
    {
        return max(0, base + blend - 1);
    }

    /**
     * 线性减淡
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 LinearDodge(float3 base, float3 blend)
    {
        return min(1, base + blend);
    }

    /**
     * 线性光
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 LinearLight(float3 base, float3 blend)
    {
        return lerp(
            LinearDodge(base,2 * (blend - 0.5)),
            LinearBurn(base, 2 * blend),
            step(blend, 0.5)
        );
    }

    /**
     * 明度
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 Luminosity(float3 base, float3 blend)
    {
        return __detail__::SetLuminosity(base, __detail__::GetLuminosity(blend));
    }

    /**
     * 正片叠底
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 Multiply(float3 base, float3 blend)
    {
        return base * blend;
    }

    /**
     * 叠加
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 Overlay(float3 base, float3 blend)
    {
        return lerp(
            2 * blend * base, //base < 0.5
            1.0 - 2 * (1.0 - blend) * (1.0 - base), // base > 0.5
            step(0.5, base) //if
        );
    }

    /**
     * 点光
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 PinLight(float3 base, float3 blend)
    {
        return lerp(
            max(base, 2 * (blend - 0.5)),
            min(base, 2 * blend),
            step(blend, 0.5)
        );
    }

    /**
     * 饱和度
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 Saturation(float3 base, float3 blend)
    {
        float3 mix = __detail__::RGB2HSV(base);
        mix.g = __detail__::RGB2HSV(blend).g;
        return __detail__::HSV2RGB(mix);
    }

    /**
     * 滤色
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 Screen(float3 base, float3 blend)
    {
        return base + blend - base * blend;
    }

    /**
     * 柔光
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 SoftLight(float3 base, float3 blend)
    {
        float3 r1 = 2.0 * base * blend + base * base * (1.0 - 2.0 * blend);
        float3 r2 = sqrt(base) * (2.0 * blend - 1.0) + 2.0 * base * (1.0 - blend);
        float3 t = step(0.5, blend);
        return r2 * t + (1.0 - t) * r1;
    }

    /**
     * 减去
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 Subtract(float3 base, float3 blend)
    {
        return max(0, base - blend);
    }

    /**
     * 亮光
     * @param base 
     * @param blend 
     * @return 
     */
    inline float3 VividLight(float3 base, float3 blend)
    {
        return lerp(
            ColorDodge(base,2 * (blend - 0.5)),
            ColorBurn(base, 2 * blend),
            step(blend, 0.5)
        );
    }

}
