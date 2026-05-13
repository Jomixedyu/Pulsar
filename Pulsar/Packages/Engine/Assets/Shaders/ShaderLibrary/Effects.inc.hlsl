
float TravelingWave(float distToShore, float time,
                        float waveDistance, float waveSpeed,
                        float waveCycle,    float foamWidth, float foamSharp)
{
    const float halfPi = 1.5707963;
    float d = saturate(distToShore / waveDistance);
    float phase = frac((time * waveSpeed + d) / waveCycle);

    float p      = saturate(phase / foamWidth);
    float active = step(phase, foamWidth);

    float rise  = pow(sin(p * halfPi), foamSharp);
    float decay = pow(1.0 - p, 1.5);

    float foam = rise * decay * active;
    foam *= pow(1.0 - d, 0.3); // 靠岸更亮

    return saturate(foam);
}
