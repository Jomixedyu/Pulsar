#pragma once
#include "Pulsar/Assets/Texture2D.h"
#include "ShaderConfig.h"
#include <variant>

namespace pulsar
{

    struct ShaderPropertyValue
    {
    public:
        ShaderPropertyType Type {};

        ShaderPropertyType GetType() const { return Type; }

        void SetValue(int value)
        {
            Value = value;
            Type = ShaderPropertyType::Int;
        }

        void SetValue(float value)
        {
            Value = value;
            Type = ShaderPropertyType::Float;;
        }

        void SetValue(Vector4f value)
        {
            Value = value;
            Type = ShaderPropertyType::Float4;
        }
        void SetValue(Color4f value)
        {
            Value = value;
            Type = ShaderPropertyType::Color;
        }
        void SetValue(const RCPtr<Texture>& value)
        {
            Value = value;
            Type = ShaderPropertyType::Texture2D;
        }

    private:
        std::variant<int, float, Vector4f, Color4f, RCPtr<Texture>> Value;

    public:
        int AsInt() const
        {
            return std::get<int>(Value);
        }
        float AsFloat() const
        {
            return std::get<float>(Value);
        }
        Vector4f AsFloat4() const
        {
            return std::get<Vector4f>(Value);
        }
        Color4f AsColor() const
        {
            return std::get<Color4f>(Value);
        }
        RCPtr<Texture> AsTexture2D() const
        {
            return std::get<RCPtr<Texture>>(Value);
        }
        int GetDataSize() const
        {
            switch (Type)
            {
            case ShaderPropertyType::Int:
                return sizeof(int);
            case ShaderPropertyType::Float:
                return sizeof(float);
            case ShaderPropertyType::Float4:
                return sizeof(Vector4f);
            case ShaderPropertyType::Color:
                return sizeof(Color4f);
            default:
                break;
            }
            return 0;
        }
    };




}