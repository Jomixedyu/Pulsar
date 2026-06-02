#include "BrushEdTool.h"

#include "Pulsar/Logger.h"

namespace pulsared
{
    void BrushEdTool::Begin()
    {
        base::Begin();
        Logger::Log("BrushTool");
    }
} // namespace pulsared