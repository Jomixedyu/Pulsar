#pragma once
#include "gfx/GFXDescriptorSet.h"

#include <Pulsar/EngineMath.h>
#include <gfx/GFXBuffer.h>
#include <queue>

namespace pulsar
{
    struct LightShaderParameter
    {
        Vector4f WorldPosition;
        Vector4f Direction;
        Vector4f Color;
        float SourceRadius;
        float SoftSourceRadius;
        float FalloffExponent;
        float _Padding0;
        Vector2f SpotAngles;
        Vector2f _Padding1;
    };

    class LightManager final
    {
    public:
        LightManager();
        ~LightManager();
        void AddLight(LightShaderParameter* lightShaderParameter);
        void RemoveLight(LightShaderParameter* lightShaderParameter);
        void MarkDirty(int id);
        int GetId(LightShaderParameter* light);
        void Update();
        int GetLightCount() const;
        const auto& GetDescriptorSet() const { return m_descriptorSet; }
        const auto& GetDescriptorSetLayout() const { return m_descriptorSetLayout; }
    private:
        gfx::GFXBuffer_sp m_buffer;
        size_t m_bufferLength = 32;
        jxcorlib::array_list<LightShaderParameter>  m_pendingBuffer;
        jxcorlib::array_list<LightShaderParameter*> m_lightShaderParameters;
        std::unordered_map<LightShaderParameter*, int> m_ptr2index;
        std::queue<int> m_dirtyList;
        std::queue<int> m_emptyIndexQueue;
        gfx::GFXDescriptorSet_sp m_descriptorSet;
        gfx::GFXDescriptorSetLayout_sp m_descriptorSetLayout;
    };

}