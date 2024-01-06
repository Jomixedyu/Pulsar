#pragma once
#include <PulsarEd/Assembly.h>

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
        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop) = 0;
        virtual Type* GetPropertyType() = 0;
    };

    class PropertyControlManager
    {
    public:
        static void RegisterControl(Type* type, PropertyControl* control);
        static PropertyControl* FindControl(Type* type);

        static bool ShowProperty(const string& name, Type* type, Object* obj);
        
    };


    namespace PImGui
    {
        // collasping header
        bool PropertyGroup(const char* label);

        // bool BeginPropertyItem(const char* name);
        //
        // void EndPropertyItem();

        bool BeginPropertyLine();
        bool PropertyLine(const string& name, Type* type, Object* obj);
        void PropertyLineText(const string& name, string_view str);
        void EndPropertyLine();

        // object field properties
        void ObjectFieldProperties(Type* type, Type* inner, Object* obj, ObjectBase* receiver, bool showDebug = false);
    }

}