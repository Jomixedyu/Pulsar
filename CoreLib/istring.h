#pragma once

#include <string_view>
#include <string>
#include <cstdint>

struct index_string
{
    index_string(const std::string& str);
    index_string(std::string_view view);

    std::string to_string() const;

    bool operator==(const index_string& right) const;
private:
    uint32_t index;
    uint32_t pos;
};
