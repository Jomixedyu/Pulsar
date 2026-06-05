#include "rdg/RDGPass.h"

namespace rdg
{

    RDGPass::RDGPass(std::string_view name)
        : Culled(true), m_name(name)
    {
    }
} // namespace rdg