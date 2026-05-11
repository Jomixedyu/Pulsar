
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

float3 GerstnerOffset4(float3 worldPosition, float2 waveDirection[4], float4 amplitude, float4 wavelength, float4 steepness, float4 speed, float time)
{
    float3 displacedPosition = worldPosition;

    displacedPosition += GerstnerOffset(worldPosition, waveDirection[0], amplitude.x, wavelength.x, steepness.x, speed.x, time);
    displacedPosition += GerstnerOffset(worldPosition, waveDirection[1], amplitude.y, wavelength.y, steepness.y, speed.y, time);
    displacedPosition += GerstnerOffset(worldPosition, waveDirection[2], amplitude.z, wavelength.z, steepness.z, speed.z, time);
    displacedPosition += GerstnerOffset(worldPosition, waveDirection[3], amplitude.w, wavelength.w, steepness.w, speed.w, time);

    return displacedPosition;
}
