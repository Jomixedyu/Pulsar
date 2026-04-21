#pragma once
#include <Pulsar/AssetObject.h>
#include <Pulsar/EngineMath.h>

namespace pulsar
{
    // 单根骨骼的描述（层级 + 绑定姿势逆矩阵）
    struct BoneInfo
    {
        string   Name;
        string   Path;               // 从导入的小场景根开始的层级路径，例如 joint1/joint2/joint3
        int32_t  ParentIndex = -1;   // -1 表示根骨骼
        Vector3f LocalTranslation = {0.f, 0.f, 0.f};
        Quat4f   LocalRotation = {0.f, 0.f, 0.f, 1.f};
        Vector3f LocalScale = {1.f, 1.f, 1.f};
        Matrix4f BindModelMatrix = Matrix4f(1.f);   // 骨骼在 canonical model space 下的 bind pose
        Matrix4f InverseBindMatrix = Matrix4f(1.f); // 将顶点从模型空间变回骨骼本地空间
    };

    // Skeleton 资产：独立存储骨骼层级，可被多个 SkinnedMesh / AnimationClip 共享
    class Skeleton : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Skeleton, AssetObject);
        CORELIB_CLASS_ATTR(new AssetIconAttribute("Editor/Icons/skeleton.png"))
    public:
        using ThisClass = Skeleton;

        static RCPtr<Skeleton> StaticCreate(string_view name, array_list<BoneInfo>&& bones, int32_t rootBoneIndex = 0);

        const array_list<BoneInfo>& GetBones()     const { return m_bones; }
        int32_t                     GetBoneCount() const { return (int32_t)m_bones.size(); }
        int32_t                     GetRootBoneIndex() const { return m_rootBoneIndex; }

        // 按名字查找骨骼索引，找不到返回 -1
        int32_t FindBoneIndex(const string& name) const;

        void Serialize(AssetSerializer* s) override;
        void OnInstantiateAsset(AssetObject* obj) override;
        // Skeleton 不引用其他资产，无需 override OnCollectAssetDependencies

    private:
        array_list<BoneInfo> m_bones;
        int32_t m_rootBoneIndex = 0;

        friend class SkinnedMesh;
    };
    DECL_PTR(Skeleton);

} // namespace pulsar
