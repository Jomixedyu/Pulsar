#include "Compressions/TextureCompression.h"
#include "astcenc.h"

namespace pulsar
{

    std::vector<uint8_t> TextureCompression::ASTC(uint8_t* data, size_t length, bool isSrgb, bool isHDR, int quality)
    {
        astcenc_swizzle swizzle = {};
        swizzle.r = ASTCENC_SWZ_R;
        swizzle.g = ASTCENC_SWZ_G;
        swizzle.b = ASTCENC_SWZ_B;
        swizzle.a = ASTCENC_SWZ_A;
        float astcenc_profile = ASTCENC_PRE_FAST;

        // Configure the compressor run
        astcenc_config my_config{};
        astcenc_config_init(isHDR ? ASTCENC_PRF_HDR : ASTCENC_PRF_LDR,
            4,4,1, astcenc_profile,0, &my_config);

        // Power users can tweak <my_config> settings here ...

        // Allocate working state given config and thread_count
        astcenc_context* my_context;
        astcenc_context_alloc(&my_config, 1, &my_context);

        // Compress each image using these config settings
        // astcenc_compress_image(my_context, &my_input, my_output, 1);

        astcenc_compress_reset(my_context);

        // Clean up
        astcenc_context_free(my_context);

        return {};
    }
} // namespace pulsar