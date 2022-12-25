#include "JsonSerializer.h"
#include "json.hpp"
#include <CoreLib/Reflection.h>
#include <cassert>

namespace jxcorlib::ser
{
    using namespace nlohmann;
    using namespace jxcorlib;

    static json _SerializeArray(IList* list, const JsonSerializerSettings& settings);
    static json _SerializeClassObject(Object* obj, const JsonSerializerSettings& settings);


    static json _SerializeObject(Object* obj, const JsonSerializerSettings& settings)
    {
        if (obj == nullptr)
        {
            return json::object();
        }

        if (obj->GetType()->is_primitive_type())
        {
            json prim_js;
            PrimitiveObjectUtil::Assign(&prim_js, obj);
            return std::move(prim_js);
        }

        if (obj->GetType()->is_enum())
        {
            json enum_js;
            if (settings.string_enum)
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
            return _SerializeArray(list, settings);
        }

        //other
        return _SerializeClassObject(obj, settings);

    }

    static json _SerializeArray(IList* list, const JsonSerializerSettings& settings)
    {
        int32_t count = list->GetCount();
        json arr_json = json::array();

        for (int32_t i = 0; i < count; i++)
        {
            Object_rsp element = list->At(i);
            arr_json.push_back(_SerializeObject(element.get(), settings));
        }
        return arr_json;
    }

    static json _SerializeClassObject(Object* obj, const JsonSerializerSettings& settings)
    {
        json obj_js = json::object();

        for (FieldInfo* info : obj->GetType()->get_fieldinfos(TypeBinding::NonPublic))
        {
            Object_sp field_inst = info->GetValue(obj);
            obj_js[info->get_name()] = _SerializeObject(field_inst.get(), settings);
        }

        return obj_js;
    }


    string JsonSerializer::Serialize(Object* obj, const JsonSerializerSettings& settings)
    {
        using namespace nlohmann;
        json js = _SerializeObject(obj, settings);
        return js.dump(settings.indent_space);
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


    static Object_sp _DeserializeObject(const json& js, Type* type, sptr<Object> default_v = nullptr);

    static Object_sp _DeserializeArray(const json& js, Type* type)
    {
        Object_sp list_sp = type->CreateSharedInstance({});
        IList* list = interface_cast<IList>(list_sp.get());
        Type* element_type = list->GetIListElementType();

        if (js.is_array())
        {
            for (auto& item : js)
            {
                list->Add(_DeserializeObject(item, element_type));
            }
        }

        return list_sp;
    }
    
    static Object_sp _DeserializeClassObject(const json& js, Type* type, sptr<Object> default_v = nullptr)
    {
        Object_sp obj = default_v;
        if (default_v == nullptr)
        {
            obj = type->CreateSharedInstance({});
        }

        for (auto field_info : type->get_fieldinfos())
        {
            auto& item = js.at(field_info->get_name());
            if (!item.empty())
            {
                field_info->SetValue(obj.get(), _DeserializeObject(item, field_info->get_field_type()));
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

    static Object_sp _DeserializeObject(const json& js, Type* type, sptr<Object> default_v)
    {
        if (type->is_primitive_type())
        {
            return _GetPrimitiveValue(js, type);
        }

        if (type->is_enum())
        {
            return _DeserializeEnum(js, type);
        }

        if (type->IsImplementedInterface(cltypeof<IStringify>()))
        {
            if (!js.is_string()) return nullptr;
            sptr<Object> obj = default_v;
            if (default_v == nullptr)
            {
                obj = type->CreateSharedInstance({});
            }

            interface_cast<IStringify>(obj.get())->IStringify_Parse(js.get<string>());
            return obj;
        }


        if (type->IsImplementedInterface(cltypeof<IList>()))
        {
            return _DeserializeArray(js, type);
        }

        //other
        return _DeserializeClassObject(js, type, default_v);
    }


    sptr<Object> JsonSerializer::Deserialize(const string& jstr, Type* type, sptr<Object> default_v)
    {
        return _DeserializeObject(nlohmann::json::parse(jstr), type, default_v);
    }

}
