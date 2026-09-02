#pragma once
#include <string>
#include <string_view>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

namespace
{
    namespace IOHelper
    {
        using namespace std;
        namespace fs = std::filesystem;

        string ReadAllText(fs::path path)
        {
            ifstream fin;
            fin.open(path, std::ios::ate | std::ios::binary);
            if (!fin.is_open())
            {
                cout << "Unable to open file！" << endl;
                return {};
            }
            size_t fileSize = (size_t)fin.tellg();
            string buffer(fileSize, ' ');

            fin.seekg(0);
            fin.read(buffer.data(), fileSize);
            fin.close();
            return buffer;
        }

        void WriteAllBytes(fs::path path, const char* data, size_t size)
        {
            fs::path dir(path);
            dir.remove_filename();

            if (!fs::exists(dir))
            {
                fs::create_directory(dir);
            }

            ofstream fs(path, ios::out | ios::binary);

            fs.write(data, size);
            fs.close();
        }

        template<typename T> 
        inline void WriteAllBytes(fs::path path, const std::vector<T>& data)
        {
            WriteAllBytes(path, reinterpret_cast<const char*>(data.data()), data.size() * sizeof(std::vector<T>::value_type));
        }

    }
}
