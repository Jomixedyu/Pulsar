#include <vector>
#include <string>
#include <cassert>
#include <stacktrace>

struct DestroyedInsight
{
    static inline std::vector<DestroyedInsight*> instances;

    struct Info
    {
        std::string FileInfo;
        std::string CreateStack;
        void* Ptr;
    };

    std::vector<Info> Infos;
    std::string DefinePosition;
    const char* Name;

public:
    DestroyedInsight(const char* name, const std::string& definePosition) :
        Name(name), DefinePosition(definePosition)
    {
        instances.push_back(this);
    }

    void Add(const std::string& fileInfo, void* ptr)
    {
        Infos.push_back({ fileInfo, std::to_string(std::stacktrace::current(1)), ptr });
    }
    void Dec(void* ptr)
    {
        auto it = std::find_if(Infos.begin(), Infos.end(), [&](const Info& info) { return info.Ptr == ptr; });
        if (it != Infos.end())
        {
            Infos.erase(it);
        }
        else
        {
            assert(0);
        }
    }
};
#define DESTROYED_INSIGHT_DEFINE(Name) static DestroyedInsight Name(#Name, std::string(__FILE__) + std::to_string(__LINE__));
#define DESTROYED_INSIGHT_CTOR(Name) Name.Add(std::string(__FILE__) + std::to_string(__LINE__), this);
#define DESTROYED_INSIGHT_DESTOR(Name) Name.Dec(this);