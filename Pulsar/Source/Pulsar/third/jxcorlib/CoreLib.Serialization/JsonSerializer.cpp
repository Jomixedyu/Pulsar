#include "JsonSerializer.h"
#include "json.hpp"
#include <CoreLib/Reflection.h>
#include <cassert>

namespace jxcorlib::ser
{
    using namespace nlohmann;
    using namespace jxcorlib;

    static json _SerializeArray(IList* list, const JsonSerializerSettings& settings, FieldInfo* fieldInfo = nullptr);
    static json _SerializeClassObject(Object* obj, const JsonSerializerSettings& settings);


    static json _SerializeObject(Object* obj, const JsonSerializerSettings& settings, FieldInfo* fieldInfo = nullptr)
    {
        if (obj == nullptr)
        {
            return json::object();
        }

        if (obj->GetType()->IsPrimitiveType())
        {
            json prim_js;
            PrimitiveObjectUtil::Assign(&prim_js, obj);
            return std::move(prim_js);
        }

        if (obj->GetType()->IsEnum())
        {
            json enum_js;
            if (settings.StringEnum)
            {
                enum_js = static_cast<Enum*>(obj)->GetName();
            }
            else
            {
                enum_js = static_cast<Enum*>(obj)->GetValue();
            }
            return enum_js;
        }

        if (IStringify* stringify = interface_cast<IStringify>(obj))
        {
            return json(stringify->IStringify_Stringify());
        }

        //list
        if (IList* list = interface_cast<IList>(obj))
        {
            return _SerializeArray(list, settings, fieldInfo);
        }

        //other
        return _SerializeClassObject(obj, settings);

    }

    static json _SerializeArray(IList* list, const JsonSerializerSettings& settings, FieldInfo* fieldInfo)
    {
        int32_t count = list->GetCount();

        json arr_json = json::array();

        for (int32_t i = 0; i < count; i++)
        {
            Object_rsp element = list->At(i);
            arr_json.push_back(_SerializeObject(element.get(), settings));
        }

        json retjson;
        if (settings.SaveObjectType)
        {
            retjson = json::object();

            Type* itemType = nullptr;

            if (fieldInfo)
            {
                auto attr = fieldInfo->GetAttribute<SerializableArrayAttribute>();
                if (attr)
                {
                    itemType = attr->GetItemType();
                }
            }
            if (!itemType)
            {
                itemType = list->GetIListElementType();
            }

            retjson["$type"] = util::GetSerializableTypeName(list->GetType());
            retjson["$values"] = arr_json;
        }
        else
        {
            retjson = std::move(arr_json);
        }

        return retjson;
    }

    static json _SerializeClassObject(Object* obj, const JsonSerializerSettings& settings)
    {
        json obj_js = json::object();

        if (settings.SaveObjectType)
        {
            obj_js["$type"] = util::GetSerializableTypeName(obj->GetType());
        }

        for (FieldInfo* info : obj->GetType()->GetFieldInfos(TypeBinding::NonPublic))
        {
            Object_sp field_inst = info->GetValue(obj);
            obj_js[info->GetName()] = _SerializeObject(field_inst.get(), settings);
        }

        return obj_js;
    }


    string JsonSerializer::Serialize(Object* obj, const JsonSerializerSettings& settings)
    {
        using namespace nlohmann;
        json js = _SerializeObject(obj, settings);
        return js.dump(settings.IndentSpace);
    }



    template<typename T>
    static bool _GetPrimitiveValue(const json& js, Type* type, Object_sp* out_obj)
    {
        if (type == cltypeof<T>())
        {
            *out_obj = mksptr((Object*)(new T{ js.get<typename T::unboxing_type>() }));
            return true;
        }
        return false;
    }

    static Object_sp _GetPrimitiveValue(const json& js, Type* field_type)
    {
        if (js.is_boolean()) return BoxUtil::Box(js.get<bool>());
        if (js.is_number_integer())
        {
            Object_sp obj = nullptr;
            auto r =
                _GetPrimitiveValue<Integer32>(js, field_type, &obj) ||
                _GetPrimitiveValue<Integer64>(js, field_type, &obj) ||
                _GetPrimitiveValue<Integer16>(js, field_type, &obj) ||
                _GetPrimitiveValue<Integer8>(js, field_type, &obj) ||
                _GetPrimitiveValue<UInteger32>(js, field_type, &obj) ||
                _GetPrimitiveValue<UInteger64>(js, field_type, &obj) ||
                _GetPrimitiveValue<UInteger16>(js, field_type, &obj) ||
                _GetPrimitiveValue<UInteger8>(js, field_type, &obj);
            return obj;
        }

        if (js.is_number_float())
        {
            Object_sp obj = nullptr;
            auto r =
                _GetPrimitiveValue<Single32>(js, field_type, &obj) ||
                _GetPrimitiveValue<Double64>(js, field_type, &obj);
            return obj;
        }
        if (js.is_string())
        {
            return BoxUtil::Box(js.get<string>());
        }
        return nullptr;
    }


