#ifndef _POST_PROCESS_INC
#define _POST_PROCESS_INC
#include "Common.inc.hlsl"

#define USER_DESCSET space3

Texture2D PP_InColor    : register(t0, space2);
Texture2D PP_InDepth    : register(t1, space2);

SamplerState ColorSampler : register(s0, space2);
SamplerState InDepthSampler : register(s1, space2);

struct PPVSOutput
{
	float4 Position  : SV_POSITION;
	float2 TexCoord0 : TEXCOORD0;
};


#endif //_POST_PROCESS_INC