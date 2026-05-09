#include "Components/SkinnedMeshRendererComponent.h"
#include "Rendering/SimplePrimitiveUtils.h"

#include "AssetManager.h"
#include "Components/RendererComponent.h"
#include <Pulsar/Application.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Rendering/ShaderConfig.h>
#include <gfx/GFXBuffer.h>

namespace pulsar
{
    // -----------------------------------------------------------------------
    // SkinnedMeshRenderObject
    // -----------------------------------------------------------------------
    static gfx::GFXDescriptorSetLayout_wp SkinnedMeshDescriptorSetLayout;

    class SkinnedMeshRenderObject final : public rendering::RenderObject
    {
    public:
        array_list<rendering::MeshBatch> m_batches;
        RCPtr<SkinnedMesh>               m_skinnedMesh;
        array_list<RCPtr<Material>>   m_materials;
        array_list<int32_t>           m_priorities;

        // set2 binding0: PerRendererData
        gfx::GFXBuffer_sp             m_perRendererBuffer;
        // set2 binding1: SkinnedRendererData (BoneMatrices)
        gfx::GFXBuffer_sp             m_skinningBuffer;

        gfx::GFXDescriptorSet_sp      m_descriptorSet;
        gfx::GFXDescriptorSetLayout_sp m_descriptorSetLayout;

        SkinnedMeshRenderObject* SetSkinnedMesh(RCPtr<SkinnedMesh> mesh)
        {
            m_skinnedMesh = std::move(mesh);
            return this;
        }
        SkinnedMeshRenderObject* SetMaterials(const array_list<RCPtr<Material>>& mats, const array_list<int32_t>& priorities)
        {
            m_materials = mats;
            m_priorities = priorities;
            return this;
        }

        // Animator 调用：将骨骼矩阵写入 GPU UBO
        void UploadBoneMatrices(const array_list<Matrix4f>& boneMatrices)
        {
            if (!m_skinningBuffer) return;

            SkinnedRendererData data{};
            const size_t count = std::min(boneMatrices.size(), (size_t)SKINNEDMESH_MAX_BONES);
            for (size_t i = 0; i < count; ++i)
                data.BoneMatrices[i] = boneMatrices[i];

            m_skinningBuffer->Fill(&data);
        }

        void SubmitChange();
        void OnCreateResource() override;
        void OnDestroyResource() override
        {
            m_descriptorSet.reset();
            m_descriptorSetLayout.reset();
            m_perRendererBuffer.reset();
            m_skinningBuffer.reset();
        }

        void OnChangedTransform() override
        {
            for (auto& batch : m_batches)
                batch.IsReverseCulling = IsDeterminantNegative();
            m_perRendererBuffer->Fill(&m_perModelData);
        }

        array_list<rendering::MeshBatch> GetMeshBatches() override { return m_batches; }
        std::string GetInterface() const override { return "RENDERER_SKINNEDMESH"; }
    };

