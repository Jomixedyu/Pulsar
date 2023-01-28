#pragma once
#include <ApatiteEd/Assembly.h>

#define APATITEED_PROPERTY_CONTROL(TYPE, INST) \
static inline struct __aped_prop_init { \
    __aped_prop_init() { PropertyControlManager::RegisterControl(TYPE, INST); } \
} __aped_prop_init_;


namespace apatiteed
{
    class PropertyControl
    {
    public:
        virtual void OnDrawImGui(const string& name, sptr<Object> prop) = 0;
        virtual Type* GetPropertyType() = 0;
    };

    class PropertyControlManager
    {
    public:
        static void RegisterControl(Type* type, PropertyControl* control);
        static PropertyControl* FindControl(Type* type);

        static void ShowProperty(const string& name, const sptr<Object>& obj);
        
    };
}