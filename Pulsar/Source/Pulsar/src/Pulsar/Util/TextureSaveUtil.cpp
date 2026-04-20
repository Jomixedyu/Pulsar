#include "Util/TextureSaveUtil.h"

#include <Pulsar/Assets/Texture2D.h>
#include <Pulsar/Assets/RenderTexture.h>
#include <Pulsar/Application.h>
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Logger.h>
#include <gfx/GFXImage.h>
#include <gfx/GFXTexture.h>
#include <CoreLib/File.h>

#include <chrono>
#include <ctime>
#include <filesystem>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include "stb_image_write.h"

namespace pulsar
{
    struct PngWriteContext
    {
        std::vector<uint8_t> buffer;
    };

    static void PngWriteCallback(void* context, void* data, int size)
    {
        auto* ctx = static_cast<PngWriteContext*>(context);
        auto* bytes = static_cast<uint8_t*>(data);
        ctx->buffer.insert(ctx->buffer.end(), bytes, bytes + size);
    }

    bool TextureSaveUtil::SavePixelDataToPng(int32_t width, int32_t height, int32_t channelCount,
        const uint8_t* pixelData, const std::string& outputPath)
    {
        if (!pixelData || width <= 0 || height <= 0 || channelCount <= 0 || channelCount > 4)
        {
            Logger::Log("TextureSaveUtil::SavePixelDataToPng: invalid parameters", LogLevel::Error);
            return false;
        }

        // Encode PNG to memory first
        PngWriteContext ctx;
        int result = stbi_write_png_to_func(PngWriteCallback, &ctx,
            width, height, channelCount, pixelData, width * channelCount);

        if (!result || ctx.buffer.empty())
        {
            Logger::Log("TextureSaveUtil::SavePixelDataToPng: stbi_write_png_to_func failed", LogLevel::Error);
            return false;
        }

        // Write the encoded PNG data to file
        jxcorlib::FileUtil::WriteAllBytes(
            jxcorlib::StrToU8Path(outputPath),
            reinterpret_cast<char*>(ctx.buffer.data()),
            ctx.buffer.size());

        Logger::Log(std::format("TextureSaveUtil: Saved PNG to '{}' ({}x{}, {}ch, {} bytes)",
            outputPath, width, height, channelCount, ctx.buffer.size()));
        return true;
    }

    bool TextureSaveUtil::SaveTexture2DToPng(Texture2D* texture, const std::string& outputPath)
    {
        if (!texture)
        {
            Logger::Log("TextureSaveUtil::SaveTexture2DToPng: texture is null", LogLevel::Error);
            return false;
        }

        const int32_t width = texture->GetWidth();
        const int32_t height = texture->GetHeight();

        std::vector<uint8_t> rgbaData;

        if (texture->GetCompressedFormat() == TextureCompressionFormat::HDR_Compressed)
        {
            auto gfxTex = texture->GetGFXTexture();
            if (!gfxTex)
            {
                Logger::Log("TextureSaveUtil::SaveTexture2DToPng: HDR texture GPU resource not available", LogLevel::Error);
                return false;
            }
            int32_t outW, outH;
            rgbaData = ReadbackGFXTexture(gfxTex.get(), &outW, &outH);
        }
        else
        {
            auto gfxTex = texture->GetGFXTexture();
            if (gfxTex)
            {
                int32_t outW, outH;
                rgbaData = ReadbackGFXTexture(gfxTex.get(), &outW, &outH);
            }
        }

        if (rgbaData.empty())
        {
            Logger::Log("TextureSaveUtil::SaveTexture2DToPng: failed to get pixel data", LogLevel::Error);
            return false;
        }

        int32_t channelCount = static_cast<int32_t>(rgbaData.size()) / (width * height);
        if (channelCount < 1 || channelCount > 4)
        {
            channelCount = 4;
        }

        return SavePixelDataToPng(width, height, channelCount, rgbaData.data(), outputPath);
    }

    bool TextureSaveUtil::SaveRenderTextureToPng(RenderTexture* renderTexture, const std::string& outputPath)
    {
        if (!renderTexture)
        {
            Logger::Log("TextureSaveUtil::SaveRenderTextureToPng: renderTexture is null", LogLevel::Error);
            return false;
        }

        const int32_t width = renderTexture->GetWidth();
        const int32_t height = renderTexture->GetHeight();

        auto& renderTargets = renderTexture->GetRenderTargets();
        if (renderTargets.empty())
        {
            Logger::Log("TextureSaveUtil::SaveRenderTextureToPng: no render targets", LogLevel::Error);
            return false;
        }

        auto gfxTex = renderTargets[0];
        if (!gfxTex)
        {
            Logger::Log("TextureSaveUtil::SaveRenderTextureToPng: GFX texture is null", LogLevel::Error);
            return false;
        }

        int32_t outW, outH;
        auto rgbaData = ReadbackGFXTexture(gfxTex.get(), &outW, &outH);

        if (rgbaData.empty())
        {
            Logger::Log("TextureSaveUtil::SaveRenderTextureToPng: readback failed", LogLevel::Error);
            return false;
        }

        int32_t channelCount = static_cast<int32_t>(rgbaData.size()) / (width * height);
        if (channelCount < 1 || channelCount > 4)
        {
            channelCount = 4;
        }

        return SavePixelDataToPng(width, height, channelCount, rgbaData.data(), outputPath);
    }

    std::vector<uint8_t> TextureSaveUtil::ReadbackGFXTexture(gfx::GFXTexture* texture,
        int32_t* outWidth, int32_t* outHeight)
    {
        if (!texture)
        {
            return {};
        }

        auto gfxApp = Application::GetGfxApp();
        if (!gfxApp)
        {
            return {};
        }

        const int32_t width = texture->GetWidth();
        const int32_t height = texture->GetHeight();

        if (outWidth) *outWidth = width;
        if (outHeight) *outHeight = height;

        return gfxApp->ReadbackTexture(texture, width, height);
    }

    std::string TextureSaveUtil::CaptureCameraScreenshot(CameraComponent* camera)
    {
        if (!camera)
        {
            Logger::Log("TextureSaveUtil::CaptureCameraScreenshot: camera is null", LogLevel::Warning);
            return "";
        }

        auto rt = camera->GetRenderTexture();
        if (!rt)
        {
            Logger::Log("TextureSaveUtil::CaptureCameraScreenshot: no render texture", LogLevel::Warning);
            return "";
        }

        // Build output path: Saved/Screenshots/Screenshot_YYYYMMDD_HHmmss.png
        auto savedDir = std::filesystem::current_path() / "Saved" / "Screenshots";
        std::filesystem::create_directories(savedDir);

        auto now = std::chrono::system_clock::now();
        auto time_t_val = std::chrono::system_clock::to_time_t(now);
        std::tm* tm_ptr = std::localtime(&time_t_val);
        char timeStr[32];
        std::strftime(timeStr, sizeof(timeStr), "%Y%m%d_%H%M%S", tm_ptr);

        auto outPath = savedDir / (std::string("Screenshot_") + timeStr + ".png");
        std::string pathStr = outPath.string();

        bool success = SaveRenderTextureToPng(rt.GetPtr(), pathStr);
        if (success)
        {
            Logger::Log(std::format("Screenshot saved to '{}'", pathStr));
            return pathStr;
        }
        else
        {
            Logger::Log(std::format("Failed to save screenshot to '{}'", pathStr), LogLevel::Error);
            return "";
        }
    }

} // namespace pulsar
