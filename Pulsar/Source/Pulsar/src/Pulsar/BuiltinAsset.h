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
        static inline const char* Shapes_Sphere = "Engine/Shapes/Sphere";
        static inline const char* Shapes_Cube = "Engine/Shapes/Cube";
        static inline const char* Shapes_Plane = "Engine/Shapes/Plane";
        static inline const char* Shader_Missing = "Engine/Shaders/Error";

        static inline const char* Material_Lambert = "Engine/Materials/Lambert";

        static inline const char* Texture_White = "Engine/Texture/T_White";
        static inline const char* Texture_Black = "Engine/Texture/T_Black";
        static inline const char* Texture_Gray = "Engine/Texture/T_Gray";
        static inline const char* Texture_DefaultNormal = "Engine/Texture/T_DefaultNormal";

        static RCPtr<Texture2D> GetTextureBlack();


        static void ClearAssets();

        // 将 shader 配置中的纹理默认值标识符解析为资产路径
        // 例如 "T2D_WHITE" -> "Engine/Texture/T_White"
        static const char* ResolveTextureDefault(const std::string& defaultValue);
    };
}