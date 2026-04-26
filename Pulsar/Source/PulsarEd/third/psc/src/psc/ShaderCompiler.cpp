
#include "psc/ShaderCompiler.h"
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <unordered_map>
#include <string>
#include "cxxopts.hpp"
#include "stlext.hpp"
#include "GlslangCompilerImpl.h"

// Include iomapper.h for TVarEntryInfo full definition and TIoMapResolver.
// TVarEntryInfo was moved from iomapper.cpp to iomapper.h so external
// resolvers like HlslBindingResolver can access its members.
#include "../glslang/glslang/MachineIndependent/iomapper.h"

namespace psc
{
    // =========================================================================
    // HlslBindingResolver
    //
    // Naming rules for set=0 (space0 = user material params):
    //   cbuffer                → binding 0
    //   Texture2D  _X          → binding 1, 2, 3 … (declaration order)
    //   SamplerState Sampler__X → same binding as _X  (→ CombinedImageSampler)
    //   SamplerState other     → own sequential binding
    //
    // Resources in set != 0 (engine buffers, explicit spaceN) are left alone.
    // =========================================================================
    class HlslBindingResolver : public glslang::TIoMapResolver
    {
    public:
        // Returns the ORIGINALLY DECLARED set (spaceN) from the symbol qualifier.
        // This must be used instead of ent.newSet in resolveBinding/resolveSet,
        // because clearNewAssignments() resets ent.newSet to -1 before those calls.
        static int GetDeclaredSet(const glslang::TVarEntryInfo& ent)
        {
            const glslang::TQualifier& q = ent.symbol->getQualifier();
            return q.hasSet() ? q.layoutSet : 0;
        }

        // ---- Phase 1: notification (collect before resolving) ---------------

        void notifyBinding(EShLanguage /*stage*/, glslang::TVarEntryInfo& ent) override
        {
            // Read the ORIGINAL declared set from the qualifier (not ent.newSet which
            // gets cleared by clearNewAssignments() before resolveBinding is called).
            // Resources with an explicit spaceN > 0 are engine buffers — skip them.
            if (GetDeclaredSet(ent) > 0) return;

            const std::string name = ent.symbol->getName().c_str();
            const glslang::TType& type = ent.symbol->getType();

            if (type.getBasicType() == glslang::EbtSampler)
            {
                const glslang::TSampler& sampler = type.getSampler();
                // Collect textures (Texture2D), not pure samplers
                if (sampler.isTexture() || sampler.isCombined())
                {
                    if (m_nameToBinding.find(name) == m_nameToBinding.end())
                        m_pendingTextures.push_back(name);
                }
            }
        }

        void endNotifications(EShLanguage /*stage*/) override
        {
            // Assign sequential bindings to collected textures (1-based; 0 = cbuffer)
            for (const auto& texName : m_pendingTextures)
            {
                if (m_nameToBinding.find(texName) == m_nameToBinding.end())
                    m_nameToBinding[texName] = m_nextBinding++;
            }
        }

        // ---- Phase 2: resolve -----------------------------------------------

        int resolveBinding(EShLanguage /*stage*/, glslang::TVarEntryInfo& ent) override
        {
            // NOTE: glslang ignores the return value — we must write ent.newBinding directly.
            // Use GetDeclaredSet() — ent.newSet has been reset to -1 by clearNewAssignments().
            if (GetDeclaredSet(ent) > 0) return -1; // engine buffer (spaceN > 0), leave alone

            const std::string name = ent.symbol->getName().c_str();
            const glslang::TType& type = ent.symbol->getType();

            // cbuffer / uniform block → binding 0
            if (type.getBasicType() == glslang::EbtBlock &&
                type.getQualifier().storage == glslang::EvqUniform)
            {
                ent.newBinding = 0;
                return 0;
            }

            if (type.getBasicType() == glslang::EbtSampler)
            {
                const glslang::TSampler& sampler = type.getSampler();

                if (sampler.isTexture() || sampler.isCombined())
                {
                    auto it = m_nameToBinding.find(name);
                    int b = (it != m_nameToBinding.end()) ? it->second : [&]{
                        int nb = m_nextBinding++;
                        m_nameToBinding[name] = nb;
                        return nb;
                    }();
                    ent.newBinding = b;
                    return b;
                }

                if (sampler.isPureSampler())
                {
                    // "Sampler__X" → same binding as "_X"
                    if (name.rfind("Sampler_", 0) == 0)
                    {
                        std::string texName = name.substr(8); // strip "Sampler_"
                        auto it = m_nameToBinding.find(texName);
                        if (it != m_nameToBinding.end())
                        {
                            ent.newBinding = it->second;
                            return it->second;
                        }
                    }
                    // unpaired sampler: own slot
                    auto it = m_nameToBinding.find(name);
                    int b = (it != m_nameToBinding.end()) ? it->second : [&]{
                        int nb = m_nextBinding++;
                        m_nameToBinding[name] = nb;
                        return nb;
                    }();
                    ent.newBinding = b;
                    return b;
                }
            }
            return -1;
        }

