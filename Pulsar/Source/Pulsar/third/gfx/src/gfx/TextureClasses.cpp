#include "gfx/TextureClasses.h"

namespace gfx
{
    std::vector<std::pair<std::string, uint32_t>> BuildEnumMap_SamplerFilter()
    {
        std::vector<std::pair<std::string, uint32_t>> map;

        map.emplace_back("Nearest", (uint32_t)GFXSamplerFilter::Nearest);
        map.emplace_back("Linear", (uint32_t)GFXSamplerFilter::Linear);
        map.emplace_back("Cubic", (uint32_t)GFXSamplerFilter::Cubic);

        return map;
    }

    std::vector<std::pair<std::string, uint32_t>> BuildEnumMap_SamplerAddressMode()
    {
        std::vector<std::pair<std::string, uint32_t>> map;

        map.emplace_back("Repeat", (uint32_t)GFXSamplerAddressMode::Repeat);
        map.emplace_back("MirroredRepeat", (uint32_t)GFXSamplerAddressMode::MirroredRepeat);
        map.emplace_back("ClampToEdge", (uint32_t)GFXSamplerAddressMode::ClampToEdge);

        return map;
    }

}