#ifndef _POST_PROCESS_INC
#define _POST_PROCESS_INC
#include "Common.inc.hlsl"

Texture2D PP_InColor    : register(t0, space2);
Texture2D PP_InDepth    : register(t1, space2);

SamplerState DefaultSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct PPVSOutput
{
	float4 Position  : SV_POSITION;
	float2 TexCoord0 : TEXCOORD0;
};


#endif //_POST_PROCESS_INC