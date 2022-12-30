#pragma once
#include "Assembly.h"
#include "Object.h"
#include "Type.h"
#include "BasicTypes.h"
#include <functional>
#include "CommonException.h"

namespace jxcorlib
{
    class Delegate : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::Delegate, Object);

    public:
        virtual Object_sp DynamicInvoke(const array_list<Object_sp>& params) = 0;
    };

#define _CORELIB_DELEGATE_GETVALUE(TPi, index) UnboxUtil::TryUnbox<typename get_boxing_type<TPi>::type>(params[index])

#define _CORELIB_DELEGATE_INVOKE1 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0))
#define _CORELIB_DELEGATE_INVOKE2 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1))
#define _CORELIB_DELEGATE_INVOKE3 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1), _CORELIB_DELEGATE_GETVALUE(T3, 2))
#define _CORELIB_DELEGATE_INVOKE4 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1), _CORELIB_DELEGATE_GETVALUE(T3, 2), _CORELIB_DELEGATE_GETVALUE(T4, 3))
#define _CORELIB_DELEGATE_INVOKE5 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1), _CORELIB_DELEGATE_GETVALUE(T3, 2), _CORELIB_DELEGATE_GETVALUE(T4, 3), _CORELIB_DELEGATE_GETVALUE(T5, 4))
#define _CORELIB_DELEGATE_INVOKE6 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1), _CORELIB_DELEGATE_GETVALUE(T3, 2), _CORELIB_DELEGATE_GETVALUE(T4, 3), _CORELIB_DELEGATE_GETVALUE(T5, 4), _CORELIB_DELEGATE_GETVALUE(T6, 5))
#define _CORELIB_DELEGATE_INVOKE7 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1), _CORELIB_DELEGATE_GETVALUE(T3, 2), _CORELIB_DELEGATE_GETVALUE(T4, 3), _CORELIB_DELEGATE_GETVALUE(T5, 4), _CORELIB_DELEGATE_GETVALUE(T6, 5), _CORELIB_DELEGATE_GETVALUE(T7, 6))
#define _CORELIB_DELEGATE_INVOKE8 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1), _CORELIB_DELEGATE_GETVALUE(T3, 2), _CORELIB_DELEGATE_GETVALUE(T4, 3), _CORELIB_DELEGATE_GETVALUE(T5, 4), _CORELIB_DELEGATE_GETVALUE(T6, 5), _CORELIB_DELEGATE_GETVALUE(T7, 6), _CORELIB_DELEGATE_GETVALUE(T8,7))
#define _CORELIB_DELEGATE_INVOKE9 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1), _CORELIB_DELEGATE_GETVALUE(T3, 2), _CORELIB_DELEGATE_GETVALUE(T4, 3), _CORELIB_DELEGATE_GETVALUE(T5, 4), _CORELIB_DELEGATE_GETVALUE(T6, 5), _CORELIB_DELEGATE_GETVALUE(T7, 6), _CORELIB_DELEGATE_GETVALUE(T8,7), _CORELIB_DELEGATE_GETVALUE(T9,8))
#define _CORELIB_DELEGATE_INVOKE10 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1), _CORELIB_DELEGATE_GETVALUE(T3, 2), _CORELIB_DELEGATE_GETVALUE(T4, 3), _CORELIB_DELEGATE_GETVALUE(T5, 4), _CORELIB_DELEGATE_GETVALUE(T6, 5), _CORELIB_DELEGATE_GETVALUE(T7, 6), _CORELIB_DELEGATE_GETVALUE(T8,7), _CORELIB_DELEGATE_GETVALUE(T9,8), _CORELIB_DELEGATE_GETVALUE(T10,9))
#define _CORELIB_DELEGATE_INVOKE11 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1), _CORELIB_DELEGATE_GETVALUE(T3, 2), _CORELIB_DELEGATE_GETVALUE(T4, 3), _CORELIB_DELEGATE_GETVALUE(T5, 4), _CORELIB_DELEGATE_GETVALUE(T6, 5), _CORELIB_DELEGATE_GETVALUE(T7, 6), _CORELIB_DELEGATE_GETVALUE(T8,7), _CORELIB_DELEGATE_GETVALUE(T9,8), _CORELIB_DELEGATE_GETVALUE(T10,9), _CORELIB_DELEGATE_GETVALUE(T11,10))
#define _CORELIB_DELEGATE_INVOKE12 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1), _CORELIB_DELEGATE_GETVALUE(T3, 2), _CORELIB_DELEGATE_GETVALUE(T4, 3), _CORELIB_DELEGATE_GETVALUE(T5, 4), _CORELIB_DELEGATE_GETVALUE(T6, 5), _CORELIB_DELEGATE_GETVALUE(T7, 6), _CORELIB_DELEGATE_GETVALUE(T8,7), _CORELIB_DELEGATE_GETVALUE(T9,8), _CORELIB_DELEGATE_GETVALUE(T10,9), _CORELIB_DELEGATE_GETVALUE(T11,10), _CORELIB_DELEGATE_GETVALUE(T12,11))
#define _CORELIB_DELEGATE_INVOKE13 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1), _CORELIB_DELEGATE_GETVALUE(T3, 2), _CORELIB_DELEGATE_GETVALUE(T4, 3), _CORELIB_DELEGATE_GETVALUE(T5, 4), _CORELIB_DELEGATE_GETVALUE(T6, 5), _CORELIB_DELEGATE_GETVALUE(T7, 6), _CORELIB_DELEGATE_GETVALUE(T8,7), _CORELIB_DELEGATE_GETVALUE(T9,8), _CORELIB_DELEGATE_GETVALUE(T10,9), _CORELIB_DELEGATE_GETVALUE(T11,10), _CORELIB_DELEGATE_GETVALUE(T12,11), _CORELIB_DELEGATE_GETVALUE(T13,12))
#define _CORELIB_DELEGATE_INVOKE14 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1), _CORELIB_DELEGATE_GETVALUE(T3, 2), _CORELIB_DELEGATE_GETVALUE(T4, 3), _CORELIB_DELEGATE_GETVALUE(T5, 4), _CORELIB_DELEGATE_GETVALUE(T6, 5), _CORELIB_DELEGATE_GETVALUE(T7, 6), _CORELIB_DELEGATE_GETVALUE(T8,7), _CORELIB_DELEGATE_GETVALUE(T9,8), _CORELIB_DELEGATE_GETVALUE(T10,9), _CORELIB_DELEGATE_GETVALUE(T11,10), _CORELIB_DELEGATE_GETVALUE(T12,11), _CORELIB_DELEGATE_GETVALUE(T13,12), _CORELIB_DELEGATE_GETVALUE(T14,13))
#define _CORELIB_DELEGATE_INVOKE15 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1), _CORELIB_DELEGATE_GETVALUE(T3, 2), _CORELIB_DELEGATE_GETVALUE(T4, 3), _CORELIB_DELEGATE_GETVALUE(T5, 4), _CORELIB_DELEGATE_GETVALUE(T6, 5), _CORELIB_DELEGATE_GETVALUE(T7, 6), _CORELIB_DELEGATE_GETVALUE(T8,7), _CORELIB_DELEGATE_GETVALUE(T9,8), _CORELIB_DELEGATE_GETVALUE(T10,9), _CORELIB_DELEGATE_GETVALUE(T11,10), _CORELIB_DELEGATE_GETVALUE(T12,11), _CORELIB_DELEGATE_GETVALUE(T13,12), _CORELIB_DELEGATE_GETVALUE(T14,13), _CORELIB_DELEGATE_GETVALUE(T15,14))
#define _CORELIB_DELEGATE_INVOKE16 (inst->*func)(_CORELIB_DELEGATE_GETVALUE(T1, 0), _CORELIB_DELEGATE_GETVALUE(T2, 1), _CORELIB_DELEGATE_GETVALUE(T3, 2), _CORELIB_DELEGATE_GETVALUE(T4, 3), _CORELIB_DELEGATE_GETVALUE(T5, 4), _CORELIB_DELEGATE_GETVALUE(T6, 5), _CORELIB_DELEGATE_GETVALUE(T7, 6), _CORELIB_DELEGATE_GETVALUE(T8,7), _CORELIB_DELEGATE_GETVALUE(T9,8), _CORELIB_DELEGATE_GETVALUE(T10,9), _CORELIB_DELEGATE_GETVALUE(T11,10), _CORELIB_DELEGATE_GETVALUE(T12,11), _CORELIB_DELEGATE_GETVALUE(T13,12), _CORELIB_DELEGATE_GETVALUE(T14,13), _CORELIB_DELEGATE_GETVALUE(T15,14), _CORELIB_DELEGATE_GETVALUE(T16,15))

    template<typename T, typename TReturn>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(), const array_list<Object_sp>& params)
    {
        assert(params.size() == 0);
        if constexpr (std::is_same_v<TReturn, void>) { (inst->*func)(); return nullptr; }
        else return BoxUtil::Box<TReturn>((inst->*func)());
    }

    template<typename T, typename TReturn, typename T1>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1), const array_list<Object_sp>& params)
    {
        assert(params.size() == 1);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE1; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE1);
    }
    template<typename T, typename TReturn, typename T1, typename T2>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2), const array_list<Object_sp>& params)
    {
        assert(params.size() == 2);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE2; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE2);
    }
    template<typename T, typename TReturn, typename T1, typename T2, typename T3>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2, T3), const array_list<Object_sp>& params)
    {
        assert(params.size() == 3);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE3; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE3);
    }
    template<typename T, typename TReturn, typename T1, typename T2, typename T3, typename T4>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2, T3, T4), const array_list<Object_sp>& params)
    {
        assert(params.size() == 4);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE4; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE4);
    }
    template<typename T, typename TReturn, typename T1, typename T2, typename T3, typename T4, typename T5>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2, T3, T4, T5), const array_list<Object_sp>& params)
    {
        assert(params.size() == 5);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE5; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE5);
    }
    template<typename T, typename TReturn, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2, T3, T4, T5, T6), const array_list<Object_sp>& params)
    {
        assert(params.size() == 6);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE6; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE6);
    }
    template<typename T, typename TReturn, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2, T3, T4, T5, T6, T7), const array_list<Object_sp>& params)
    {
        assert(params.size() == 7);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE7; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE7);
    }
    template<typename T, typename TReturn, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2, T3, T4, T5, T6, T7, T8), const array_list<Object_sp>& params)
    {
        assert(params.size() == 8);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE8; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE8);
    }
    template<typename T, typename TReturn, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2, T3, T4, T5, T6, T7, T8, T9), const array_list<Object_sp>& params)
    {
        assert(params.size() == 9);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE9; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE9);
    }
    template<typename T, typename TReturn, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10), const array_list<Object_sp>& params)
    {
        assert(params.size() == 10);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE10; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE10);
    }
    template<typename T, typename TReturn, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11), const array_list<Object_sp>& params)
    {
        assert(params.size() == 11);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE11; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE11);
    }
    template<typename T, typename TReturn, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12), const array_list<Object_sp>& params)
    {
        assert(params.size() == 12);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE12; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE12);
    }
    template<typename T, typename TReturn, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13), const array_list<Object_sp>& params)
    {
        assert(params.size() == 13);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE13; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE13);
    }
    template<typename T, typename TReturn, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14), const array_list<Object_sp>& params)
    {
        assert(params.size() == 14);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE14; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE14);
    }
    template<typename T, typename TReturn, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15), const array_list<Object_sp>& params)
    {
        assert(params.size() == 15);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE15; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE15);
    }
    template<typename T, typename TReturn, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16>
    Object_sp InvokeInstanceFunction(T* inst, TReturn(T::* func)(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16), const array_list<Object_sp>& params)
    {
        assert(params.size() == 16);
        if constexpr (std::is_same_v<TReturn, void>) { _CORELIB_DELEGATE_INVOKE16; return nullptr; }
        else return BoxUtil::Box<TReturn>(_CORELIB_DELEGATE_INVOKE16);
    }
