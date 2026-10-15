#include <PulsarEd/UIControls/TypePicker.h>
#include <CoreLib/Assembly.h>
#include <imgui/imgui.h>
#include <algorithm>
#include <cctype>

namespace pulsared
{
    static string ToLowerAscii(string_view value)
    {
        string result;
        result.reserve(value.size());
        for (char ch : value)
            result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        return result;
    }
    void TypePickerPopup::Open(const char* popupId)
    {
        ImGui::OpenPopup(popupId);
    }

    bool TypePickerPopup::Draw(const char* popupId, const TypePickerOptions& options, Type*& outSelectedType)
    {
        outSelectedType = nullptr;
        bool selected = false;

        if (!ImGui::BeginPopup(popupId))
            return false;

        static char searchBuffer[128]{};
        ImGui::SetNextItemWidth(260.f);
        ImGui::InputText("##TypeSearch", searchBuffer, sizeof(searchBuffer));
        const string searchLower = ToLowerAscii(searchBuffer);
        ImGui::Separator();

        array_list<Type*> types;
        if (options.BaseType)
        {
            types = AssemblyManager::GlobalSearchType(options.BaseType);
            if (options.IncludeBaseType
                && std::ranges::find(types, options.BaseType) == types.end())
            {
                types.push_back(options.BaseType);
            }
        }
        else
        {
            for (auto* assembly : AssemblyManager::GetAssemblies())
            {
                for (const auto& type : assembly->GetTypes())
                    types.push_back(type);
            }
        }

        std::ranges::sort(types, [](Type* lhs, Type* rhs)
        {
            return lhs->GetName() < rhs->GetName();
        });

        for (auto* type : types)
        {
            if (!type)
                continue;
            if (!options.IncludeBaseType && options.BaseType && type == options.BaseType)
                continue;
            if (type->IsInterface())
                continue;
            if (options.Filter && !options.Filter(type))
                continue;

            const string& typeName = type->GetName();
            if (!searchLower.empty())
            {
                auto typeNameLower = ToLowerAscii(typeName);
                auto shortNameLower = ToLowerAscii(type->GetShortName());
                if (typeNameLower.find(searchLower) == string::npos
                    && shortNameLower.find(searchLower) == string::npos)
                {
                    continue;
                }
            }

            if (ImGui::MenuItem(type->GetShortName().c_str()))
            {
                outSelectedType = type;
                selected = true;
                searchBuffer[0] = '\0';
                ImGui::CloseCurrentPopup();
                break;
            }
        }

        ImGui::EndPopup();
        return selected;
    }
}