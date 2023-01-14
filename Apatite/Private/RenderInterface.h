#pragma once
#include <cstdint>
#include <string_view>

namespace apatite
{
    namespace detail::RenderInterface
    {
        void Clear(float r, float g, float b, float a);
        void EnableDepthTest();
        void DisableDepthTest();

        void GetDefaultBufferViewport(int32_t* width, int32_t* height);
        void GetViewport(int32_t* width, int32_t* height);
        void SetViewport(int32_t x, int32_t y, int32_t width, int32_t height);

        void LoadTexture2D(uint32_t channel_count, int32_t width, int32_t height, uint8_t* data, uint32_t* out_id);
        void UnloadTexture2D(uint32_t id);
        void UnloadTexture2Ds(uint32_t id[], int32_t length);


        double GetTime();
        float GetDeltaTime();
        uint32_t GetFrameCount();
        void Render();

        bool CompileVertexShader(std::string_view name, std::string_view code, uint32_t* out_id);
        bool CompileFragmentShader(std::string_view name, std::string_view code, uint32_t* out_id);

        void DeleteShader(uint32_t id);
        std::string GetShaderCompilerError(uint32_t id);
    }

}