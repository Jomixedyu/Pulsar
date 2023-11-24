#include "Assembly.h"
#include <cassert>
#include <functional>
#include <map>
#include <iostream>
#include <CoreLib/Delegate.h>

using namespace jxcorlib;

namespace space
{
    class DynCreateClass : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_Test, space::DynCreateClass, Object);
        CORELIB_DECL_DYNCINST() {
            if (!params.Check<int>()) {
                return nullptr;
            }
            int p1 = params.Get<int>(0);
            return new DynCreateClass(p1);
        }
    private:
        int id;
    public:
        DynCreateClass(int id) : id(id) {}
    };
}


struct MethodReflectionGenerator
{
    template<typename R>
    static std::function<Object_sp(array_list<Object_sp>&&)> Generate(R(*ptr)())
    {
        return [ptr](array_list<Object_sp>&& objs) -> Object_sp {
            auto ret = ptr();
            if (cltype_concept<R>)
            {

            }
            return BoxUtil::Box(ret);
        };
    }

    template<typename R, typename P1>
    static std::function<Object_sp(array_list<Object_sp>&&)> Generate(R(*ptr)(P1))
    {
        return [ptr](array_list<Object_sp>&& objs) -> Object_sp {
            auto ret = ptr(UnboxUtil::Unbox<P1>(objs[0]));
            return BoxUtil::Box(ret);
        };
    }

    template<typename R, typename P1, typename P2>
    static auto Generate(R(*ptr)(P1, P2))
    {
        return FunctionDelegate<R, P1, P2>(ptr);
    }

};

class DataModel : public Object
{
    CORELIB_DEF_TYPE(AssemblyObject_Test, DataModel, Object);

private:
    CORELIB_REFL_DECL_FIELD(id);
    int id = 0;
public:
    CORELIB_REFL_DECL_FIELD(is_human);
    bool is_human = true;

    CORELIB_REFL_DECL_FIELD(name);
    sptr<Object> name;

    CORELIB_REFL_DECL_STATICMETHOD(Add);
    static int Add(int32_t a, int64_t b) { return a + int(b); }

    CORELIB_REFL_DECL_METHOD(Minus);
    int Minus(int32_t a, int64_t b) { return a - int(b); }

};



void TestReflection()
{
    sptr<Object> n;

    //dynamic create
    Type* dyn_type = AssemblyManager::FindAssemblyByName(AssemblyObject_Test.name())->FindType("space::DynCreateClass");
    auto dyn = dyn_type->CreateInstance(ParameterPackage{ 20 });

    assert(dyn->GetType() == cltypeof<space::DynCreateClass>());

    //field reflection
    DataModel* model = new DataModel;

    Type* model_type = cltypeof<DataModel>();

    //id : const int
    FieldInfo* id_field = model_type->GetFieldInfo("id");
    assert(id_field->IsPublic() == true);
    assert(id_field->IsConst() == false);
    assert(id_field->IsPointer() == false);

    assert(id_field->GetName() == "id");

    id_field->SetValue(model, mksptr(new Integer32{ 3 }));

    Object_sp id_value = id_field->GetValue(model);
    assert(id_value->GetType() == cltypeof<get_boxing_type<int>::type>());
    assert(*(Integer32*)id_value.get() == 3);

    //name : Object*
    FieldInfo* name_field = model_type->GetFieldInfo("name");

    Object_sp obj = mksptr(new Object());
    name_field->SetValue(model, obj);

    auto value = name_field->GetValue(model);
    assert(value == obj);


    MethodInfo* minfo = cltypeof<DataModel>()->GetMethodInfo("Add");
    assert(minfo->GetParameterInfos().at(0)->GetParamType() == cltypeof<Integer32>());
    assert(minfo->GetParameterInfos().at(1)->GetParamType() == cltypeof<Integer64>());

    auto minfoRet1 = minfo->Invoke(nullptr, { mkbox(2), mkbox(3) });
    assert(UnboxUtil::Unbox<int>(minfoRet1) == 5);

    auto minfoRet2 = cltypeof<DataModel>()->GetMethodInfo("Minus")->Invoke(mksptr(new DataModel), {mkbox(2), mkbox(5)});
    assert(UnboxUtil::Unbox<int>(minfoRet2) == -3);
}