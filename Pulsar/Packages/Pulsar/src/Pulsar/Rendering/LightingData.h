#pragma once
#include <Pulsar/EngineMath.h>
#include <queue>

namespace pulsar
{
    struct LightShaderParameter
    {
        Vector4f WorldPosition;
        Vector4f DirectionAndFalloffExponent;
        Vector4f Color;
        Vector2f SpotAngles;
        Vector2f SourceAndSoftSourceRadius;
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
        const LightShaderParameter& GetLightParameter(int index) const { return m_pendingBuffer[index]; }
    private:
        size_t m_bufferLength = 32;
        jxcorlib::array_list<LightShaderParameter>  m_pendingBuffer;
        jxcorlib::array_list<LightShaderParameter*> m_lightShaderParameters;
        std::unordered_map<LightShaderParameter*, int> m_ptr2index;
        std::queue<int> m_dirtyList;
    };

}