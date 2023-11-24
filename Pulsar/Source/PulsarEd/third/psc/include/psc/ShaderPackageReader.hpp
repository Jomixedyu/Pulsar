#pragma once
#include "Classes.hpp"
#include "BinaryFileHeader.h"
#include <unordered_map>
#include <iostream>
#include <vector>
#include <type_traits>

namespace psc
{

    inline auto ReadShaderPackage(std::istream& stream, std::vector<ApiPlatformType> filter)
    {
        auto beginPos = stream.tellg();

        binf::BinaryFileHeader header;
        stream.read(reinterpret_cast<char*>(&header), sizeof(header));
        stream.seekg(header.ResourceTableOffset - (stream.tellg() - beginPos), std::ios::cur);

        std::vector<binf::BinaryResourceInfo> infos;
        infos.resize(header.ResourceCount);
        stream.read(reinterpret_cast<char*>(infos.data()), sizeof(binf::BinaryResourceInfo) * header.ResourceCount);
        stream.seekg(header.DataOffset - (stream.tellg() - beginPos), std::ios::cur);

        std::vector<TargetShader> list;

        auto dataBeginPos = stream.tellg();
        for (auto& info : infos)
        {
            auto partialType = (psc::FilePartialType)info.Type;
            auto platformType = (psc::ApiPlatformType)info.Platform;
           
            if (std::find(filter.begin(), filter.end(), platformType) == filter.end())
            {
                continue;
            }
            stream.seekg(info.Offset - (stream.tellg() - dataBeginPos), std::ios::cur);

            auto& item = list.emplace_back();
            item.Name = info.Name;
            item.Partial = partialType;
            item.Platform = platformType;
            item.Data.resize(info.Length);
            stream.read(item.Data.data(), info.Length);
        }

        return list;
    }

}