    void SkinnedMeshRenderObject::OnCreateResource()
    {
        // set2 layout: binding0=PerRendererData, binding1=SkinnedRendererData
        if (SkinnedMeshDescriptorSetLayout.expired())
        {
            gfx::GFXDescriptorSetLayoutDesc bindings[2] = {
                {gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXGpuProgramStageFlags::VertexFragment,
                 kRenderingDescriptorBinding_PerRenderer,  kRenderingDescriptorSpace_ModelInfo},
                {gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXGpuProgramStageFlags::VertexFragment,
                 kRenderingDescriptorBinding_SkinningData, kRenderingDescriptorSpace_ModelInfo},
            };
            m_descriptorSetLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(bindings, 2);
            SkinnedMeshDescriptorSetLayout = m_descriptorSetLayout;
        }
        else
        {
            m_descriptorSetLayout = SkinnedMeshDescriptorSetLayout.lock();
        }

        // binding0: PerRendererData
        {
            gfx::GFXBufferDesc desc{};
            desc.Usage       = gfx::GFXBufferUsage::ConstantBuffer;
            desc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnDevice;
            desc.BufferSize  = sizeof(PerRendererData);
            desc.ElementSize = sizeof(PerRendererData);
            m_perRendererBuffer = Application::GetGfxApp()->CreateBuffer(desc);
        }

        // binding1: SkinnedRendererData（初始化为单位矩阵）
        {
            gfx::GFXBufferDesc desc{};
            desc.Usage       = gfx::GFXBufferUsage::ConstantBuffer;
            desc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnDevice;
            desc.BufferSize  = sizeof(SkinnedRendererData);
            desc.ElementSize = sizeof(SkinnedRendererData);
            m_skinningBuffer = Application::GetGfxApp()->CreateBuffer(desc);

            // 默认骨骼矩阵全部为单位矩阵（静止姿势）
            SkinnedRendererData defaultData{};
            for (auto& mat : defaultData.BoneMatrices)
                mat = Matrix4f(1);
            m_skinningBuffer->Fill(&defaultData);
        }

        m_descriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_descriptorSetLayout);
        m_descriptorSet->AddDescriptor("PerRenderer", kRenderingDescriptorBinding_PerRenderer)
                       ->SetConstantBuffer(m_perRendererBuffer.get());
        m_descriptorSet->AddDescriptor("SkinningData", kRenderingDescriptorBinding_SkinningData)
                       ->SetConstantBuffer(m_skinningBuffer.get());
        m_descriptorSet->Submit();

