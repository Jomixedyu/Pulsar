#include "istring.h"
#include <map>
#include <vector>


static struct strstruct
{

};

static std::vector<std::string>* istring_table[256] = { nullptr };
static uint32_t istring_index = 0;

std::map<uint32_t, std::string> d;

static uint32_t create(std::string_view str, int* out_pos)
{
    if (str.empty())
        return 0;
    auto c = str[0];

    std::vector<std::string>* vec = istring_table[str[0]];

    if (vec == nullptr)
    {
        vec = new std::vector<std::string>;
    }
    else
    {
        //find and return
        for (size_t i = 0; i < vec->size(); i++)
        {
            if (vec->at(i) == str)
            {
                *out_pos = i;
                return;
            }
        }
    }
    //new

    *out_pos = vec->size();

}

index_string::index_string(const std::string& str)
{

}

index_string::index_string(std::string_view view)
{

}

std::string index_string::to_string() const
{
    return std::string();
}

bool index_string::operator==(const index_string& right) const
{
    return this->id == right.id;
}
