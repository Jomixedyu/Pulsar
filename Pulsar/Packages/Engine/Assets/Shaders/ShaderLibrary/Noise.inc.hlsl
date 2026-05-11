
float3 GerstnerOffset(float3 worldPosition, float2 waveDirection, float amplitude, float wavelength, float steepness, float speed, float time)
{
    float2 direction = normalize(waveDirection);
    float waveNumber = 6.28318530718 / wavelength;
    float phase = waveNumber * dot(direction, worldPosition.xy) - speed * time;
    
    float3 offset;
    offset.xy = (steepness / waveNumber) * direction * cos(phase);
    offset.z = amplitude * sin(phase);
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
    float3 displacedPosition = worldPosition;

    displacedPosition += GerstnerOffset(worldPosition, waveDirection[0], amplitude.x, wavelength.x, steepness.x, speed.x, time);
    displacedPosition += GerstnerOffset(worldPosition, waveDirection[1], amplitude.y, wavelength.y, steepness.y, speed.y, time);
    displacedPosition += GerstnerOffset(worldPosition, waveDirection[2], amplitude.z, wavelength.z, steepness.z, speed.z, time);
    displacedPosition += GerstnerOffset(worldPosition, waveDirection[3], amplitude.w, wavelength.w, steepness.w, speed.w, time);

    return displacedPosition;
}
