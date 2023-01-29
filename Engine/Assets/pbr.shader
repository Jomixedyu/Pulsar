
#pragma config StandardPBR
{
    "InAlbedo" : "white",
    "InRoughness" : "white",
    "InMetallic" : "white",
    "InNormal" : "white",
}

#pragma pass
{
    "CullBack": true,
}

#pragma vert

#version 330 core
layout(location = 0) in vec3 InPosition;
layout(location = 1) in vec3 InNormal;
layout(location = 2) in vec3 InTangent;
layout(location = 3) in vec3 InBitTangent;
layout(location = 4) in vec4 InVertColor;
layout(location = 5) in vec2 InTexCoords0;
layout(location = 6) in vec2 InTexCoords1;
layout(location = 7) in vec2 InTexCoords2;
layout(location = 8) in vec2 InTexCoords3;

out vec3 OutWorldPosition;
out vec3 OutNormal;
out vec4 OutVertColor;
out vec2 OutTexCoords0;
out vec2 OutTexCoords1;
out vec2 OutTexCoords2;
out vec2 OutTexCoords3;

struct LightingStruct
{
    int Type;
    vec3 Position;
    vec3 Direction;
    vec3 Color;
};

layout() uniform CB_Lighting
{
    LightingStruct Lights[32];
}
CB_Lighting;

layout() uniform CB_Matrix
{
    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
    mat4 ModelMatrix;
}
CB_Matrix;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

void main()
{

    OutWorldPosition = vec3(uModelMatrix * vec4(aPos, 1.0));
    OutNormal = mat3(uModelMatrix) * InNormal;
    OutTexCoords0 = InTexCoords0;
    OutTexCoords1 = InTexCoords1;
    OutTexCoords2 = InTexCoords2;
    OutTexCoords3 = InTexCoords3;
    OutVertColor = InVertColor;

    gl_Position = uProjectionMatrix * uViewMatrix * vec4(OutWorldPosition, 1.0);
}

#pragma frag

#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform sampler2D AlbedoMap;
uniform sampler2D NormalMap;
uniform sampler2D MetallicMap;
uniform sampler2D RoughnessMap;
uniform sampler2D AoMap;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 camPos;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(NormalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(WorldPos);
    vec3 Q2 = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{
    vec3 albedo = pow(texture(AlbedoMap, TexCoords).rgb, vec3(2.2));

    vec3 normal = getNormalFromMap();
    float metallic = texture(MetallicMap, TexCoords).r;
    float roughness = texture(RoughnessMap, TexCoords).r;
    float ao = texture(AoMap, TexCoords).r;

    vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < 4; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 nominator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular = nominator / denominator;

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}