
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
