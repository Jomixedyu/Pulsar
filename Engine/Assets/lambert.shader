
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
struct VertexAssemble
{
    vec3 Position;
    vec3 Normal;
    vec3 Tangent;
    vec3 BitTangent;
    vec4 VertColor;
    vec2 TexCoords0;
    vec2 TexCoords1;
    vec2 TexCoords2;
    vec2 TexCoords3;
};
struct VshAssemble
{
    vec3 WorldPosition;
    vec3 Normal;
    vec4 VertColor;
    vec2 TexCoords0;
    vec2 TexCoords1;
    vec2 TexCoords2;
    vec2 TexCoords3;
};

layout(location = 0) in VertexAssemble InVertex;

out VshAssemble OutVsh;


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
}
CB_Matrix;

layout() uniform CB_Params
{
    mat4 ModelMatrix;
}
CB_Params;

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

struct VshAssemble
{
    vec3 WorldPosition;
    vec3 Normal;
    vec4 VertColor;
    vec2 TexCoords0;
    vec2 TexCoords1;
    vec2 TexCoords2;
    vec2 TexCoords3;
};

out vec4 OutFragColor;
in VshAssemble InVsh;

uniform vec3 camPos;

void main()
{

    OutFragColor = vec4(color, 1.0);
}