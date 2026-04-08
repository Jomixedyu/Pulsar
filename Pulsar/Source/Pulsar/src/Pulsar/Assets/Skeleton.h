#pragma once
#include <Pulsar/AssetObject.h>
#include <Pulsar/EngineMath.h>

namespace pulsar
{
    // 单根骨骼的描述（层级 + 绑定姿势逆矩阵）
    struct BoneInfo
    {
        string   Name;
        int32_t  ParentIndex = -1;   // -1 表示根骨骼
        Matrix4f InverseBindMatrix;  // 将顶点从模型空间变回骨骼本地空间
    };

    // Skeleton 资产：独立存储骨骼层级，可被多个 SkinnedMesh / AnimationClip 共享
    class Skeleton : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Skeleton, AssetObject);
        CORELIB_CLASS_ATTR(new AssetIconAttribute("Editor/Icons/skeleton.png"))
    public:
        using ThisClass = Skeleton;

        static RCPtr<Skeleton> StaticCreate(string_view name, array_list<BoneInfo>&& bones);

        const array_list<BoneInfo>& GetBones()     const { return m_bones; }
        int32_t                     GetBoneCount() const { return (int32_t)m_bones.size(); }

        // 按名字查找骨骼索引，找不到返回 -1
        int32_t FindBoneIndex(const string& name) const;

        void Serialize(AssetSerializer* s) override;
        void OnInstantiateAsset(AssetObject* obj) override;

    private:
        array_list<BoneInfo> m_bones;

        friend class SkinnedMesh;
    };
    DECL_PTR(Skeleton);

} // namespace pulsar
