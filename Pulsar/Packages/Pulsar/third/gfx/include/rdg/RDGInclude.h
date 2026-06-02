#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <type_traits>

namespace gfx
{

}

namespace rdg
{
    using namespace gfx;

    struct RDGHandle
    {
        int Handle;
    };

    template <typename T>
    struct RDGResourceHandle : public RDGHandle
    {

    };
}