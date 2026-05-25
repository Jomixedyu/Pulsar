#pragma once
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace gfx
{
    // Intrusive reference-counting smart pointer.
    // T must inherit from GFXResource (or provide AddRef/Release methods).
    // Zero overhead compared to std::shared_ptr (no control block, no weak refs).
    template<typename T>
    class GFXRefCountPtr
    {
        template<typename U>
        friend class GFXRefCountPtr;

        T* m_ptr = nullptr;

    public:
        GFXRefCountPtr() = default;

        ~GFXRefCountPtr()
        {
            Reset();
        }

        GFXRefCountPtr(std::nullptr_t)
        {
        }

        GFXRefCountPtr(T* ptr)
            : m_ptr(ptr)
        {
            if (m_ptr)
                m_ptr->AddRef();
        }

        GFXRefCountPtr(const GFXRefCountPtr& other)
            : m_ptr(other.m_ptr)
        {
            if (m_ptr)
                m_ptr->AddRef();
        }

        template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        GFXRefCountPtr(const GFXRefCountPtr<U>& other)
            : m_ptr(other.m_ptr)
        {
            if (m_ptr)
                m_ptr->AddRef();
        }

        GFXRefCountPtr(GFXRefCountPtr&& other) noexcept
            : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        GFXRefCountPtr(GFXRefCountPtr<U>&& other) noexcept
            : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        GFXRefCountPtr& operator=(std::nullptr_t)
        {
            Reset();
            return *this;
        }

        GFXRefCountPtr& operator=(T* ptr)
        {
            if (m_ptr != ptr)
            {
                if (m_ptr)
                    m_ptr->ReleaseRef();
                m_ptr = ptr;
                if (m_ptr)
                    m_ptr->AddRef();
            }
            return *this;
        }

        GFXRefCountPtr& operator=(const GFXRefCountPtr& other)
        {
            return *this = other.m_ptr;
        }

        template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        GFXRefCountPtr& operator=(const GFXRefCountPtr<U>& other)
        {
            return *this = other.m_ptr;
        }

        GFXRefCountPtr& operator=(GFXRefCountPtr&& other) noexcept
        {
            if (this != &other)
            {
                if (m_ptr)
                    m_ptr->ReleaseRef();
                m_ptr = other.m_ptr;
                other.m_ptr = nullptr;
            }
            return *this;
        }

        template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        GFXRefCountPtr& operator=(GFXRefCountPtr<U>&& other) noexcept
        {
            if (m_ptr != other.m_ptr)
            {
                if (m_ptr)
                    m_ptr->ReleaseRef();
                m_ptr = other.m_ptr;
                other.m_ptr = nullptr;
            }
            return *this;
        }

        void Reset()
        {
            if (m_ptr)
            {
                m_ptr->ReleaseRef();
                m_ptr = nullptr;
            }
        }

        T* Get() const
        {
            return m_ptr;
        }

        T* operator->() const
        {
            return m_ptr;
        }

        T& operator*() const
        {
            return *m_ptr;
        }

        explicit operator bool() const
        {
            return m_ptr != nullptr;
        }

        bool IsValid() const
        {
            return m_ptr != nullptr;
        }

        // Compatibility: implicit conversion to std::shared_ptr / std::weak_ptr.
        // This allows gradual migration from shared_ptr-based APIs.
        // TODO: Remove once all downstream code uses GFXRefCountPtr directly.
        operator std::shared_ptr<T>() const
        {
            if (m_ptr)
            {
                m_ptr->AddRef();
                return std::shared_ptr<T>(m_ptr, [](T* p) { if (p) p->ReleaseRef(); });
            }
            return std::shared_ptr<T>{};
        }

        operator std::weak_ptr<T>() const
        {
            return static_cast<std::shared_ptr<T>>(*this);
        }

        std::shared_ptr<T> Lock() const
        {
            return static_cast<std::shared_ptr<T>>(*this);
        }

        // Create from a raw pointer and increment the reference count.
        // Equivalent to GFXRefCountPtr(ptr) but makes factory return sites explicit.
        static GFXRefCountPtr FromOwned(T* ptr)
        {
            GFXRefCountPtr result;
            result.m_ptr = ptr;
            if (ptr)
                ptr->AddRef();
            return result;
        }
    };

    template<typename T, typename U>
    inline bool operator==(const GFXRefCountPtr<T>& a, const GFXRefCountPtr<U>& b)
    {
        return a.Get() == b.Get();
    }

    template<typename T, typename U>
    inline bool operator!=(const GFXRefCountPtr<T>& a, const GFXRefCountPtr<U>& b)
    {
        return a.Get() != b.Get();
    }

    template<typename T>
    inline bool operator==(const GFXRefCountPtr<T>& a, std::nullptr_t)
    {
        return a.Get() == nullptr;
    }

    template<typename T>
    inline bool operator!=(const GFXRefCountPtr<T>& a, std::nullptr_t)
    {
        return a.Get() != nullptr;
    }

} // namespace gfx
