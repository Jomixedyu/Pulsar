#include <CoreLib/Delegate.h>
#include <CoreLib/Object.h>
#include <cassert>
#include <iostream>

using namespace jxcorlib;

class TestDelgClass : public Object
{
    CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, TestDelgClass, Object);

public:
    int Get(int i)
    {
        return i * 2;
    }
    int Getd(int i)
    {
        return i * 2;
    }
    int Get2()
    {
        return 2;
    }
};

static int g_delegate_check = -1;

void TestDelegate()
{
    //raw function
    {
        auto deleg = FunctionDelegate<int>::FromRaw([]() -> int { return 1; });
        int invoke_ret = deleg->Invoke();
        assert(invoke_ret == 1);

        auto dyninvoke_ret = deleg->DynamicInvoke({ });
        assert(UnboxUtil::Unbox<int>(dyninvoke_ret) == 1);
    }
    {
        auto deleg = FunctionDelegate<void, int>::FromRaw([](int i) { g_delegate_check = i; });
        deleg->Invoke(3);
        assert(g_delegate_check == 3);
        g_delegate_check = -1;

        auto dyninvoke_ret = deleg->DynamicInvoke({ mkbox(3) });
        assert(g_delegate_check == 3);
        g_delegate_check = -1;
    }
    {
        auto deleg = FunctionDelegate<string, int, string>::FromRaw([](int i, string str)->string { return str + std::to_string(i); });
        auto ret = deleg->Invoke(1, "ASD");
        assert(ret == "ASD1");

        auto dyninvoke_ret = deleg->DynamicInvoke({ mkbox(1), mkbox(string("ASD"))});
        assert(UnboxUtil::Unbox<string>(dyninvoke_ret) == "ASD1");
    }

    //member
    {
        auto inst = mksptr(new TestDelgClass);
        auto deleg2 = FunctionDelegate<int, int>::FromMember(inst, &TestDelgClass::Get);
        int result2 = deleg2->Invoke(3);
        assert(result2 == 6);

        assert(deleg2->EqualsSptr(FunctionDelegate<int, int>::FromMember(inst, &TestDelgClass::Get)));
    }




}