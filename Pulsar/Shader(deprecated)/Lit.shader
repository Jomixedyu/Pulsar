   {
	"CullMode" : "Back"
}x	  #                     GLSL.std.450                                        D:/Codes/Pulsar/Pulsar/Shader//Common.inc.hlsl       �      s   �     #define PLATFORM_VULKAN

#ifdef PLATFORM_VULKAN
#define LAYOUT_LOCATION(index) [[vk::location(index)]]
#define VK_SAMPLER(index) [[vk::combinedImageSampler]][[vk::binding(index)]]
#else
#define LAYOUT_LOCATION(index)  
#endif

// Engine Input Semantic
#define EIS_POSITION  LAYOUT_LOCATION(0)
#define EIS_NORMAL    LAYOUT_LOCATION(1)
#define EIS_TANGENT   LAYOUT_LOCATION(2)
#define EIS_VERTCOLOR LAYOUT_LOCATION(3)
#define EIS_TEXCOORD0 LAYOUT_LOCATION(4)
#define EIS_TEXCOORD1 LAYOUT_LOCATION(5)
#define EIS_TEXCOORD2 LAYOUT_LOCATION(6)
#define EIS_TEXCOORD3 LAYOUT_LOCATION(7)

struct InVertexAssembly
{
    EIS_POSITION  float3 Position  : POSITION;
    EIS_NORMAL    float3 Normal    : NORMAL;
    EIS_TANGENT   float3 Tangent   : TANGENT;
    EIS_VERTCOLOR float4 Color     : COLOR0;
    EIS_TEXCOORD0 float2 TexCoord0 : TEXCOORD0;
    EIS_TEXCOORD1 float2 TexCoord1 : TEXCOORD1;
    EIS_TEXCOORD2 float2 TexCoord2 : TEXCOORD2;
    EIS_TEXCOORD3 float2 TexCoord3 : TEXCOORD3;
};

struct InPixelAssembly
{
    float4 Position : SV_POSITION;
    float3 WorldNormal : NORMAL0;
    float4 Color : COLOR0;
    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float2 TexCoord2 : TEXCOORD2;
    float2 TexCoord3 : TEXCOORD3;
};

struct OutPixelAssembly
{
    float4 Color : SV_TARGET;
};

struct CommonBuffer
{
    float4x4 ModelMatrix;
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
};
               CommonBuffer             ModelMatrix         ViewMatrix          ProjectionMatrix         MatrixBufferObject           MatrixBufferObject           J client vulkan100    J target-env spirv1.3 J target-env vulkan1.1    J hlsl-offsets    J client vulkan100    J target-env spirv1.3 J target-env vulkan1.1    J hlsl-offsets    H            H         #       H               H           H        #   @   H              H           H        #   �   H              H         #       G        G     "       G     !            !                   	           
   	           
   
   
                       ;           6               �     �  8  \  #                     GLSL.std.450                                               D:/Codes/Pulsar/Pulsar/Shader//Common.inc.hlsl       �      s   �     #define PLATFORM_VULKAN

#ifdef PLATFORM_VULKAN
#define LAYOUT_LOCATION(index) [[vk::location(index)]]
#define VK_SAMPLER(index) [[vk::combinedImageSampler]][[vk::binding(index)]]
#else
#define LAYOUT_LOCATION(index)  
#endif

// Engine Input Semantic
#define EIS_POSITION  LAYOUT_LOCATION(0)
#define EIS_NORMAL    LAYOUT_LOCATION(1)
#define EIS_TANGENT   LAYOUT_LOCATION(2)
#define EIS_VERTCOLOR LAYOUT_LOCATION(3)
#define EIS_TEXCOORD0 LAYOUT_LOCATION(4)
#define EIS_TEXCOORD1 LAYOUT_LOCATION(5)
#define EIS_TEXCOORD2 LAYOUT_LOCATION(6)
#define EIS_TEXCOORD3 LAYOUT_LOCATION(7)

struct InVertexAssembly
{
    EIS_POSITION  float3 Position  : POSITION;
    EIS_NORMAL    float3 Normal    : NORMAL;
    EIS_TANGENT   float3 Tangent   : TANGENT;
    EIS_VERTCOLOR float4 Color     : COLOR0;
    EIS_TEXCOORD0 float2 TexCoord0 : TEXCOORD0;
    EIS_TEXCOORD1 float2 TexCoord1 : TEXCOORD1;
    EIS_TEXCOORD2 float2 TexCoord2 : TEXCOORD2;
    EIS_TEXCOORD3 float2 TexCoord3 : TEXCOORD3;
};

struct InPixelAssembly
{
    float4 Position : SV_POSITION;
    float3 WorldNormal : NORMAL0;
    float4 Color : COLOR0;
    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float2 TexCoord2 : TEXCOORD2;
    float2 TexCoord3 : TEXCOORD3;
};

struct OutPixelAssembly
{
    float4 Color : SV_TARGET;
};

struct CommonBuffer
{
    float4x4 ModelMatrix;
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
};
               texture1         state   J client vulkan100    J target-env spirv1.3 J target-env vulkan1.1    J hlsl-offsets    J client vulkan100    J target-env spirv1.3 J target-env vulkan1.1    J hlsl-offsets    G     "       G     !      G     "       G     !           !                  	 	                               
       	   ;  
                            ;            6               �     �  8  