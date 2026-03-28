#ifndef _POST_PROCESS_INC
#define _POST_PROCESS_INC

#include "Blit.inc.hlsl"

Texture2D PP_InColor    	     : register(t0, space2);
SamplerState Sampler_PP_InColor  : register(s0, space2);

Texture2D PP_InDepth            : register(t1, space2);
SamplerState Sampler_PP_InDepth : register(s1, space2);





#endif //_POST_PROCESS_INC