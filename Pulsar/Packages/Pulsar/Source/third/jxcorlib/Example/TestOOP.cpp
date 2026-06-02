#include <CoreLib/CoreLib.h>
#include <iostream>
#include <cassert>
#include <CoreLib/List.h>
using namespace jxcorlib;



class ExampleClass : public Object
{
    CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, ExampleClass, Object);
public:
    CORELIB_REFL_DECL_FIELD(ilist);
    ArrayList_sp ilist;

    CORELIB_REFL_DECL_FIELD(slist);
    ArrayList_sp slist;
};
CORELIB_DECL_SHORTSPTR(ExampleClass);

namespace jxcorlib
{
    // template class List<int>;
    // template class List< SPtr<class ExampleClass> >;
}

template<typename T, typename K>
class TemplateClass : public Object
{
public:
    CORELIB_DEF_TEMPLATE_TYPE(AssemblyObject_jxcorlib, TemplateClass, Object, T, K);

};



void TestOOP()
{
    using namespace std;

    ExampleClass_sp exm = mksptr(new ExampleClass);

    ExampleClass_sp exm1 = mksptr(new ExampleClass);

    exm->ilist = mksptr(new ArrayList{ mkbox(2), mkbox(3) } );
    exm1->ilist = mksptr(new ArrayList{ mkbox(3), mkbox(4) });

    exm->slist = mksptr(new ArrayList{ exm1 });

    auto copy = sptr_cast<ExampleClass>(ObjectUtil::DeepCopyObject(exm));

    assert(exm->ilist->Equals(copy->ilist.get()));

    assert(exm->GetType()->GetBase() == cltypeof<Object>());
    assert(exm->GetType()->GetName() == string("ExampleClass"));

    assert(cltypeof<Object>()->IsInstanceOfType(exm.get()));

    assert(ExampleClass::StaticType() == cltypeof<ExampleClass>());

    assert(cltypeof<ExampleClass>()->IsSubclassOf(cltypeof<Object>()));

    auto* templc = new TemplateClass<int, double>;
    Type* templc_type = templc->GetType();

}