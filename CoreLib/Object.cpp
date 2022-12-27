#include "Object.h"
#include <vector>
#include <map>
#include "Type.h"
#include "Assembly.h"

namespace jxcorlib
{

    static Object* CreateInstance(const ParameterPackage&)
    {
        return new Object;
    }

    Type* Object::StaticType()
    {
        static Type* type = nullptr;
        if (type == nullptr)
        {
            Assembly* assm = Assembly::StaticBuildAssemblyByName(AssemblyObject_jxcorlib.name());
            type = new Type(CreateInstance, assm, nullptr, "jxcorlib::Object", typeid(Object), sizeof(Object));
            assm->RegisterType(type);
        }
        return type;
    }

    Type* Object::GetType() const
    {
        return StaticType();
    }

    Object::~Object()
    {
    }

    string Object::ToString() const
    {
        return this->GetType()->get_name();
    }

    bool Object::Equals(Object* object) const
    {
        return this == object;
    }

}

std::string std::to_string(jxcorlib::Object* obj)
{
    return obj->ToString();
}
