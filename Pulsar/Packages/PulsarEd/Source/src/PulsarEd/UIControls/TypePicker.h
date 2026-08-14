#pragma once
#include <PulsarEd/Assembly.h>
#include <functional>

namespace pulsared
{
    struct TypePickerOptions
    {
        Type* BaseType = nullptr;
        bool IncludeBaseType = false;
        bool IncludeAbstractTypes = false;
        std::function<bool(Type*)> Filter;
    };

    class TypePickerPopup
    {
    public:
        static void Open(const char* popupId);
        static bool Draw(const char* popupId, const TypePickerOptions& options, Type*& outSelectedType);
    };
}