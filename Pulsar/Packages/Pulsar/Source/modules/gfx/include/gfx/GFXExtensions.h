#pragma once
#include "GFXPreDefined.h"

namespace gfx
{
    struct GFXExtensions
    {
    public:
        array_list<string> GetExtensionList() const
        {
            array_list<string> ret;
            for (size_t i = 0; i < m_count; i++)
            {
                ret.push_back(m_extensions[i]);
            }
            return ret;
        }
        const char* const* GetExtensionNames() const
        {
            return m_extensions;
        }
        size_t GetCount() const { return m_count; }
        bool HasExtension(string_view ext) const
        {
            for (size_t i = 0; i < m_count; i++)
            {
                if (m_extensions[i] == ext)
                {
                    return true;
                }
            }
            return false;
        }
    public:
        GFXExtensions(const char* const* extensions, size_t count)
            : m_extensions(extensions), m_count(count)
        {
        }
    private:
        size_t              m_count;
        const char* const* m_extensions;
    };
}