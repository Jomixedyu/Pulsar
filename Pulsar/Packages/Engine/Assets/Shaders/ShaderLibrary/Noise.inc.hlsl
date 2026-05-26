
float3 GerstnerOffset(float3 worldPosition, float2 waveDirection, float amplitude, float wavelength, float steepness, float speed, float time)
{
    const float TwoPi = 6.28318530718f;
    
    float2 direction = normalize(waveDirection);
    float waveNumber = TwoPi / wavelength;
    float phase = waveNumber * dot(direction, worldPosition.xy) - speed * time;
    
    float s, c;
    sincos(phase, s, c);
    
    float3 offset;
    offset.xy = (steepness / waveNumber) * direction * c;
    offset.z = amplitude * s;
    return offset;
}

/* Default:
Amplitude	20	10	5	2
Wavelength	300	150	80	40
Steepness	0.8	0.6	0.4	0.3
Speed	1.2	1.5	2.0	2.5
DirectionX	1.0	0.5	-0.8	-0.3
DirectionY	0.3	1.0	0.2	-0.9
*/
float3 GerstnerOffset4(float3 worldPosition, float2 waveDirection[4], float4 amplitude, float4 wavelength, float4 steepness, float4 speed, float time)
{
    const float TwoPi = 6.28318530718f;
    
    float2 dir0 = normalize(waveDirection[0]);
    float2 dir1 = normalize(waveDirection[1]);
    float2 dir2 = normalize(waveDirection[2]);
    float2 dir3 = normalize(waveDirection[3]);
    
    float4 waveNumber = TwoPi / wavelength;
    
    float4 phase = waveNumber * float4(
        dot(dir0, worldPosition.xy),
        dot(dir1, worldPosition.xy),
        dot(dir2, worldPosition.xy),
        dot(dir3, worldPosition.xy)
    ) - speed * time;
    
    float4 s, c;
    sincos(phase, s, c);
    
    float4 qOverK = steepness / waveNumber;
    
    float3 offset;
    offset.x = dot(qOverK * float4(dir0.x, dir1.x, dir2.x, dir3.x), c);
    offset.y = dot(qOverK * float4(dir0.y, dir1.y, dir2.y, dir3.y), c);
    offset.z = dot(amplitude, s);
    
    return offset;
}