#undef _CORELIB_DELEGATE_GETVALUE
#undef _CORELIB_DELEGATE_INVOKE1
#undef _CORELIB_DELEGATE_INVOKE2
#undef _CORELIB_DELEGATE_INVOKE3
#undef _CORELIB_DELEGATE_INVOKE4
#undef _CORELIB_DELEGATE_INVOKE5
#undef _CORELIB_DELEGATE_INVOKE6
#undef _CORELIB_DELEGATE_INVOKE7
#undef _CORELIB_DELEGATE_INVOKE8
#undef _CORELIB_DELEGATE_INVOKE9
#undef _CORELIB_DELEGATE_INVOKE10
#undef _CORELIB_DELEGATE_INVOKE11
#undef _CORELIB_DELEGATE_INVOKE12
#undef _CORELIB_DELEGATE_INVOKE13
#undef _CORELIB_DELEGATE_INVOKE14
#undef _CORELIB_DELEGATE_INVOKE15
#undef _CORELIB_DELEGATE_INVOKE16

    template<typename TReturn, typename... TArgs>
    class FunctionDelegate : public Delegate
    {
        CORELIB_DEF_TEMPLATE_TYPE(AssemblyObject_jxcorlib, jxcorlib::FunctionDelegate, Delegate, TReturn, TArgs...);

        static_assert(sizeof...(TArgs) <= 16);

        using This = FunctionDelegate<TReturn, TArgs...>;
        using FunctionType = std::function<TReturn(TArgs...)>;
        using FunctionPointer = TReturn(*)(TArgs...);

        enum class FunctionInfoType
        {
            Static, Lambda, Member
        };

        class FunctionInfo
        {
        public:
            FunctionInfoType type;
        public:
            FunctionInfo(const FunctionInfoType& type) : type(type)
            {
            }
        public:
            virtual bool Equals(FunctionInfo* Func) const = 0;
            virtual TReturn Invoke(TArgs... args) = 0;
        };

        class StaticFunctionInfo : public FunctionInfo
        {
        public:
            FunctionPointer ptr_;
            StaticFunctionInfo(FunctionPointer ptr) : FunctionInfo(FunctionInfoType::Static), ptr_(ptr)
            {
            }
            virtual bool Equals(FunctionInfo* func) const override
            {
                if (func == nullptr || this->type != func->type) return false;
                return this->ptr_ == static_cast<StaticFunctionInfo*>(func)->ptr_;
            }
            virtual TReturn Invoke(TArgs... args) override
            {
                return (*this->ptr_)(args...);
            }
        };

        class LambdaFunctionInfo : public FunctionInfo
        {
        public:
            FunctionType func_;
            LambdaFunctionInfo(const FunctionType& func)
                : FunctionInfo(FunctionInfoType::Lambda), func_(func)
            {
            }
            virtual bool Equals(FunctionInfo* func) const override
            {
                if (func == nullptr || this->type != func->type) return false;
                return false;
                //return this->func_.target() == static_cast<LambdaFunctionInfo*>(func)->func_.target();
            }
            virtual TReturn Invoke(TArgs... args) override
            {
                return this->func_(args...);
            }
        };

        template<typename TObj>
        class MemberFunctionInfo : public FunctionInfo
        {
        public:
            sptr<TObj> instance_;
            TReturn(TObj::* ptr_)(TArgs...);

            MemberFunctionInfo(const sptr<TObj>& instance, TReturn(TObj::* ptr)(TArgs...))
                : FunctionInfo(FunctionInfoType::Member), instance_(instance), ptr_(ptr)
            {
            }
            virtual bool Equals(FunctionInfo* func) const override
            {
                if (func == nullptr || this->type != func->type) return false;
                return this->ptr_ == static_cast<MemberFunctionInfo*>(func)->ptr_;
            }
            virtual TReturn Invoke(TArgs... args) override
            {
                return (this->instance_.get()->*ptr_)(args...);
            }
        };

    protected:
        FunctionInfo* func_ptr_;

        FunctionDelegate(FunctionPointer funcptr) : func_ptr_(new StaticFunctionInfo(funcptr)) {}
        FunctionDelegate(const FunctionType& func) : func_ptr_(new LambdaFunctionInfo(func)) {}

        template<typename TObject>
        FunctionDelegate(const sptr<TObject>& obj, TReturn(TObject::* ptr)(TArgs...))
            : func_ptr_(new MemberFunctionInfo<TObject>(obj, ptr))
        {}
    public:

        static sptr<FunctionDelegate> FromRaw(FunctionPointer funcptr)
        {
            return mksptr(new FunctionDelegate(funcptr));
        }
        static sptr<FunctionDelegate> FromLambda(const FunctionType& func)
        {
            return mksptr(new FunctionDelegate(func));
        }

        template<typename TObject>
        static sptr<FunctionDelegate> FromMember(const sptr<TObject>& obj, TReturn(TObject::* ptr)(TArgs...))
        {
            return mksptr(new FunctionDelegate(obj, ptr));
        }

        FunctionDelegate() = delete;
        FunctionDelegate(const FunctionDelegate&) = delete;
        FunctionDelegate(FunctionDelegate&&) = delete;

        ~FunctionDelegate()
        {
            delete this->func_ptr_;
        }

        virtual bool Equals(Object* object) const override
        {
            if (object == nullptr || object->GetType() != this->GetType()) return false;
            return this->func_ptr_->Equals(static_cast<This*>(object)->func_ptr_);
        }
        TReturn Invoke(TArgs... args)
        {
            return this->func_ptr_->Invoke(args...);
        }
        virtual Object_sp DynamicInvoke(const array_list<Object_sp>& params) override
        {
            return InvokeInstanceFunction<FunctionInfo, TReturn, TArgs...>(this->func_ptr_, &FunctionInfo::Invoke, params);
        }
    };

}