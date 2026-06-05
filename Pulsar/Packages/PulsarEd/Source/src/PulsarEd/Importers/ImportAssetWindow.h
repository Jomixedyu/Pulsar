#pragma once
#include "AssetImporter.h"
#include <PulsarEd/EditorAppInstance.h>
#include <filesystem>

namespace pulsared
{
    // Modal dialog: first choose asset type + configure settings,
    // then click "Select File & Import" to open the file dialog and import.
    class ImportAssetWindow : public ModalDialog
    {
    public:
        // Pass all available factories and the target folder.
        // No file path needed at construction time.
        ImportAssetWindow(
            array_list<AssetImporterFactory*> factories,
            string targetFolder);

        void OnDraw(float dt) override;
        ImVec2 GetWindowSize() const override;

    private:
        void RebuildSettings(int index);
        void DoImport();

        array_list<AssetImporterFactory*>  m_factories;
        string                             m_targetFolder;

        int                                m_selectedIndex = 0;
        SPtr<AssetImporterSettings>        m_currentSettings;
    };
}
