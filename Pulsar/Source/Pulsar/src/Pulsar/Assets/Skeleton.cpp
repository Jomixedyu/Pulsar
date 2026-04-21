#include "Assets/Skeleton.h"
#include "AssetSerializerUtil.h"
#include "EngineMath.h"

namespace pulsar
{
    // -----------------------------------------------------------------------
    // Serialization helpers
    // -----------------------------------------------------------------------
    static std::iostream& SerializeBoneInfo(std::iostream& stream, bool isWrite, BoneInfo& data)
    {
        sser::ReadWriteStream(stream, isWrite, data.Name);
        sser::ReadWriteStream(stream, isWrite, data.Path);
        sser::ReadWriteStream(stream, isWrite, data.ParentIndex);
        // Matrix4f 展开为 4 列 Vector4f
        ReadWriteStream(stream, isWrite, data.InverseBindMatrix.M[0]);
        ReadWriteStream(stream, isWrite, data.InverseBindMatrix.M[1]);
        ReadWriteStream(stream, isWrite, data.InverseBindMatrix.M[2]);
        ReadWriteStream(stream, isWrite, data.InverseBindMatrix.M[3]);
        return stream;
    }

    // -----------------------------------------------------------------------
    // Skeleton
    // -----------------------------------------------------------------------
    RCPtr<Skeleton> Skeleton::StaticCreate(string_view name, array_list<BoneInfo>&& bones, int32_t rootBoneIndex)
    {
        auto self = NewAssetObject<Skeleton>();
        self->SetIndexName(name);
        self->m_bones = std::move(bones);
        self->m_rootBoneIndex = rootBoneIndex;
        return self;
    }

    int32_t Skeleton::FindBoneIndex(const string& name) const
    {
        for (int32_t i = 0; i < (int32_t)m_bones.size(); ++i)
        {
            if (m_bones[i].Name == name)
                return i;
        }
        return -1;
    }

    void Skeleton::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        auto& stream  = s->GetStream();
        bool  isWrite = s->IsWrite;

        if (!isWrite)
        {
            m_bones.clear();
            m_rootBoneIndex = 0;
        }

        sser::ReadWriteStream(stream, isWrite, m_rootBoneIndex);

        uint32_t boneCount = static_cast<uint32_t>(m_bones.size());
        sser::ReadWriteStream(stream, isWrite, boneCount);
        if (!isWrite) m_bones.resize(boneCount);

        for (auto& bone : m_bones)
            SerializeBoneInfo(stream, isWrite, bone);
    }

    void Skeleton::OnInstantiateAsset(AssetObject* obj)
    {
        base::OnInstantiateAsset(obj);
        auto sk = static_cast<ThisClass*>(obj);
        sk->m_bones = m_bones;
        sk->m_rootBoneIndex = m_rootBoneIndex;
    }

} // namespace pulsar