        int resolveSet(EShLanguage /*stage*/, glslang::TVarEntryInfo& ent) override
        {
            // NOTE: glslang ignores the return value — we must write ent.newSet directly.
            // Use GetDeclaredSet() — ent.newSet has been reset to -1 by clearNewAssignments().
            int declaredSet = GetDeclaredSet(ent);
            if (declaredSet > 0)
            {
                // Engine buffer with explicit spaceN: preserve original set.
                ent.newSet = declaredSet;
                return declaredSet;
            }
            // space0 / unspecified → assign to set=0 (user material set)
            ent.newSet = 0;
            return 0;
        }

        // ---- Required pure-virtual stubs ------------------------------------
        bool validateBinding(EShLanguage, glslang::TVarEntryInfo&) override { return true; }
        bool validateInOut(EShLanguage, glslang::TVarEntryInfo&) override   { return true; }
        int  resolveUniformLocation(EShLanguage, glslang::TVarEntryInfo&) override { return -1; }
        int  resolveInOutLocation(EShLanguage, glslang::TVarEntryInfo&) override   { return -1; }
        int  resolveInOutComponent(EShLanguage, glslang::TVarEntryInfo&) override  { return -1; }
        int  resolveInOutIndex(EShLanguage, glslang::TVarEntryInfo&) override      { return -1; }
        void notifyInOut(EShLanguage, glslang::TVarEntryInfo&) override {}
        void beginNotifications(EShLanguage) override {}
        void beginResolve(EShLanguage) override {}
        void endResolve(EShLanguage) override {}
        void beginCollect(EShLanguage) override {}
        void endCollect(EShLanguage) override {}
        void reserverStorageSlot(glslang::TVarEntryInfo&, TInfoSink&) override {}
        void reserverResourceSlot(glslang::TVarEntryInfo&, TInfoSink&) override {}
        void addStage(EShLanguage, glslang::TIntermediate&) override {}

    private:
        std::vector<std::string>          m_pendingTextures;  // ordered texture names
        std::unordered_map<std::string,int> m_nameToBinding;  // name → final binding
        int m_nextBinding = 1;                                 // 0 is reserved for cbuffer
    };

    // =========================================================================
    // GlslangCompilerImpl::CompilePass
    // =========================================================================
    PassCompileResult GlslangCompilerImpl::CompilePass(
        const PassCompileInfo& info,
        const char* extraDebugPath)
    {
        glslang::EShClient client = GetShClient(info.platform);
        constexpr int ClientInputSemanticsVersion = 100;
        constexpr glslang::EShTargetClientVersion   ClientVersion = glslang::EShTargetVulkan_1_3;
        constexpr glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_3;

        EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules |
                                             EShMsgAbsolutePath | EShMsgDisplayErrorColumn);
        TBuiltInResource resources = GetDefaultTBuiltInResource();
        constexpr int DefaultVersion = 100;

        std::string preamble;
        for (auto& def : info.PreDefines)
            preamble += "#define " + def + " 1 \n";

        struct StageData {
            std::string preprocessed;
            std::unique_ptr<glslang::TShader> shader;
        };

