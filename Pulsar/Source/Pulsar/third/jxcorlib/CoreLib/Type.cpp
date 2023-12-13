#include "Type.h"

#include <vector>
#include <iostream>

#include "CommonException.h"
#include "Reflection.h"
#include "Assembly.h"
#include "BasicTypes.h"
#include "Attribute.h"

namespace jxcorlib
{

    Type::Type(
        CreateInstFunc dyncreate,
        Assembly* assembly,
        Type* base,
        const string& name,
        const std::type_info& info,
        int32_t structure_size,
        bool isGeneric)
        :
        m_createInstanceFunc(dyncreate),
        m_base(base),
        m_typeinfo(info),
        m_structureSize(structure_size),
        m_assembly(assembly),
        m_isInterface(false),
        m_enumGetter(nullptr),
        m_isGeneric(isGeneric)
    {
        assert(assembly);
        assert(name.length());

        if (name[0] == ':')
        {
            for (size_t i = 0; i < name.length(); i++)
            {
                if (name[i] != ':')
                {
                    m_name = name.substr(i, name.size() - i);
                    break;
                }
            }
            assert(m_name.length());
        }
        else
        {
            m_name = name;
        }

        auto pos = m_name.find_last_of("::");
        if (pos != m_name.npos)
        {
            m_shortName = m_name.substr(pos + 1, m_name.size() - pos - 1);
        }
        else
        {
            m_shortName = m_name;
        }
    }

    bool Type::IsImplementedInterface(Type* type)
    {
        for (auto& [item, fun, sfun] : this->m_interfaces)
        {
            if (type->IsSubclassOf(item))
            {
                return true;
            }
        }
        return false;
    }

    array_list<Type*> Type::GetInterfaces() const
    {
        array_list<Type*> ret;
        for (auto& [type, _, __] : this->m_interfaces)
        {
            ret.push_back(type);
        }
        return ret;
    }

    string Type::ToString() const
    {
        return this->m_name;
    }