        SubmitChange();
    }

    void SkinnedMeshRenderObject::SubmitChange()
    {
        m_batches.clear();
        if (!m_skinnedMesh) return;

        for (int matIndex = 0; matIndex < (int)m_materials.size(); ++matIndex)
        {
            auto& slot = m_materials.at(matIndex);

            auto& batch = m_batches.emplace_back();
            batch.State.Topology  = gfx::GFXPrimitiveTopology::TriangleList;
            batch.State.VertexLayouts = {SkinnedMesh::StaticGetVertexLayout()};
            batch.IsUsedIndices   = true;
            batch.IsCastShadow    = true;
            batch.Material        = slot;
            batch.Priority        = (matIndex < (int)m_priorities.size()) ? m_priorities.at(matIndex) : 0;

            bool isInvalidMaterial = !batch.Material
                || !batch.Material->GetShader()
                || !batch.Material->CreateGPUResource();
            if (isInvalidMaterial)
            {
                batch.Material = AssetManager::Get()->LoadAsset<Material>("Engine/Materials/Error");
                if (batch.Material) batch.Material->CreateGPUResource();
            }
            if (!batch.Material || !batch.Material->GetShader())
            {
                m_batches.pop_back();
                continue;
            }

            if (batch.Material->GetShader()->GetConfig() &&
                batch.Material->GetShader()->GetConfig()->Passes &&
                !batch.Material->GetShader()->GetConfig()->Passes->empty())
            {
                auto& pass0 = (*batch.Material->GetShader()->GetConfig()->Passes)[0];
                batch.Queue = pass0->Queue;
                if (pass0->GraphicsPipeline)
                    batch.CullMode = pass0->GraphicsPipeline->CullMode;
            }

            batch.Interface           = GetInterface();
            batch.DescriptorSetLayout = m_descriptorSetLayout;

            if (!m_skinnedMesh->IsCreatedGPUResource())
                m_skinnedMesh->CreateGPUResource();

            auto vertBuffers    = m_skinnedMesh->GetGPUResourceVertexBuffers();
            auto indicesBuffers = m_skinnedMesh->GetGPUResourceIndicesBuffers();

            if (matIndex < (int)vertBuffers.size())
            {
                auto& element         = batch.Elements.emplace_back();
                element.Vertex        = vertBuffers[matIndex];
                element.Indices       = indicesBuffers[matIndex];
                element.ModelDescriptor = m_descriptorSet;
            }
        }
    }

    // -----------------------------------------------------------------------
    // SkinnedMeshRendererComponent
    // -----------------------------------------------------------------------
    SkinnedMeshRendererComponent::SkinnedMeshRendererComponent()
    {
        m_canDrawGizmo = true;
        init_sptr_member(m_materials);
        init_sptr_member(m_priorities);
    }

    SPtr<rendering::RenderObject> SkinnedMeshRendererComponent::CreateRenderObject()
    {
        auto ro = mksptr(new SkinnedMeshRenderObject());
        if (m_skinnedMesh)
        {
            m_skinnedMesh->CreateGPUResource();
            for (const auto& mat : *m_materials)
            {
                if (mat)
                    mat->CreateGPUResource();
            }
            ro->SetSkinnedMesh(m_skinnedMesh)
              ->SetMaterials(*m_materials, *m_priorities)
              ->SubmitChange();
        }
        return ro;
    }

    void SkinnedMeshRendererComponent::BeginComponent()
    {
        base::BeginComponent();
        m_renderObject = sptr_static_cast<SkinnedMeshRenderObject>(CreateRenderObject());
        GetWorld()->AddRenderObject(m_renderObject);
        ResizeMaterials(m_materials->size());
        RebuildBoneReferences();
        OnTransformChanged();
    }

    void SkinnedMeshRendererComponent::EndComponent()
    {
        base::EndComponent();
        GetWorld()->RemoveRenderObject(m_renderObject);
        m_renderObject.reset();
    }

    void SkinnedMeshRendererComponent::OnTick(Ticker ticker)
    {
        base::OnTick(ticker);

        if (!m_skinnedMesh || !m_skinnedMesh->GetSkeleton()) return;
        if (!m_root) return;

        const auto& skeletonBones = m_skinnedMesh->GetSkeleton()->GetBones();
        if (skeletonBones.empty()) return;
        if (m_bones->size() != skeletonBones.size()) return;
        if (!m_bones->at(0)) return;

        static bool s_loggedSkinningDebug = false;
        static bool s_loggedSkinMatrices = false;
        if (!s_loggedSkinningDebug)
        {
            for (int i = 0; i < (int)skeletonBones.size() && i < 4; ++i)
            {
                auto boneTransform = m_bones->at(i);
                Logger::Log(
                    "SkinnedMeshRendererComponent::OnTick bone bind debug - bone=" + skeletonBones[i].Name +
                    ", path=" + skeletonBones[i].Path +
                    ", parent=" + std::to_string(skeletonBones[i].ParentIndex) +
                    ", resolved=" + string(boneTransform ? boneTransform->GetNode()->GetName() : "<null>") +
                    ", invBindT=(" + std::to_string(skeletonBones[i].InverseBindMatrix[3][0]) + "," + std::to_string(skeletonBones[i].InverseBindMatrix[3][1]) + "," + std::to_string(skeletonBones[i].InverseBindMatrix[3][2]) + ")",
                    LogLevel::Warning);
            }
            s_loggedSkinningDebug = true;
        }

        const int rootBoneIndex = m_skinnedMesh->GetSkeleton()->GetRootBoneIndex();
        if (rootBoneIndex < 0 || rootBoneIndex >= (int)m_bones->size() || !m_bones->at(rootBoneIndex))
            return;

        const Matrix4f rendererRootWorldInv = jmath::Inverse(m_root->GetLocalToWorldMatrix());

        array_list<Matrix4f> boneMatrices(skeletonBones.size(), Matrix4f(1.f));
        for (int i = 0; i < (int)skeletonBones.size(); ++i)
        {
            auto boneTransform = m_bones->at(i);
            if (!boneTransform) continue;

            const Matrix4f boneModelMatrix = rendererRootWorldInv * boneTransform->GetLocalToWorldMatrix();
            boneMatrices[i] = boneModelMatrix * skeletonBones[i].InverseBindMatrix;

            if (i < 4 && !s_loggedSkinMatrices)
            {
                const auto& finalMat = boneMatrices[i];
                const float diag0 = finalMat[0][0];
                const float diag1 = finalMat[1][1];
                const float diag2 = finalMat[2][2];
                const float offDiag =
                    std::abs(finalMat[0][1]) + std::abs(finalMat[0][2]) +
                    std::abs(finalMat[1][0]) + std::abs(finalMat[1][2]) +
                    std::abs(finalMat[2][0]) + std::abs(finalMat[2][1]);
                const float translationAbs =
                    std::abs(finalMat[3][0]) + std::abs(finalMat[3][1]) + std::abs(finalMat[3][2]);
                const bool nearIdentity =
                    std::abs(diag0 - 1.f) < 0.05f &&
                    std::abs(diag1 - 1.f) < 0.05f &&
                    std::abs(diag2 - 1.f) < 0.05f &&
                    offDiag < 0.05f &&
                    translationAbs < 0.05f;

                Logger::Log(
                    "SkinnedMeshRendererComponent::OnTick skin matrix debug - bone=" + skeletonBones[i].Name +
                    ", modelT=(" + std::to_string(boneModelMatrix[3][0]) + "," + std::to_string(boneModelMatrix[3][1]) + "," + std::to_string(boneModelMatrix[3][2]) + ")" +
                    ", finalT=(" + std::to_string(finalMat[3][0]) + "," + std::to_string(finalMat[3][1]) + "," + std::to_string(finalMat[3][2]) + ")" +
                    ", diag=(" + std::to_string(diag0) + "," + std::to_string(diag1) + "," + std::to_string(diag2) + ")" +
                    ", offDiagSum=" + std::to_string(offDiag) +
                    ", nearIdentity=" + string(nearIdentity ? "true" : "false"),
                    LogLevel::Warning);
                if (i == std::min(3, (int)skeletonBones.size() - 1))
                {
                    s_loggedSkinMatrices = true;
                }
            }
        }

        UpdateBoneMatrices(boneMatrices);
    }

    void SkinnedMeshRendererComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        if (info->GetName() == NAMEOF(m_skinnedMesh))
        {
            SetSkinnedMesh(m_skinnedMesh);
        }
        else if (info->GetName() == NAMEOF(m_materials))
        {
            if (m_materialsSize != m_materials->size())
                ResizeMaterials(m_materials->size());
            OnMaterialChanged();
        }
        else if (info->GetName() == NAMEOF(m_root))
        {
            RebuildBoneReferences();
        }
        else if (info->GetName() == NAMEOF(m_bones))
        {
            RebuildBoneReferences();
        }
    }

    void SkinnedMeshRendererComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        base::OnDrawGizmo(painter, selected);

        if (!m_skinnedMesh || !m_skinnedMesh->GetSkeleton()) return;

        const auto& bones = m_skinnedMesh->GetSkeleton()->GetBones();
        if (bones.empty()) return;

            painter->Context.LineTint = selected
                ? Color4f{0.2f, 1.f, 0.4f, 1.f}
                : Color4f{0.2f, 0.6f, 1.f, 0.8f};
            painter->Context.LineModelMatrix = Matrix4f(1.f);

            static array_list<Vector3f> jointSphere = []()
            {
                auto sphere = SimplePrimitiveUtils::CreateSphere<Vector3f>(12);
                return array_list<Vector3f>(sphere.begin(), sphere.end());
            }();

        const bool hasBoneRefs = m_root
            && m_bones
            && m_bones->size() == bones.size();

        auto getBoneWorldPos = [&](int i) -> Vector3f
        {
            if (hasBoneRefs)
            {
                auto boneTransform = m_bones->at(i);
                if (boneTransform)
                {
                    const Matrix4f& wm = boneTransform->GetLocalToWorldMatrix();
                    return { wm[3][0], wm[3][1], wm[3][2] };
                }
            }

            // Fallback：bind pose 位置（相对 Skeleton 显式记录的根骨骼；若缺失则退回小场景根）
            Matrix4f bindMat = jmath::Inverse(bones[i].InverseBindMatrix);
            const Matrix4f& rootWorld = m_root
                ? m_root->GetLocalToWorldMatrix()
                : GetNode()->GetTransform()->GetLocalToWorldMatrix();
            Vector4f wp = rootWorld * Vector4f(bindMat[3][0], bindMat[3][1], bindMat[3][2], 1.f);
            return { wp.x, wp.y, wp.z };
        };

        for (int i = 0; i < (int)bones.size(); ++i)
        {
            int parentIdx = bones[i].ParentIndex;
            Vector3f bonePos = getBoneWorldPos(i);

            const Color4b jointColor = hasBoneRefs
                ? (selected ? Color4b{ 120, 255, 170, 255 } : Color4b{ 90, 220, 255, 220 })
                : (selected ? Color4b{ 255, 220, 120, 255 } : Color4b{ 180, 210, 255, 200 });
            const Color4b connectorColor = hasBoneRefs
                ? (selected ? Color4b{ 120, 255, 170, 255 } : Color4b{ 90, 220, 255, 220 })
                : (selected ? Color4b{ 255, 220, 120, 255 } : Color4b{ 180, 210, 255, 200 });

            const float jointRadius = 0.18f;

            if (parentIdx >= 0)
            {
                Vector3f parentPos = getBoneWorldPos(parentIdx);
                Vector3f axis = bonePos - parentPos;
                float boneLen = Magnitude(axis);
                if (boneLen > 0.0001f)
                {
                    Vector3f dir = axis / boneLen;
                    float clampedRadius = std::min(jointRadius, boneLen * 0.2f);
                    Vector3f start = parentPos + dir * clampedRadius;
                    Vector3f end = bonePos - dir * clampedRadius;

                    Vector3f basisHint = fabsf(dir.y) < 0.95f ? Vector3f{0.f, 1.f, 0.f} : Vector3f{1.f, 0.f, 0.f};
                    Vector3f sideA = Normalize(Cross(dir, basisHint));
                    Vector3f sideB = Normalize(Cross(dir, sideA));
                    float baseRadius = std::min(jointRadius * 0.8f, boneLen * 0.16f);
                    float tipRadius = std::min(jointRadius * 0.28f, boneLen * 0.06f);

                    Vector3f startA = start + sideA * baseRadius;
                    Vector3f startB = start - sideA * baseRadius;
                    Vector3f startC = start + sideB * baseRadius;
                    Vector3f startD = start - sideB * baseRadius;
                    Vector3f endA = end + sideA * tipRadius;
                    Vector3f endB = end - sideA * tipRadius;
                    Vector3f endC = end + sideB * tipRadius;
                    Vector3f endD = end - sideB * tipRadius;

                    auto drawBoneEdge = [&](const Vector3f& p0, const Vector3f& p1)
                    {
                        StaticMeshVertex a{}, b{};
                        a.Position = p0;
                        b.Position = p1;
                        a.Color = b.Color = connectorColor;
                        painter->DrawLine(a, b);
                    };

                    drawBoneEdge(startA, endA);
                    drawBoneEdge(startB, endB);
                    drawBoneEdge(startC, endC);
                    drawBoneEdge(startD, endD);
                    drawBoneEdge(startA, startC);
                    drawBoneEdge(startC, startB);
                    drawBoneEdge(startB, startD);
                    drawBoneEdge(startD, startA);
                    drawBoneEdge(endA, endC);
                    drawBoneEdge(endC, endB);
                    drawBoneEdge(endB, endD);
                    drawBoneEdge(endD, endA);
                }
            }

            auto drawSphere = [&](const Vector3f& center, float radius, Color4b color)
            {
                array_list<StaticMeshVertex> sphereVerts;
                sphereVerts.reserve(jointSphere.size());
                for (auto p : jointSphere)
                {
                    auto& v = sphereVerts.emplace_back();
                    v.Position = center + p * radius;
                    v.Color = color;
                }
                painter->DrawLineArray(sphereVerts);
            };

            drawSphere(bonePos, jointRadius, jointColor);
        }
    }

    void SkinnedMeshRendererComponent::SetSkinnedMesh(const RCPtr<SkinnedMesh>& mesh)
    {
        m_skinnedMesh = mesh;
        RebuildBoneReferences();
        OnMeshChanged();
    }

    void SkinnedMeshRendererComponent::SetRoot(SceneObjectPtr<TransformComponent> root)
    {
        m_root = std::move(root);
        RebuildBoneReferences();
    }

    void SkinnedMeshRendererComponent::UpdateBoneMatrices(const array_list<Matrix4f>& boneMatrices)
    {
        if (m_renderObject)
            m_renderObject->UploadBoneMatrices(boneMatrices);
    }

    RCPtr<Material> SkinnedMeshRendererComponent::GetMaterial(int index) const
    {
        return m_materials->at(index);
    }

    void SkinnedMeshRendererComponent::SetMaterial(int index, RCPtr<Material> material)
    {
        if (index >= (int)m_materials->size()) return;
        m_materials->at(index) = std::move(material);
        OnMaterialChanged();
    }

    size_t SkinnedMeshRendererComponent::AddMaterial(RCPtr<Material> material)
    {
        ResizeMaterials(m_materials->size() + 1);
        if (material) SetMaterial((int)m_materialsSize - 1, material);
        return m_materialsSize - 1;
    }

    void SkinnedMeshRendererComponent::GetSubscribeObserverHandles(array_list<ObjectHandle>& out)
    {
        base::GetSubscribeObserverHandles(out);
        out.push_back(m_skinnedMesh.GetHandle());
        for (auto& mat : *m_materials)
        {
            if (mat) out.push_back(mat.GetHandle());
        }
    }

    void SkinnedMeshRendererComponent::OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg)
    {
        base::OnNotifyObserver(inDependency, msg);
    }

    void SkinnedMeshRendererComponent::ResizeMaterials(size_t size)
    {
        m_materialsSize = size;
        m_materials->resize(size);
        RebuildObserver();
    }

    void SkinnedMeshRendererComponent::OnTransformChanged()
    {
        base::OnTransformChanged();
        if (m_renderObject)
            m_renderObject->SetTransform(GetNode()->GetTransform()->GetLocalToWorldMatrix());
    }

    void SkinnedMeshRendererComponent::OnMeshChanged()
    {
        if (m_skinnedMesh && m_skinnedMesh->GetMaterialCount() > m_materials->size())
        {
            ResizeMaterials(m_skinnedMesh->GetMaterialCount());
            OnMaterialChanged();
        }
        if (m_renderObject)
            m_renderObject->SetSkinnedMesh(m_skinnedMesh)->SubmitChange();
        RebuildObserver();
    }

    void SkinnedMeshRendererComponent::OnMaterialChanged()
    {
        if (m_renderObject)
            m_renderObject->SetMaterials(*m_materials, *m_priorities)->SubmitChange();
        RebuildObserver();
    }

    void SkinnedMeshRendererComponent::RebuildBoneReferences()
    {
        if (!m_bones)
        {
            m_bones = mksptr(new List<SceneObjectPtr<TransformComponent>>);
        }
        m_bones->clear();

        if (!m_root || !m_skinnedMesh || !m_skinnedMesh->GetSkeleton())
            return;

        const auto& skeletonBones = m_skinnedMesh->GetSkeleton()->GetBones();
        m_bones->resize(skeletonBones.size());

        auto sceneRoot = m_root;

        for (int i = 0; i < (int)skeletonBones.size(); ++i)
        {
            const auto& bone = skeletonBones[i];
            SceneObjectPtr<TransformComponent> resolvedBone;
            if (sceneRoot)
            {
                resolvedBone = !bone.Path.empty()
                    ? SceneObjectPtr<TransformComponent>(sceneRoot->FindByPath(bone.Path))
                    : SceneObjectPtr<TransformComponent>(sceneRoot->FindByName(bone.Name));
            }
            m_bones->at(i) = resolvedBone;
        }
    }

    void SkinnedMeshRendererComponent::GetDependenciesAsset(array_list<guid_t>& deps) const
    {
        SceneObject::GetDependenciesAsset(deps);
        if (m_skinnedMesh)
        {
            deps.push_back(m_skinnedMesh.GetGuid());
        }
        if (m_materials)
        {
            for (auto& mat : *m_materials)
            {
                if (mat)
                {
                    deps.push_back(mat.GetGuid());
                }
            }
        }
    }

} // namespace pulsar