        auto PrepareStage = [&](EShLanguage langStage, const std::string& entry) -> StageData
        {
            StageData sd;
            sd.shader = std::make_unique<glslang::TShader>(langStage);
            sd.shader->setStrings(&info.code, 1);
            sd.shader->setEnvInput(glslang::EShSourceHlsl, langStage, client,
                                   ClientInputSemanticsVersion);
            sd.shader->setEnvClient(client, ClientVersion);
            sd.shader->setEnvTarget(glslang::EShTargetSpv, TargetVersion);
            sd.shader->setEntryPoint(entry.c_str());
            sd.shader->setPreamble(preamble.c_str());
            // Do NOT call setAutoMapBindings here — we want our resolver to control bindings.
            sd.shader->setAutoMapLocations(true);

            DirStackFileIncluder includer;
            for (auto& p : info.IncludePaths)
                includer.pushExternalLocalDirectory(p.string());

            if (!sd.shader->preprocess(&resources, DefaultVersion, ENoProfile,
                                       false, false, messages,
                                       &sd.preprocessed, includer))
            {
                std::string err = "Preprocessing Failed: ";
                err += sd.shader->getInfoLog();
                err += sd.shader->getInfoDebugLog();
                if (extraDebugPath) err += std::string("; path: ") + extraDebugPath;
                throw std::runtime_error(err);
            }

            const char* src = sd.preprocessed.c_str();
            sd.shader->setStrings(&src, 1);

            if (!sd.shader->parse(&resources, 110, false, messages))
            {
                std::string err = "Shader parsing failed: ";
                err += sd.shader->getInfoLog();
                err += sd.shader->getInfoDebugLog();
                if (extraDebugPath) err += std::string("; path: ") + extraDebugPath;
                throw std::runtime_error(err);
            }
            return sd;
        };

        bool hasVS = !info.vsEntry.empty();
        bool hasPS = !info.psEntry.empty();

        StageData vsData, psData;
        if (hasVS) vsData = PrepareStage(EShLangVertex,   info.vsEntry);
        if (hasPS) psData = PrepareStage(EShLangFragment, info.psEntry);

        glslang::TProgram Program;
        if (hasVS) Program.addShader(vsData.shader.get());
        if (hasPS) Program.addShader(psData.shader.get());

        if (!Program.link(messages))
        {
            std::string err = "Linking Failed: ";
            if (hasVS) err += vsData.shader->getInfoLog();
            if (hasPS) err += psData.shader->getInfoLog();
            throw std::runtime_error(err);
        }

        // Single unified mapIO call with our resolver — guarantees VS and PS
        // receive identical binding numbers for the same resource name.
        HlslBindingResolver resolver;
        if (!Program.mapIO(&resolver))
        {
            std::string err = "Mapping IO Failed (CompilePass): ";
            if (hasVS) err += vsData.shader->getInfoLog();
            if (hasPS) err += psData.shader->getInfoLog();
            throw std::runtime_error(err);
        }

        auto ToSpirv = [&](EShLanguage langStage) -> std::vector<char>
        {
            std::vector<unsigned int> spirv;
            spv::SpvBuildLogger logger;
            glslang::SpvOptions opts;
            if (info.Debug) {
                opts.generateDebugInfo = true;
                opts.disableOptimizer  = true;
                opts.emitNonSemanticShaderDebugInfo   = true;
                opts.emitNonSemanticShaderDebugSource = true;
            }
            glslang::GlslangToSpv(*Program.getIntermediate(langStage), spirv, &logger, &opts);
            std::cout << logger.getAllMessages();
            std::vector<char> data(spirv.size() * sizeof(unsigned int));
            memcpy(data.data(), spirv.data(), data.size());
            return data;
        };

        PassCompileResult result;
        if (hasVS) result.vsSpirv = ToSpirv(EShLangVertex);
        if (hasPS) result.psSpirv = ToSpirv(EShLangFragment);
        return result;
    }

    // =========================================================================

    std::shared_ptr<ShaderCompiler> CreateShaderCompiler(ApiPlatformType platform)
    {
        switch (platform)
        {
        case ApiPlatformType::Vulkan:
        case ApiPlatformType::OpenGL:
        case ApiPlatformType::OpenGLES:
            return std::make_shared<GlslangCompilerImpl>();
            break;
        case ApiPlatformType::Direct3D:
            //return std::make_shared<DxcCompilerImpl>();
        default:
            throw 0;
        }
        return {};
    }

}
