#include <Apatite/Private/ResourceInterface.h>

#define STB_IMAGE_IMPLEMENTATION
#include <ThirdParty/stb_image.h>
#include <string>

namespace apatite::detail
{
    using namespace std;
    unsigned char* ResourceInterface::LoadBitmap(std::string_view name, int* out_width, int* out_height, int* out_channel)
    {
        //stbi_set_flip_vertically_on_load(true);
        return stbi_load(name.data(), out_width, out_height, out_channel, 0);
    }

    void ResourceInterface::FreeBitmap(unsigned char* data)
    {
        stbi_image_free(data);
    }

}

