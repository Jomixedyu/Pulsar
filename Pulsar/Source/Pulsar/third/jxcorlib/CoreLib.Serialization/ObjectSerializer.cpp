#include "ObjectSerializer.h"
#include <map>
#include <utility>
#include <vector>
#include <cassert>
#include "json.hpp"

namespace jxcorlib::ser
{
    using namespace nlohmann;

#define JSON_CAST(ptr) std::static_pointer_cast<VarientJson>(ptr)

    class VarientJson : public Varient, public std::enable_shared_from_this<VarientJson>
    {
    public:
        virtual int AsInt() const override
        {
            return js.get<int>();
        }
        virtual float AsFloat() const override
        {
            return js.get<float>();
        }
        virtual double AsDouble() const override
        {
            return js.get<double>();
        }
        virtual bool AsBool() const override
        {
            return js.get<bool>();
        }

        virtual std::string AsString() const override
        {
            return js.get<std::string>();
        }

        virtual bool IsNumber() const override
        {
            return js.is_number();
        }
        virtual bool IsBool() const override
        {
            return js.is_boolean();
        }
        virtual bool IsString() const override
        {
            return js.is_boolean();
        }
        virtual bool IsArray() const override
        {
            return js.is_array();
        }
        virtual bool IsObject() const override
        {
            return js.is_object();
        }
        virtual bool IsNull() const override
        {
            return js.is_null();
        }

        virtual size_t GetCount() override
        {
            return js.size();
        }
        virtual std::vector<std::string> GetKeys() const override
        {
            std::vector<std::string> ret;
            for (auto& item : js.items())
            {
                ret.emplace_back(item.key());
            }
            return ret;
        }
        virtual void Push(std::shared_ptr<Varient> value) override
        {
            js.push_back(JSON_CAST(value)->js);
        }
        virtual void Add(const char* key, std::shared_ptr<Varient> value) override
        {
            js[key] = JSON_CAST(value)->js;
        }
        virtual void RemoveAt(size_t index) override
        {
            js.erase(index);
        }
        virtual void RemoveAt(const std::string& key) override
        {
            js.erase(key);
        }

        virtual VarientType GetType() const override
        {
            if (IsNull())
                return VarientType::Null;
            else if (IsNumber())
                return VarientType::Number;
            else if (IsString())
                return VarientType::String;
            else if (IsBool())
                return VarientType::Bool;
            else if (IsArray())
                return VarientType::Array;
            else if (IsObject())
                return VarientType::Object;
            else
                return VarientType::Null;
        }
        virtual std::shared_ptr<Varient> New(VarientType type = VarientType::Null) const override
        {
            auto p = new VarientJson;
            switch (type)
            {
            case jxcorlib::ser::VarientType::Null:
                break;
            case jxcorlib::ser::VarientType::Number:
                p->js = 0;
                break;
            case jxcorlib::ser::VarientType::String:
                p->js = "";
                break;
            case jxcorlib::ser::VarientType::Bool:
                p->js = false;
                break;
            case jxcorlib::ser::VarientType::Array:
                p->js = json::array();
                break;
            case jxcorlib::ser::VarientType::Object:
                p->js = json::object();
                break;
            default:
                break;
            }
            return std::shared_ptr<VarientJson>(p);
        }

        virtual std::shared_ptr<Varient> Assign(int value) override
        {
            js = value;
            return shared_from_this();
        }

        virtual std::shared_ptr<Varient> Assign(float value) override
        {
            js = value;
            return shared_from_this();
        }

        virtual std::shared_ptr<Varient> Assign(double value) override
        {
            js = value;
            return shared_from_this();
        }

        virtual std::shared_ptr<Varient> Assign(bool value) override
        {
            js = value;
            return shared_from_this();
        }

        virtual std::shared_ptr<Varient> Assign(const std::string& value) override
        {
            js = value;
            return shared_from_this();
        }

        virtual std::shared_ptr<Varient> Assign(std::nullptr_t) override
        {
            js = nullptr;
            return shared_from_this();
        }

        virtual std::shared_ptr<Varient> Assign(const std::shared_ptr<Varient>& value) override
        {
            js = std::static_pointer_cast<VarientJson>(value)->js;
            return shared_from_this();
        }

        virtual void AssignParse(std::string_view content) override
        {
            js = json::parse(content);
        }
        virtual std::string ToString(bool readable = true) const override
        {
            return js.dump(readable ? 4 : -1);
        }

        virtual std::shared_ptr<Varient> At(size_t index) override
        {
            auto n = JSON_CAST(New());
            n->js = js[index];
            return n;
        }

        virtual std::shared_ptr<Varient> At(const std::string& key) override
        {
            auto n = JSON_CAST(New());
            n->js = js[key];
            return n;
        }

        virtual void Clear() override
        {
            js.clear();
        }

    protected:
        json js;
    };

    void Varient::Add(std::string_view key, std::shared_ptr<Varient> value)
    {
        Add(key.data(), std::move(value));
    }
    void Varient::Add(std::string_view key, int value)
    {
        Add(key.data(), New(VarientType::Number)->Assign(value));
    }
    void Varient::Add(std::string_view key, float value)
    {
        Add(key.data(), New(VarientType::Number)->Assign(value));
    }
    void Varient::Add(std::string_view key, const std::string& value)
    {
        Add(key.data(), New(VarientType::String)->Assign(value));
    }

    VarientRef CreateVarient(std::string_view provider)
    {
        if (provider == "json")
        {
            return std::shared_ptr<Varient>(new VarientJson);
        }
        return {};
    }

}