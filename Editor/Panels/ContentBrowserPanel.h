//
// Created by lukas on 23.04.22.
//

#ifndef VENDOR_CONTENTBROWSERPANEL_H
#define VENDOR_CONTENTBROWSERPANEL_H

#include <filesystem>
#include "../../Engine/Graphics/Texture.h"
#include "../../Engine/Graphics/Descriptors.h"

namespace Engine {

    class ContentBrowserPanel {
    public:
        ContentBrowserPanel(std::shared_ptr<Device> device);

        void OnImGuiRender();

    private:
        std::filesystem::path m_CurrentDirectory;

        void FileTree(const std::filesystem::path &path);

        std::unique_ptr<Texture> m_FileIcon;
        std::unique_ptr<Texture> m_DirectoryIcon;
    };

}


#endif //VENDOR_CONTENTBROWSERPANEL_H
