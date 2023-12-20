/*
* @Moudule     : File
* @Date        : 2020/07/14
* @Author      : JomiXedYu
* @Description : This file is part of jxcorlib : https://github.com/JomiXedYu/jxcorlib
* @StdRequired : c++20
*/

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <filesystem>

namespace jxcorlib
{
    namespace fs = std::filesystem;


    fs::path u8path(std::string_view u8str);

    namespace FileUtil
    {
        std::string ReadAllText(std::filesystem::path path);
        void WriteAllText(std::filesystem::path path, std::string_view content);
        std::vector<uint8_t> ReadAllBytes(std::filesystem::path path);
    }

    namespace PathUtil
    {
        std::string GetFilenameWithoutExt(std::string_view path);
        std::string GetFilename(std::string_view path);
        std::string GetFilenameExt(std::string_view path);
        std::string GetDirectory(std::string_view path);
        std::string Combine(const std::string& p1, const std::string& p2);
        //eg. a= Dir/Path/file b= Dir/Path   return true
        bool AInB(std::string_view a, std::string_view b);

        std::vector<std::string> Dir(std::string_view path, const std::vector<std::string>& target);
        std::string GetRoot(std::string_view path);
        void GenericSelf(std::string* path);
    };
}


