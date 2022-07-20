#pragma once

#include <filesystem>
#include "../../Engine/graphics/texture.h"
#include "../../Engine/graphics/descriptor_set.h"

namespace Engine {
    class ContentBrowserPanel {
    public:
        ContentBrowserPanel(std::shared_ptr<Device> device);

        void render();

    private:
        void file_tree(const std::filesystem::path &path);

        std::filesystem::path current_directory;
        std::unique_ptr<Texture> file_icon;
        std::unique_ptr<Texture> directory_icon;
    };
}
