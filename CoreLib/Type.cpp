#include "Type.h"

#include <vector>
#include <iostream>

#include "CommonException.h"
#include "Reflection.h"
#include "Assembly.h"
#include "BasicTypes.h"

namespace jxcorlib
{
    static std::vector<Type*>* g_types = nullptr;

    bool Type::IsImplementedInterface(Type* type)
    {
        for (auto& [item, fun, sfun]: this->interfaces_)
        {
            if (type->IsSubclassOf(item))
            {
                return true;
            }
        }
        return false;
    }

    //array_list<Type*> Type::GetInterfaces() const
    //{
    //    return this->interfaces_;
    //}

    string Type::ToString() const
    {
        return this->name_;
    }

    bool Type::IsInstanceOfType(const Object* object) const
    {
        return object->GetType()->IsSubclassOf(this);
    }
    bool Type::IsSharedInstanceOfType(const sptr<Object>& ptr) const
    {
        return this->IsInstanceOfType(ptr.get());
    }

    bool Type::IsSubclassOf(const Type* type) const
    {
        const Type* base = this;
        while (base != nullptr)
        {
            if (base == type) {
                return true;
            }
            else {
                base = base->get_base();
            }
        }
        return false;
    }

    Object* Type::CreateInstance(const ParameterPackage& v)
    {
        if (this->c_inst_ptr_ == nullptr) {
            throw NotImplementException(this->get_name() + ": the creation method is not implemented");
        }

        return (*this->c_inst_ptr_)(v);
    }

    sptr<Object> Type::CreateSharedInstance(const ParameterPackage& v)
    {
        if (this->c_inst_ptr_ == nullptr) {
            throw NotImplementException(this->get_name() + ": the creation method is not implemented");
        }
        return mksptr((*this->c_inst_ptr_)(v));
    }


    Type* Type::StaticType()
    {
        static Type* type = nullptr;
        if (type == nullptr)
        {
            Assembly* assm = Assembly::StaticBuildAssemblyByName(AssemblyObject_jxcorlib.name());
            type = new Type(nullptr, assm, cltypeof<Object>(), "jxcorlib::Type", typeid(Object), sizeof(Object));
            assm->RegisterType(type);
        }
        return type;
    }

    bool Type::is_primitive_type() const
    {
        return this->IsSubclassOf(cltypeof<PrimitiveObject>());
    }

    bool Type::is_boxing_type() const
    {
        return this->IsSubclassOf(cltypeof<BoxingObject>());
    }

    void Type::get_memberinfos(array_list<MemberInfo*>& out, TypeBinding attr)
    {
        for (auto& [name, info] : this->member_infos_)
        {
            if (!EnumHasFlag(attr, TypeBinding::NonPublic))
            {
                if (!info->is_public_) continue;
            }
            if (EnumHasFlag(attr, TypeBinding::Static))
            {
                if (!info->is_static_) continue;
            }

            out.push_back(info);
        }
        if (this->get_base())
        {
            this->get_base()->get_memberinfos(out, attr);
        }
    }

    array_list<MemberInfo*> Type::get_memberinfos(TypeBinding attr)
    {
        array_list<MemberInfo*> v;
        this->get_memberinfos(v, attr);
        return v;
    }

    MemberInfo* Type::get_memberinfo(const string& name)
    {
        if (this->member_infos_.find(name) == this->member_infos_.end())
        {
            return nullptr;
        }
        return this->member_infos_.at(name);
    }

    void Type::get_fieldinfos(array_list<FieldInfo*>& out, TypeBinding attr)
    {
        Type* fieldinfo_type = cltypeof<FieldInfo>();

        for (auto& [name, info] : this->member_infos_)
        {
            if (!fieldinfo_type->IsInstanceOfType(info))
            {
                continue;
            }
            if (!EnumHasFlag(attr, TypeBinding::NonPublic))
            {
                if (!info->is_public_) continue;
            }
            if (EnumHasFlag(attr, TypeBinding::Static))
            {
                if (!info->is_static_) continue;
            }
            out.push_back(static_cast<FieldInfo*>(info));
        }
        if (EnumHasFlag(attr, TypeBinding::IncludeSuper) && this->get_base())
        {
            this->get_base()->get_fieldinfos(out, attr);
        }
    }

    array_list<FieldInfo*> Type::get_fieldinfos(TypeBinding attr)
    {
        array_list<FieldInfo*> v;
        this->get_fieldinfos(v, attr);
        return v;
    }

    FieldInfo* Type::get_fieldinfo(const string& name)
    {
        if (this->member_infos_.find(name) == this->member_infos_.end())
        {
            return nullptr;
        }
        MemberInfo* info = this->member_infos_.at(name);
        if (!info->GetType()->IsSubclassOf(cltypeof<FieldInfo>()))
        {
            return nullptr;
        }
        return static_cast<FieldInfo*>(info);
    }

    void Type::get_methodinfos(array_list<MethodInfo*>& out, TypeBinding attr)
    {
        Type* methodi_type = cltypeof<MethodInfo>();

        for (auto& [name, info] : this->member_infos_)
        {
            if (!methodi_type->IsInstanceOfType(info))
            {
                continue;
            }
            if (!EnumHasFlag(attr, TypeBinding::NonPublic))
            {
                if (!info->is_public_) continue;
            }
            if (EnumHasFlag(attr, TypeBinding::Static))
            {
                if (!info->is_static_) continue;
            }
            out.push_back(static_cast<MethodInfo*>(info));
        }
        if (this->get_base())
        {
            this->get_base()->get_methodinfos(out, attr);
        }
    }

    array_list<MethodInfo*> Type::get_methodinfos(TypeBinding attr)
    {
        array_list<MethodInfo*> v;
        this->get_methodinfos(v, attr);
        return v;
    }

    MethodInfo* Type::get_methodinfo(const string& name)
    {
        if (this->member_infos_.find(name) == this->member_infos_.end())
        {
            return nullptr;
        }
        MemberInfo* info = this->member_infos_.at(name);
        if (!info->GetType()->IsSubclassOf(cltypeof<MethodInfo>()))
        {
            return nullptr;
        }
        return static_cast<MethodInfo*>(info);
    }
    void Type::_AddMemberInfo(MemberInfo* info)
    {
        this->member_infos_.insert({ info->get_name(), info });
    }

    IInterface_sp Type::GetSharedInterface(Object_rsp instance, Type* type)
    {
        for (auto& [ty, func, sfunc] : this->interfaces_)
        {
            if (type->IsSubclassOf(ty))
            {
                return sfunc(instance);
            }
        }
        return nullptr;
    }

    IInterface* Type::GetInterface(Object* instance, Type* type)
    {
        for (auto& [ty, func, sfunc] : this->interfaces_)
        {
            if (type->IsSubclassOf(ty))
            {
                return func(instance);
            }
        }
        if (this->get_base())
        {
            IInterface* intfc = this->get_base()->GetInterface(instance, type);
            if (intfc) { return intfc; }
        }
        return nullptr;
    }

    Type* IInterface::StaticType()
    {
        static Type* type = nullptr;
        if (type == nullptr)
        {
            Assembly* assm = ::jxcorlib::Assembly::StaticBuildAssemblyByName(AssemblyObject_jxcorlib.name());
            type = new Type(nullptr, assm, cltypeof<Object>(), "jxcorlib::IInterface", typeid(IInterface), sizeof(IInterface));
            type->is_interface_ = true;
            assm->RegisterType(type);
        }
        return type;
    }

}
