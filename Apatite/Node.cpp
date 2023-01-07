#include <Apatite/Node.h>
#include <Apatite/Components/Component.h>

using namespace std;

namespace apatite
{

    bool Node::get_is_active() const
    {
        auto node = self();
        while (node != nullptr)
        {
            if (!node->get_is_active())
            {
                return false;
            }
            node = this->get_parent().lock();
        }
        return true;
    }

    bool Node::get_is_active_self() const
    {
        return this->is_active_;
    }
    void Node::set_is_active_self(bool value)
    {
        this->is_active_ = value;
    }

    Node_wp Node::get_parent() const
    {
        return this->parent_;
    }
    void Node::set_parent(wptr<Node> parent, bool keep_world_transform)
    {
        if (this->parent_.expired()) //top node
        {
            if (!parent.expired())
            {
                auto new_parent = parent.lock();
                new_parent->childs_->push_back(self());
                if (keep_world_transform)
                {
                    //todo transform matrix

                }
            }
        }
        else
        {
            auto _parnet = parent.lock();

            auto self_parent = this->parent_.lock();

            auto it = std::find_if(self_parent->childs_->begin(), self_parent->childs_->end(), [this](auto& child) { return child.get() == this; });
            self_parent->childs_->erase(it);

            if (!parent.expired())
            {
                parent.lock()->childs_->push_back(self());
            }
        }
        this->parent_ = parent;
    }

    int32_t Node::get_child_count() const
    {
        return this->childs_->size();
    }


    void Node::Serialize(ser::Stream& stream, bool is_ser)
    {
    }

    void Node::OnInitialize()
    {

    }

    void Node::OnDestory()
    {

    }

    void Node::OnUpdate()
    {

    }

    Component_sp Node::AddComponent(Type* type)
    {
        auto obj = type->CreateSharedInstance({});
        Component_sp component = sptr_cast<Component, Object>(obj);
        //init
        component->node_ = self_weak();
        this->components_->push_back(component);
        component->Construct();
        return component;
    }


    Component_sp Node::GetComponent(Type* type) const
    {
        for (auto& item : *this->components_)
        {
            if (item->GetType() == type)
            {
                return item;
            }
        }
        return nullptr;
    }

    void Node::GetAllComponents(List_sp<Component_sp>& list)
    {
        for (auto& item : *this->components_)
        {
            list->push_back(item);
        }
    }
    void Node::GetChildren(List_sp<Node_sp>& list)
    {
        for (auto& item : *this->childs_)
        {
            list->push_back(item);
        }
    }
    void Node::OnInstantiateAsset(sptr<AssetObject>& obj)
    {
        auto that = sptr_cast<Node>(obj);

        for (auto& item : *this->childs_)
        {
            auto n = sptr_cast<Node>(item->InstantiateAsset());
            n->parent_ = that;
            that->childs_->push_back(n);
        }

        //todo: ser component panel data
        //for (auto& item : *this->components_)
        //{
        //    that->components_->push_back(sptr_cast<Component>(item->InstantiateAsset()));
        //}
    }
    void Node::Translate(Vector3f v)
    {
    }
    void Node::Rotate(Quat4f v)
    {
    }
    void Node::RotateEuler(Vector3f v)
    {
    }
    void Node::Scale(Vector3f v)
    {
    }
    Matrix4f Node::GetLocalMatrix() const
    {
        return {};
        //return Math::Translate(Matrix4f::StaticScalar(), this->position_) * Math::Rotate(this->rotation_) * Math::Scale(this->scale_);
    }

    Matrix4f Node::GetWorldMatrix() const
    {
        Matrix4f m = this->GetLocalMatrix();

        if (!this->parent_.expired())
        {
            Node_sp p = this->parent_.lock();
            while (p != nullptr)
            {
                m = p->GetLocalMatrix() * m;
                if (p->parent_.expired())
                {
                    p = nullptr;
                }
                else
                {
                    p = p->parent_.lock();
                }
            }
        }
        return m;
    }

    void Node::OnTick(Ticker ticker)
    {
        for (auto& comp : *this->components_)
        {
            if (comp->IsAlive())
            {
                comp->OnTick(ticker);
            }
        }
        for (auto& node : *this->childs_)
        {
            if (node->IsAlive())
            {
                node->OnTick(ticker);
            }
        }
    }

    Node_sp Node::GetChild(string_view name)
    {
        for (auto& item : *this->childs_)
        {
            if (item->name_ == name)
            {
                return item;
            }
        }
        return nullptr;
    }

    Node_sp Node::GetChildAt(int index)
    {
        return this->childs_->at(index);
    }

}

