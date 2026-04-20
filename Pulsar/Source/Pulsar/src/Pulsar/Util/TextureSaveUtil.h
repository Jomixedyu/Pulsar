#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace gfx
{
    class GFXTexture;
}

namespace pulsar
{
    class RenderTexture;
    class Texture2D;

    class TextureSaveUtil
    {
    public:
        /**
         * @brief Save a Texture2D asset to PNG file.
         * Decodes the origin compressed data to RGBA8 pixels and writes to PNG.
         * @param texture The Texture2D to save
         * @param outputPath The output file path (should end with .png)
         * @return true on success
         */
        static bool SaveTexture2DToPng(Texture2D* texture, const std::string& outputPath);

        /**
         * @brief Save a RenderTexture (GPU render target) to PNG file.
         * Reads back the GPU texture data and writes to PNG.
         * @param renderTexture The RenderTexture to save
         * @param outputPath The output file path (should end with .png)
         * @return true on success
         */
        static bool SaveRenderTextureToPng(RenderTexture* renderTexture, const std::string& outputPath);

        /**
         * @brief Save raw RGBA8 pixel data to PNG file.
         * @param width Image width
         * @param height Image height
         * @param channelCount Number of channels (1-4)
         * @param pixelData Raw pixel data (top-to-bottom, RGBA8)
         * @param outputPath The output file path
         * @return true on success
         */
        static bool SavePixelDataToPng(int32_t width, int32_t height, int32_t channelCount,
            const uint8_t* pixelData, const std::string& outputPath);

        /**
         * @brief Read back GPU texture data to CPU memory as RGBA8.
         * @param texture The GFX texture to read back
         * @param width Output: texture width
         * @param height Output: texture height
         * @return The RGBA8 pixel data, or empty on failure
         */
        static std::vector<uint8_t> ReadbackGFXTexture(gfx::GFXTexture* texture,
            int32_t* outWidth, int32_t* outHeight);
    };
} // namespace pulsar
