#include "File.h"
#include "UString.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace jxcorlib
{
    namespace FileUtil
    {
        using namespace std;
        std::string ReadAllText(std::string_view path)
        {
            std::ifstream ifs;
            std::stringstream ss;
            ifs.open(path);
            if (!ifs.is_open()) {
                throw std::invalid_argument("Unable to open file");
            }
            ss << ifs.rdbuf() << std::endl;
            ifs.close();
            return ss.str();
        }
        void WriteAllText(std::string_view path, std::string_view content)
        {
            ofstream outfile(path.data(), ios::ate);
            outfile << content;
            outfile.close();
        }
    }


    namespace PathUtil
    {
        std::string GetFilenameWithoutExt(std::string_view path)
        {
            int last = -1;
            for (size_t i = 0; i < path.size(); i++)
            {
                if (path[i] == '\\' || path[i] == '/')
                {
                    last = (int)i;
                }
            }
            if (last < 0)
            {
                return std::string{ path };
            }
            return std::string{ path.substr(static_cast<size_t>(last) + 1, path.find_last_of('.') - last - 1) };
        }
        std::string GetFilename(std::string_view path)
        {
            return StringUtil::StringCast(std::filesystem::path(path).filename().generic_u8string());
        }
        std::string GetFilenameExt(std::string_view path)
        {
            for (int i = (int)path.length() - 1; i >= 0; i--)
            {
                char c = path[i];
                if (c == '.')
                {
                    return string{ path.substr(i) };
                }
                else if (path[i] == '/' || path[i] == '\\')
                {
                    return string{};
                }
            }
            return string{};
        }
        std::string GetDirectory(std::string_view path)
        {
            for (int i = (int)path.length() - 1; i >= 0; i--)
            {
                char c = path[i];
                if (path[i] == '/' || path[i] == '\\')
                {
                    return string{ path.substr(0, i) };
                }
            }
            return string{};
        }
        std::string Combine(const std::string& p1, const std::string& p2)
        {
            if (p1.length() == 0)
            {
                return p2;
            }
            if (p2.length() == 0)
            {
                return p1;
            }

            std::string ret;
            ret.reserve(p1.length() + p2.length() + 1);
            ret.append(p1);

            char p1_lastc = p1[p1.length() - 1];
            if (p1_lastc != '/' && p1_lastc != '\\')
            {
                ret.append("/");
            }
            char p2_firstc = p2[0];
            if (p2_firstc == '/' || p2_firstc == '\\')
            {
                ret.append(p2, 1);
            }
            else
            {
                ret.append(p2);
            }

            return ret;
        }
        bool AInB(std::string_view a, std::string_view b)
        {
            if (b.empty() && !a.empty()) return true;
            if (a == b || a.length() <= b.length()) return false;

            for (size_t i = 0; i < b.length(); i++)
            {
                if (a[i] != b[i]) return false;
            }
            return a[b.length()] == '/' || a[b.length()] == '\\';
        }

        std::vector<std::string> Dir(std::string_view path, const std::vector<std::string>& target)
        {
            std::vector<std::string> ret;
            string pathstart = path.length() == 0 ? "" : string{ path } + '/';
            for (auto& item : target)
            {
                if (item.length() > path.length() && item.starts_with(pathstart))
                {
                    if (AInB(item, path) && std::find(ret.begin(), ret.end(), item) == ret.end())
                    {
                        ret.push_back(item);
                    }
                }
            }

            return ret;
        }
    }
}

