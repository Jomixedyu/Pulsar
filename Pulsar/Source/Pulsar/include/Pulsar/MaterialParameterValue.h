#pragma once
#include <Pulsar/ObjectBase.h>
#include <Pulsar/Assets/Texture2D.h>
#include <variant>

namespace pulsar
{
    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar,
        ShaderPassRenderingType,
        OpaqueForward,
        OpaqueDeferred,
        Transparency,
        PostProcessing,
        );

    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar,
        ShaderParameterType,
        IntScalar,
        Scalar,
        Vector,
        Texture2D,
    )
}

CORELIB_DECL_BOXING(pulsar::ShaderPassRenderingType, pulsar::BoxingShaderPassRenderingType);
CORELIB_DECL_BOXING(pulsar::ShaderParameterType, pulsar::BoxingShaderParameterType);

namespace pulsar
{

    struct MaterialParameterValue
    {
    public:
        ShaderParameterType Type;

        void SetValue(int value)
        {
            Value = value;
            Type = ShaderParameterType::IntScalar;
        }

        void SetValue(float value)
        {
            Value = value;
            Type = ShaderParameterType::Scalar;;
        }

        void SetValue(Vector4f value)
        {
            Value = value;
            Type = ShaderParameterType::Vector;
        }

        void SetValue(const RCPtr<Texture2D>& value)
        {
            Value = value;
            Type = ShaderParameterType::Texture2D;
        }

    private:
        std::variant<int, float, Vector4f, RCPtr<Texture2D>> Value;

    public:
        int AsIntScalar() const
        {
            return std::get<int>(Value);
        }
        float AsScalar() const
        {
            return std::get<float>(Value);
        }
        Vector4f AsVector() const
        {
            return std::get<Vector4f>(Value);
        }
        RCPtr<Texture2D> AsTexture() const
        {
            return std::get<RCPtr<Texture2D>>(Value);
        }
        int GetDataSize() const
        {
            switch (Type)
            {
            case ShaderParameterType::IntScalar:
            case ShaderParameterType::Scalar:
                return sizeof(float);
            case ShaderParameterType::Vector:
                return sizeof(Vector4f);
            default:
                break;
            }
            return 0;
        }
    };


    struct MaterialParameterInfo
    {
        size_t Offset;
        MaterialParameterValue Value;
    };

}