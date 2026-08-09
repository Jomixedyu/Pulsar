
float FoamTravelingWave(float distToShore, float time,
                        float waveDistance, float waveSpeed,
                        float waveCycle,    float foamWidth, float foamSharp,
                        float shoreEase)
{
    const float halfPi = 1.5707963;

    float d = saturate(distToShore / waveDistance);

    float dPhase = pow(d, shoreEase);
    float phase  = frac((time * waveSpeed + dPhase) / waveCycle);

    float p      = saturate(phase / foamWidth);
    float active = step(phase, foamWidth);

    float rise  = pow(sin(p * halfPi), foamSharp);
    float decay = pow(1.0 - p, 1.5);

    float foam = rise * decay * active;
    foam *= pow(1.0 - d, 0.3);
    return saturate(foam);
}


float TwoSideAlpha(float v, float offset = -0.5, float scale = 1, float softness = 1)
{
    float c = abs((v - offset) * 2);
    float start = 1 - scale;
    float soft = softness * scale;
    return saturate(1 - smoothstep(start, start + soft, c));
}


half3 RimLight2(half NoL, half NoV, half NoLOffset = 0,
  float frontWidth = 0.85, float frontSoftness = 0.05, half3 frontColor = half3(1, 0.75, 0.2), half frontIntensity = 1,
  float backWidth = 0.85, float backSoftness = 0.05, half3 backColor = half3(0.3, 0.5, 0.7), half backIntensity = 1
)
{
    NoL = NoL + NoLOffset;
    half fr = saturate(1 - NoV);

    float frontMin = saturate(frontWidth - frontSoftness);
    float frontMax = saturate(frontWidth + frontSoftness);
    half frontMask = (half)saturate((fr - frontMin) / (frontMax - frontMin)) * saturate(NoL);
    half3 frontPart = frontColor * frontIntensity * frontMask;

    float backMin = saturate(backWidth - backSoftness);
    float backMax = saturate(backWidth + backSoftness);
    half backMask = (half)saturate((fr - backMin) / (backMax - backMin)) * saturate(NoL * -1);
    half3 backPart = backColor * backIntensity * backMask;

    return frontPart + backPart;
}