    bool Type::IsInstanceOfType(const Object* object) const
    {
        if(object == nullptr) return false;
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
                base = base->GetBase();
            }
        }
        return false;
    }

    Object* Type::CreateInstance(const ParameterPackage& v)
    {
        if (this->m_createInstanceFunc == nullptr) {
            throw NotImplementException(this->GetName() + ": the creation method is not implemented");
        }

        return (*this->m_createInstanceFunc)(v);
    }

    sptr<Object> Type::CreateSharedInstance(const ParameterPackage& v)
    {
        if (this->m_createInstanceFunc == nullptr) {
            throw NotImplementException(this->GetName() + ": the creation method is not implemented");
        }
        return mksptr((*this->m_createInstanceFunc)(v));
    }


    Type* Type::StaticType()
    {
        static Type* type = nullptr;
        if (type == nullptr)
        {
            Assembly* assm = AssemblyManager::BuildAssemblyByName(AssemblyObject_jxcorlib.name());
            type = new Type(nullptr, assm, cltypeof<Object>(), "jxcorlib::Type", typeid(Object), sizeof(Object));
            assm->RegisterType(type);
        }
        return type;
    }

    bool Type::IsPrimitiveType() const
    {
        return this->IsSubclassOf(cltypeof<PrimitiveObject>());
    }

    bool Type::IsBoxingType() const
    {
        return this->IsSubclassOf(cltypeof<BoxingObject>());
    }

    void Type::GetMemberInfos(array_list<MemberInfo*>& out, TypeBinding attr)
    {
        for (auto& [name, info] : this->m_memberInfos)
        {
            if (!EnumHasFlag(attr, TypeBinding::NonPublic))
            {
                if (!info->m_isPublic) continue;
            }

            out.push_back(info.get());
        }
        if (this->GetBase())
        {
            this->GetBase()->GetMemberInfos(out, attr);
        }
    }

    array_list<MemberInfo*> Type::GetMemberInfos(TypeBinding attr)
    {
        array_list<MemberInfo*> v;
        this->GetMemberInfos(v, attr);
        return v;
    }

    MemberInfo* Type::GetMemberInfo(const string& name)
    {
        if (this->m_memberInfos.find(name) == this->m_memberInfos.end())
        {
            return nullptr;
        }
        return this->m_memberInfos.at(name).get();
    }

    void Type::GetFieldInfos(array_list<FieldInfo*>& out, TypeBinding attr)
    {
        Type* fieldinfo_type = cltypeof<FieldInfo>();

        for (auto& [name, info] : this->m_memberInfos)
        {
            if (!fieldinfo_type->IsInstanceOfType(info.get()))
            {
                continue;
            }
            if (!EnumHasFlag(attr, TypeBinding::NonPublic))
            {
                if (!info->m_isPublic) continue;
            }
            out.push_back(static_cast<FieldInfo*>(info.get()));
        }
        if (this->GetBase())
        {
            this->GetBase()->GetFieldInfos(out, attr);
        }
    }

    array_list<FieldInfo*> Type::GetFieldInfos(TypeBinding attr)
    {
        array_list<FieldInfo*> v;
        this->GetFieldInfos(v, attr);
        return v;
    }

    FieldInfo* Type::GetFieldInfo(const string& name)
    {
        if (this->m_memberInfos.find(name) == this->m_memberInfos.end())
        {
            return nullptr;
        }
        MemberInfo* info = this->m_memberInfos.at(name).get();
        if (!info->GetType()->IsSubclassOf(cltypeof<FieldInfo>()))
        {
            return nullptr;
        }
        return static_cast<FieldInfo*>(info);
    }

    void Type::GetMethodInfos(array_list<MethodInfo*>& out, TypeBinding attr)
    {
        Type* methodi_type = cltypeof<MethodInfo>();

        for (auto& [name, info] : this->m_memberInfos)
        {
            if (!methodi_type->IsInstanceOfType(info.get()))
            {
                continue;
            }
            if (!EnumHasFlag(attr, TypeBinding::NonPublic))
            {
                if (!info->m_isPublic) continue;
            }
            out.push_back(static_cast<MethodInfo*>(info.get()));
        }
        if (this->GetBase())
        {
            this->GetBase()->GetMethodInfos(out, attr);
        }
    }

    array_list<MethodInfo*> Type::GetMethodInfos(TypeBinding attr)
    {
        array_list<MethodInfo*> v;
        this->GetMethodInfos(v, attr);
        return v;
    }

    MethodInfo* Type::GetMethodInfo(const string& name)
    {
        if (this->m_memberInfos.find(name) == this->m_memberInfos.end())
        {
            return nullptr;
        }
        auto info = this->m_memberInfos.at(name);
        if (!info->GetType()->IsSubclassOf(cltypeof<MethodInfo>()))
        {
            return nullptr;
        }
        return static_cast<MethodInfo*>(info.get());
    }
    void Type::_AddMemberInfo(MemberInfo* info)
    {
        this->m_memberInfos.insert({ info->GetName(), mksptr(info) });
    }

    IInterface_sp Type::GetSharedInterface(Object_rsp instance, Type* type)
    {
        for (auto& [ty, func, sfunc] : this->m_interfaces)
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
        for (auto& [ty, func, sfunc] : this->m_interfaces)
        {
            if (type->IsSubclassOf(ty))
            {
                return func(instance);
            }
        }
        if (this->GetBase())
        {
            IInterface* intfc = this->GetBase()->GetInterface(instance, type);
            if (intfc) { return intfc; }
        }
        return nullptr;
    }

    sptr<Attribute> Type::GetAttribute(Type* type, bool inherit)
    {
        Type* target = this;
        do
        {
            for (auto& attr : target->m_attributes)
            {
                if (type->IsInstanceOfType(attr.get()))
                {
                    return attr;
                }
            }
            // not found
            if (inherit)
            {
                target = target->GetBase();
            }
            else
            {
                break;
            }
        } while (target);

        return nullptr;
    }
    array_list<sptr<Attribute>> Type::GetAttributes(Type* type, bool inherit)
    {
        array_list<sptr<Attribute>> ret;
        Type* target = this;
        do
        {
            for (auto& attr : target->m_attributes)
            {
                if (type->IsInstanceOfType(attr.get()))
                {
                    ret.push_back(attr);
                }
            }
            if (inherit)
            {
                target = target->GetBase();
            }
            else
            {
                target = nullptr;
            }
        } while (target);

        return ret;
    }

    bool Type::IsDefinedAttribute(Type* type, bool inherit)
    {
        return GetAttribute(type, inherit) != nullptr;
    }

    Type* IInterface::StaticType()
    {
        static Type* type = nullptr;
        if (type == nullptr)
        {
            Assembly* assm = ::jxcorlib::AssemblyManager::BuildAssemblyByName(AssemblyObject_jxcorlib.name());
            type = new Type(nullptr, assm, cltypeof<Object>(), "jxcorlib::IInterface", typeid(IInterface), sizeof(IInterface));
            type->m_isInterface = true;
            assm->RegisterType(type);
        }
        return type;
    }

}
