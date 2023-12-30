#pragma once
#include <memory>
#include <string_view>
#include <string>
#include <stdint.h>
#include <any>
#include <vector>

namespace jxcorlib::ser
{
    enum class VarientType
    {
        Null,
        Number,
        String,
        Bool,
        Array,
        Object
    };

    class Varient
    {
    public:
        virtual ~Varient() {}

        virtual int AsInt() const = 0;
        virtual float AsFloat() const = 0;
        virtual double AsDouble() const = 0;
        virtual bool AsBool() const = 0;
        virtual std::string AsString() const = 0;

        virtual bool IsNull() const = 0;
        virtual bool IsNumber() const = 0;
        virtual bool IsBool() const = 0;
        virtual bool IsString() const = 0;
        virtual bool IsArray() const = 0;
        virtual bool IsObject() const = 0;


        virtual std::shared_ptr<Varient> Assign(int value) = 0;
        virtual std::shared_ptr<Varient> Assign(float value) = 0;
        virtual std::shared_ptr<Varient> Assign(double value) = 0;
        virtual std::shared_ptr<Varient> Assign(bool value) = 0;
        virtual std::shared_ptr<Varient> Assign(const std::string& value) = 0;
        virtual std::shared_ptr<Varient> Assign(std::nullptr_t) = 0;
        virtual std::shared_ptr<Varient> Assign(const std::shared_ptr<Varient>& value) = 0;

        virtual void AssignParse(std::string_view content) = 0;

        virtual size_t GetCount() = 0;
        virtual std::vector<std::string> GetKeys() const = 0;

        virtual std::shared_ptr<Varient> At(size_t index) = 0;
        virtual std::shared_ptr<Varient> At(const std::string& key) = 0;

        virtual void Push(std::shared_ptr<Varient> value) = 0;
        void Push(int value) { Push(New(VarientType::Number)->Assign(value)); }
        void Push(float value) { Push(New(VarientType::Number)->Assign(value)); }
        void Push(double value) { Push(New(VarientType::Number)->Assign(value)); }
        void Push(bool value) { Push(New(VarientType::Bool)->Assign(value)); }
        void Push(const std::string& value) { Push(New(VarientType::String)->Assign(value)); }

        void Add(std::string_view key, std::shared_ptr<Varient> value);
        void Add(std::string_view key, int value);
        void Add(std::string_view key, float value);
        void Add(std::string_view key, const std::string& value);
        virtual void Add(const char* key, std::shared_ptr<Varient> value) = 0;
        virtual void RemoveAt(size_t index) = 0;
        virtual void RemoveAt(const std::string& key) = 0;
        virtual void Clear() = 0;

        virtual VarientType GetType() const = 0;
        virtual std::string ToString(bool readable = true) const = 0;

        virtual std::shared_ptr<Varient> New(VarientType type = VarientType::Null) const = 0;
    };
    using VarientRef = std::shared_ptr<Varient>;

    extern VarientRef CreateVarient(std::string_view provider);
}