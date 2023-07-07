#include <Pulsar/Assets/CubeMap.h>
#include <ThirdParty/glad/glad.h>
#include <Pulsar/Private/ResourceInterface.h>
#include <CoreLib/File.h>
#include <CoreLib.Serialization/JsonSerializer.h>

namespace pulsar
{



    //AssetSource_sp CubeMapAssetSourceImporter::OnImport(const AssetSourceImporterContext& ctx, Type* type)
    //{
    //    auto ser = Serialization::JsonSerializer::Deserialize<CubeMapAssetImportObject>(FileUtil::ReadAllText(ctx.get_assetpath()));
    //    CubeMapAsset* cubemap = new CubeMapAsset;
    //    cubemap->set_name(ctx.get_filename_noext());
    //    string* str[6] = {
    //        &ser->Right,
    //        &ser->Left,
    //        &ser->Top,
    //        &ser->Bottom,
    //        &ser->Front,
    //        &ser->Back,
    //    };
    //    const string& dir = ctx.get_directory();
    //    glGenTextures(1, &cubemap->id);
    //    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->id);

    //    for (unsigned int i = 0; i < 6; i++)
    //    {
    //        int width, height, channel;
    //        const string& path = StringUtil::Concat(dir, "/", *str[i]);
    //        uint8_t* data = Private::ResourceInterface::LoadBitmap(path, &width, &height, &channel);
    //        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
    //            0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
    //        );
    //        Private::ResourceInterface::FreeBitmap(data);
    //    }
    //    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    //    auto as = mksptr(new AssetSourcePackage);
    //    as->set_name(cubemap->get_name());
    //    as->AddAsset(mksptr(cubemap));

    //    return as;
    //}
}