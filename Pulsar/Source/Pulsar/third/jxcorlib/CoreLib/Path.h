#pragma once
#include <filesystem>
#include "BasicTypes.h"

namespace jxcorlib
{
    using path = std::filesystem::path;

    class Path : public BoxingObject, public IStringify
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::Path, BoxingObject);
        CORELIB_IMPL_INTERFACES(IStringify);
    public:
        using unboxing_type = path;

        path get_unboxing_value() const { return value_; }

        Path() : CORELIB_INIT_INTERFACE(IStringify) {}
        Path(const path& p) : value_(p), CORELIB_INIT_INTERFACE(IStringify) {}
        Path(path&& p) : value_(std::move(p)), CORELIB_INIT_INTERFACE(IStringify) {}
        Path(std::string_view str) : value_(str), CORELIB_INIT_INTERFACE(IStringify) {}

        const path& GetPath() const { return value_; }
        void SetPath(const path& p) { value_ = p; }

        virtual string ToString() const override
        {
            return value_.generic_string();
        }

    public: /* IStringify */
        virtual void IStringify_Parse(const string& value) override
        {
            value_ = path(value);
        }
        virtual string IStringify_Stringify() override
        {
            return ToString();
        }

    private:
        path value_;
    };

    CORELIB_DECL_SHORTSPTR(Path);

} // namespace jxcorlib

CORELIB_DECL_BOXING(jxcorlib::path, jxcorlib::Path);

