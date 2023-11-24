#include "psc/ShaderCompiler.h"
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <iostream>
#include "cxxopts.hpp"
#include "stlext.hpp"
#include "GlslangCompilerImpl.h"
#include "DxcCompilerImpl.h"

using namespace psc;
using namespace std;

static ApiPlatformType _GetPlatformType(std::string name)
{
    if (name == "vulkan") return ApiPlatformType::Vulkan;
    if (name == "direct3d") return ApiPlatformType::Direct3D;
    if (name == "opengl") return ApiPlatformType::OpenGL;
    if (name == "opengles") return ApiPlatformType::OpenGLES;
    if (name == "metal") return ApiPlatformType::Metal;
    return {};
}


int main(int argc, char* argv[])
{
    using namespace std;

    cxxopts::Options opts(argv[0], "shader compiler help");
    opts.positional_help("[optional args]").show_positional_help();

    bool isDebug = false;
    std::vector<string> inPlatforms;
    inPlatforms.push_back("vulkan");

    opts
        .add_options()
        ("debugmode", "binary with debug info", cxxopts::value<bool>())
        ("h,help", "help")
        ("i,include", "include folder", cxxopts::value<std::vector<std::filesystem::path>>(), "FOLDERS;...")
        ("s,source", "shader json source", cxxopts::value<string>(), "PATH")
        ("c,compile", "compiling output filename", cxxopts::value<string>(), "PATH")
        ("d,defines", "predefine macros", cxxopts::value<std::vector<string>>(), "KEY=VALUE")
        ("p,platform", "target platforms. vulkan,opengl,d3d",
            cxxopts::value<std::vector<string>>(inPlatforms)->default_value("vulkan"), "vulkan,opengl,d3d")
        ;

    try
    {
        auto result = opts.parse(argc, argv);

        if (result.count("help") || result.arguments().size() == 0)
        {
            std::cout << opts.help({ "" }) << std::endl;
            return true;
        }

        CompileInfo compileInfo;
        if (result.count("debugmode"))
        {
            compileInfo.Debug = result["debugmode"].as<bool>();
        }
        cout << "is debug mode: " << compileInfo.Debug << endl;

        if (result.count("include"))
        {
            compileInfo.IncludePaths = result["include"].as<std::vector<std::filesystem::path>>();
        }
        if (result.count("defines"))
        {
            compileInfo.PreDefines = result["defines"].as<std::vector<std::string>>();
        }

        std::vector<ApiPlatformType> targetPlatforms;
        for (auto& item : inPlatforms)
        {
            auto platform = _GetPlatformType(item);
            if (platform == ApiPlatformType::None)
            {
                cerr << "ERR: platform not found. " << item;
                return 1;
            }
            if (!stlext::contains(targetPlatforms, platform))
            {
                targetPlatforms.push_back(platform);
            }
        }




        if (result.count("source"))
        {
            auto sourcePath = result["source"].as<string>();
            string compileFilename;

            if (result.count("compile"))
            {
                compileFilename = result["compile"].as<string>();
            }
            else
            {
                auto t = std::filesystem::path(sourcePath);
                t.replace_extension().replace_extension();
                compileFilename = t.string() + ".shader";
            }

            psc::GlslangCompilerImpl compiler;

            try
            {
                std::ofstream ofs{ compileFilename, ios::ate | ios::binary | ios::out };
                compiler.CompileShader(sourcePath, compileInfo, targetPlatforms, ofs);
                ofs.close();
                cout << "compile success: " << compileFilename << endl;
            }
            catch (const std::exception& e)
            {
                cerr << e.what() << endl;
            }
        }

        std::cout << "Unmatched options: ";
        for (const auto& option : result.unmatched())
        {
            std::cout << "'" << option << "' ";
        }
        std::cout << std::endl;
    }
    catch (const cxxopts::exceptions::exception& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        return false;
    }

    return 0;
}
