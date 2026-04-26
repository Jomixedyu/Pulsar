#pragma once
#include <PulsarEd/Assembly.h>
#include <span>

#define EDITOR_IMPL_PROPERTY_CONTROL(TYPE, INST) \
static inline struct __aped_prop_init { \
    __aped_prop_init() { PropertyControlManager::RegisterControl(TYPE, INST); } \
} __aped_prop_init_; \
public: virtual Type* GetPropertyType() override { \
    return TYPE; \
}


namespace pulsared
{
    class PropertyControl
    {
    public:
        virtual ~PropertyControl() = default;
        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop, std::span<Attribute*> attrs = {}) = 0;
        virtual Type* GetPropertyType() = 0;
    };

    class PropertyControlManager
    {
    public:
        static void RegisterControl(Type* type, PropertyControl* control);
        static PropertyControl* FindControl(Type* type);

        static bool ShowProperty(const string& name, Type* type, Object* obj, std::span<Attribute*> attrs = {});
        
    };


    namespace PImGui
    {
        bool PropertyGroup(const char* label);
        bool BeginPropertyLines();
        bool PropertyLine(const string& name, Type* type, Object* obj, std::span<Attribute*> attrs = {});
        void PropertyLineText(const string& name, string_view str);
        void EndPropertyLines();

        // object field properties
        bool ObjectFieldProperties(Type* type, Type* inner, Object* obj, Object* receiver, bool showDebug = false);
    }

}