    static Object_sp _DeserializeObject(const json& js, Type* type, FieldInfo* fieldInfo = nullptr);

    static Object_sp _DeserializeArray(const json& js, Type* type, FieldInfo* fieldInfo = nullptr)
    {
        Type* realListType = type;
        bool isSavedType = js.contains("$type");
        if (isSavedType)
        {
            realListType = util::GetSerializableType(js["$type"]);
        }

        Object_sp list_sp = realListType->CreateSharedInstance({});
        IList* list = interface_cast<IList>(list_sp.get());
        Type* elementType = nullptr;
        
        if (fieldInfo != nullptr)
        {
            auto attr = fieldInfo->GetAttribute<SerializableArrayAttribute>();
            if (attr)
            {
                elementType = attr->GetItemType();
            }
        }
        if(!elementType)
        {
            elementType = list->GetIListElementType();
        }

        json arr_json = js;
        if (isSavedType)
        {
            arr_json = js["$values"];
        }

        if (arr_json.is_array())
        {
            for (auto& item : arr_json)
            {
                list->Add(_DeserializeObject(item, elementType));
            }
        }

        return list_sp;
    }
    
    static Object_sp _DeserializeClassObject(const json& js, Type* type)
    {
        Type* realtype = type;
        if (js.contains("$type"))
        {
            realtype = util::GetSerializableType(js["$type"]);
        }

        Object_sp obj = realtype->CreateSharedInstance({});

        for (auto fieldInfo : realtype->GetFieldInfos())
        {
            if (!js.contains(fieldInfo->GetName()))
            {
                continue;
            }
            auto& item = js[fieldInfo->GetName()];

            if (!item.empty())
            {
                fieldInfo->SetValue(obj.get(), _DeserializeObject(item, fieldInfo->GetFieldType(), fieldInfo));
            }
        }

        return obj;
    }

    static Object_sp _DeserializeEnum(const json& js, Type* type)
    {
        Enum_sp ptr = sptr_cast<Enum>(type->CreateSharedInstance({}));
        uint32_t value = 0;
        if (js.is_string())
        {
            const string& name = js.get<string>();
            Enum::StaticTryParse(type, name, &value);
        }
        else if (js.is_number())
        {
            value = js.get<uint32_t>();
        }

        ptr->SetValue(value);
        return ptr;
    }

    static Object_sp _DeserializeObject(const json& js, Type* type, FieldInfo* fieldInfo)
    {
        if (type->IsPrimitiveType())
        {
            return _GetPrimitiveValue(js, type);
        }

        if (type->IsEnum())
        {
            return _DeserializeEnum(js, type);
        }

        if (type->IsImplementedInterface(cltypeof<IStringify>()))
        {
            if (!js.is_string()) return nullptr;
            sptr<Object> obj = type->CreateSharedInstance({});

            interface_cast<IStringify>(obj.get())->IStringify_Parse(js.get<string>());
            return obj;
        }


        if (type->IsImplementedInterface(cltypeof<IList>()))
        {
            return _DeserializeArray(js, type, fieldInfo);
        }

        //other
        return _DeserializeClassObject(js, type);
    }


    sptr<Object> JsonSerializer::Deserialize(const string& jstr, Type* type)
    {
        return _DeserializeObject(nlohmann::json::parse(jstr), type);
    }

}
namespace jxcorlib::ser::util
{
    string GetSerializableTypeName(Type* type)
    {
        return type->GetName() + "," + type->GetAssembly()->GetName();
    }

    Type* GetSerializableType(string_view name)
    {
        auto split = StringUtil::Split(name, ',');
        Assembly* assmbly = AssemblyManager::FindAssemblyByName(split[1]);
        Type* type = assmbly->FindType(split[0]);
        return type;
    }

}
