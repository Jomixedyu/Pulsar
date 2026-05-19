
float FoamTravelingWave(float distToShore, float time,
                        float waveDistance, float waveSpeed,
                        float waveCycle,    float foamWidth, float foamSharp,
                        float shoreEase /* 1.0=不变, 2.0=岸边变慢, 0.5=岸边更快 */)
{
    const float halfPi = 1.5707963;

    float d = saturate(distToShore / waveDistance);

    float dPhase = pow(d, shoreEase);              // ★ 改这一行
    float phase  = frac((time * waveSpeed + dPhase) / waveCycle);

    float p      = saturate(phase / foamWidth);
    float active = step(phase, foamWidth);

    float rise  = pow(sin(p * halfPi), foamSharp);
    float decay = pow(1.0 - p, 1.5);

    float foam = rise * decay * active;
    foam *= pow(1.0 - d, 0.3);
    return saturate(foam);
}
