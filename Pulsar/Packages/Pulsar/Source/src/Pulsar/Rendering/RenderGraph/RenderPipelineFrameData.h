#pragma once
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace pulsar
{
    class RenderFrameData
    {
    public:
        template<typename T>
        T* Get()
        {
            auto it = m_items.find(std::type_index(typeid(T)));
            return it == m_items.end() ? nullptr : static_cast<T*>(it->second.get());
        }

        template<typename T>
        const T* Get() const
        {
            auto it = m_items.find(std::type_index(typeid(T)));
            return it == m_items.end() ? nullptr : static_cast<const T*>(it->second.get());
        }

        template<typename T>
        T& GetOrCreate()
        {
            auto key = std::type_index(typeid(T));
            auto it = m_items.find(key);
            if (it == m_items.end())
            {
                auto item = MakeItem<T>();
                auto* value = static_cast<T*>(item.get());
                m_items.emplace(key, std::move(item));
                return *value;
            }
            return *static_cast<T*>(it->second.get());
        }

        template<typename T>
        T& Set(T data)
        {
            auto item = MakeItem<T>(std::move(data));
            auto* value = static_cast<T*>(item.get());
            m_items.insert_or_assign(std::type_index(typeid(T)), std::move(item));
            return *value;
        }

    private:
        template<typename T, typename... TArgs>
        static std::unique_ptr<void, void(*)(void*)> MakeItem(TArgs&&... args)
        {
            return {
                new T(std::forward<TArgs>(args)...),
                [](void* pointer) { delete static_cast<T*>(pointer); }
            };
        }

        std::unordered_map<std::type_index, std::unique_ptr<void, void(*)(void*)>> m_items;
    };
}