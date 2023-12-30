#ifndef _POST_PROCESS_INC
#define _POST_PROCESS_INC

struct PPVSOutput
{
	float4 Position  : SV_POSITION;
	float2 TexCoord0 : TEXCOORD0;
};

#endif //_POST_PROCESS_INC