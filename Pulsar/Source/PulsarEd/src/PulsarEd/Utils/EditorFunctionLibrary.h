#pragma once
#include <string>
#include <PulsarEd/Assembly.h>

namespace pulsared
{
    /**
     * @brief Editor function library for external calls (e.g. network API).
     * Provides high-level editor operations as static methods.
     */
    class EditorFunctionLibrary final
    {
    public:
        /**
         * @brief Capture a screenshot from the current editor scene camera and save to Saved/Screenshots/.
         * @return The output file path on success, empty string on failure.
         */
        static std::string CaptureScreenshot();
    };
}
