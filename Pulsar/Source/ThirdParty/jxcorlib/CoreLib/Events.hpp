/*
* @Moudule     : Events
* @Date        : 2021/05/06
* @Author      : JomiXedYu
* @Description : This file is part of jxcorlib : https://github.com/JomiXedYu/jxcorlib
* @StdRequired : c++11
*/

#pragma once

#include <list>
#include <functional>

namespace jxcorlib
{
    template<typename TReturn, typename... TArgs>
    class Events
    {
    public:
        using FunctionType = std::function<TReturn(TArgs...)>;
        using FunctionPointer = TReturn(*)(TArgs...);

    protected:
        enum class FunctionInfoType
        {
            Static,
            Lambda,
            Member
        };
        class FunctionInfo
        {
        public:
            unsigned int index;
            FunctionInfoType type;
        public:
            FunctionInfo(const unsigned int& index, const FunctionInfoType& type)
                : index(index), type(type)
            {
            }
        public:
            bool operator ==(const FunctionInfo& r) {
                return this->index == r.index;
            }
            virtual TReturn Invoke(TArgs...) = 0;
        };
        class StaticFunctionInfo : public FunctionInfo
        {
        public:
            FunctionPointer ptr;
            StaticFunctionInfo(const unsigned int& index, FunctionPointer ptr)
                : FunctionInfo(index, FunctionInfoType::Static), ptr(ptr)
            {
            }
            virtual TReturn Invoke(TArgs... args) override {
                return (*ptr)(args...);
            }
        };
        class LambdaFunctionInfo : public FunctionInfo
        {
        public:
            FunctionType func;
            void* instance;
            LambdaFunctionInfo(
                const unsigned int& index,
                void* instance,
                const FunctionType& func)
                : FunctionInfo(index, FunctionInfoType::Lambda), instance(instance), func(func)
            {
            }
            virtual TReturn Invoke(TArgs... args) override {
                return func(args...);
            }
        };
        template<typename TObj>
        class MemberFunctionInfo : public FunctionInfo
        {
        public:
            TObj* instance;
            TReturn(TObj::* ptr)(TArgs...);

            MemberFunctionInfo(
                const unsigned int& index,
                TObj* instance,
                TReturn(TObj::* ptr)(TArgs...))
                : FunctionInfo(index, FunctionInfoType::Member), instance(instance), ptr(ptr)
            {
            }
            virtual TReturn Invoke(TArgs... args) override {
                return (instance->*ptr)(args...);
            }
        };

    protected:
        unsigned int index;
        std::list<FunctionInfo*> event_list_;
    public:
        int Count() const {
            return this->event_list_.size();
        }
    public:
        Events() : index(0) {}
        Events(const Events& right) = delete;
        Events(Events&& right) = delete;
        ~Events() {
            this->RemoveAllListener();
        }
    protected:
        void RemoveAllListener() {
            for (auto it = this->event_list_.begin(); it != this->event_list_.end(); it++) {
                delete* it;
            }
            this->event_list_.clear();
        }
    public:
        //static
        unsigned int AddListener(FunctionPointer funcPtr) {
            if (funcPtr == nullptr) {
                return 0;
            }
            this->event_list_.push_back(new StaticFunctionInfo(++this->index, funcPtr));
            return this->index;
        }
        //member
        template<typename TObj>
        unsigned int AddListener(TObj* obj, TReturn(TObj::* ptr)(TArgs...)) {
            if (obj == nullptr) {
                return 0;
            }
            this->event_list_.push_back(new MemberFunctionInfo<TObj>(++this->index, obj, ptr));
            return this->index;
        }
        //lambda
        template<typename TObj>
        unsigned int AddListener(TObj* obj, const FunctionType& func) {
            this->event_list_.push_back(new LambdaFunctionInfo(++this->index, obj, func));
            return this->index;
        }

        //static
        unsigned int RemoveListener(FunctionPointer funcPtr) {
            for (auto it = this->event_list_.rbegin(); it != this->event_list_.rend(); it++) {
                if ((*it)->type == FunctionInfoType::Static
                    && static_cast<StaticFunctionInfo*>(*it)->ptr == funcPtr) {
                    auto index = (*it)->index;
                    delete* it;
                    this->event_list_.erase((++it).base());
                    return index;
                }
            }
            return 0;
        }
        //member
        template<typename TObj>
        unsigned int RemoveListener(TObj* obj, TReturn(TObj::* ptr)(TArgs...)) {
            for (auto it = this->event_list_.rbegin(); it != this->event_list_.rend(); it++)
            {
                if ((*it)->type == FunctionInfoType::Member
                    && static_cast<MemberFunctionInfo<TObj>*>(*it)->instance == obj
                    && static_cast<MemberFunctionInfo<TObj>*>(*it)->ptr == ptr)
                {
                    auto index = (*it)->index;
                    delete* it;
                    this->event_list_.erase((++it).base());
                    return index;
                }
            }
            return 0;
        }


        unsigned int RemoveListenerByIndex(unsigned int index) {
            if (index <= 0) {
                return 0;
            }
            for (auto it = this->event_list_.rbegin(); it != this->event_list_.rend(); it++) {
                if ((*it)->index == index) {
                    delete* it;
                    this->event_list_.erase((++it).base());
                    return index;
                }
            }
            return 0;
        }

        //member lambda
        template<typename TObj>
        void RemoveListenerByInstance(TObj* obj) {
            for (auto it = this->event_list_.begin(); it != this->event_list_.end(); ) {
                if ((
                    (*it)->type == FunctionInfoType::Member
                    && (static_cast<MemberFunctionInfo<TObj>*>(*it))->instance == obj
                    ) || (
                    (*it)->type == FunctionInfoType::Lambda
                    && static_cast<LambdaFunctionInfo*>(*it)->instance == obj
                    )
                    )
                {
                    delete* it;
                    it = this->event_list_.erase(it);
                }
                else {
                    it++;
                }
            }
        }

        unsigned int operator+=(FunctionPointer ptr) {
            return this->AddListener(ptr);
        }

        unsigned int operator-=(FunctionPointer ptr) {
            return this->RemoveListener(ptr);
        }
    };

    template<typename TReturn, typename... TArgs>
    class DelegateBase : public Events<TReturn, TArgs...>
    {
        using base = Events<TReturn, TArgs...>;
    public:
        void Invoke(TArgs... t) {
            for (auto& item : this->event_list_) {
                item->Invoke(t...);
            }
        }

        void RemoveAllListener() {
            base::RemoveAllListener();
        }
    };

    template<typename... TArgs>
    using ActionEvents = Events<void, TArgs...>;

    template<typename... TArgs>
    using Action = DelegateBase<void, TArgs...>;

    template<typename TReturn, typename... TArgs>
    using FunctionEvents = Events<TReturn, TArgs...>;

    template<typename TReturn, typename... TArgs>
    class Function : public DelegateBase<TReturn, TArgs...>
    {
    public:
        std::vector<TReturn> InvokeResult(TArgs... args) {
            std::vector<TReturn> retList;
            for (auto& item : this->event_list_) {
                retList.push_back(item->Invoke(args...));
            }
            return retList;
        }
    };

    template<>
    class Function<bool> : public DelegateBase<bool>
    {
    public:
        std::vector<bool> InvokeResult() {
            std::vector<bool> retList;
            for (auto& item : this->event_list_) {
                retList.push_back(item->Invoke());
            }
            return retList;
        }

        bool IsValidReturnInvoke() {
            for (const bool& item : this->InvokeResult()) {
                if (!item) return false;
            }
            return true;
        }
    };

}
