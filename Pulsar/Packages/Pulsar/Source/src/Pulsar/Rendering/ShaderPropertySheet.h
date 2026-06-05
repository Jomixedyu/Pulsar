#pragma once
#include <Pulsar/Rendering/ShaderPropertyValue.h>

#include <map>
#include <string>
#include <vector>

namespace pulsar
{
    class ShaderPropertySheet
    {
    public:
        void SetInt(const std::string& name, int value)
        {
            m_properties[name].SetValue(value);
        }

        void SetFloat(const std::string& name, float value)
        {
            m_properties[name].SetValue(value);
        }

        void SetFloat4(const std::string& name, Vector4f value)
        {
            m_properties[name].SetValue(value);
        }

        void SetTexture(const std::string& name, const RCPtr<Texture>& value)
        {
            m_properties[name].SetValue(value);
        }

        bool GetInt(const std::string& name, int& outValue) const
        {
            auto it = m_properties.find(name);
            if (it == m_properties.end()) return false;
            if (it->second.Type != ShaderPropertyType::Int) return false;
            outValue = it->second.AsInt();
            return true;
        }

        bool GetFloat(const std::string& name, float& outValue) const
        {
            auto it = m_properties.find(name);
            if (it == m_properties.end()) return false;
            if (it->second.Type != ShaderPropertyType::Float) return false;
            outValue = it->second.AsFloat();
            return true;
        }

        bool GetFloat4(const std::string& name, Vector4f& outValue) const
        {
            auto it = m_properties.find(name);
            if (it == m_properties.end()) return false;
            if (it->second.Type != ShaderPropertyType::Float4) return false;
            outValue = it->second.AsFloat4();
            return true;
        }

        bool GetTexture(const std::string& name, RCPtr<Texture>& outValue) const
        {
            auto it = m_properties.find(name);
            if (it == m_properties.end()) return false;
            if (it->second.Type != ShaderPropertyType::Texture2D) return false;
            outValue = it->second.AsTexture2D();
            return true;
        }

        bool HasProperty(const std::string& name) const
        {
            return m_properties.contains(name);
        }

        std::vector<std::string> GetPropertyNames() const
        {
            std::vector<std::string> names;
            names.reserve(m_properties.size());
            for (const auto& [name, _] : m_properties)
            {
                names.push_back(name);
            }
            return names;
        }

        const ShaderPropertyValue* FindProperty(const std::string& name) const
        {
            auto it = m_properties.find(name);
            if (it == m_properties.end()) return nullptr;
            return &it->second;
        }

        const std::map<std::string, ShaderPropertyValue>& GetAllProperties() const
        {
            return m_properties;
        }

    private:
        std::map<std::string, ShaderPropertyValue> m_properties;
    };
}
