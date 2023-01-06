#pragma once

#include <vector>
#include <Apatite/ObjectBase.h>
#include <Apatite/Node.h>
#include <Apatite/AssetObject.h>
#include <Apatite/Assets/CubeMap.h>

namespace apatite
{
    class Node;

    class Scene : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::Scene, AssetObject)
    public:
        List_rsp<Node_sp> get_nodes() const { return this->scene_nodes_; }

        virtual void Serialize(ser::Stream& stream, bool is_ser) override;

    public:
        void AddNode(Node_rsp node);
        void RemoveNode(Node_rsp node);
        void OnUpdate();

    private:

        List_sp<Node_sp> scene_nodes_;

        CORELIB_REFL_DECL_FIELD(cubemap_);
        CubeMapAsset_sp cubemap_;
    };
    CORELIB_DECL_SHORTSPTR(Scene);
}

