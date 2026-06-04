#pragma once
#include <string>
#include <Pulsar/AssetObject.h>

namespace pulsar
{
    class Shader;
    class Texture2D;

    class BuiltinAsset
    {
    public:
        static inline const char* Shapes_Sphere = "Pulsar/Shapes/Sphere";
        static inline const char* Shapes_Cube = "Pulsar/Shapes/Cube";
        static inline const char* Shapes_Plane = "Pulsar/Shapes/Plane";
        static inline const char* Shapes_XYPlane = "Pulsar/Shapes/XYPlane";
        static inline const char* Shader_Missing = "Pulsar/Shaders/Error";

        static inline const char* Material_Lambert = "Pulsar/Materials/Lambert";

        static inline const char* Texture_White = "Pulsar/Texture/T_White";
        static inline const char* Texture_Black = "Pulsar/Texture/T_Black";
        static inline const char* Texture_Gray = "Pulsar/Texture/T_Gray";
        static inline const char* Texture_DefaultNormal = "Pulsar/Texture/T_DefaultNormal";

        static RCPtr<Texture2D> GetTextureBlack();


        static void ClearAssets();

        // 将 shader 配置中的纹理默认值标识符解析为资产路径
        // 例如 "T2D_WHITE" -> "Pulsar/Texture/T_White"
        static const char* ResolveTextureDefault(const std::string& defaultValue);
    };
}
