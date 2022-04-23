//
// Created by lukas on 08.11.21.
//

#ifndef ENGINEEDITOR_MODEL_H
#define ENGINEEDITOR_MODEL_H

#include "device.h"
#include "buffer.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace Engine {
    class Model {
    public:
        struct Vertex {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex &other) const {
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
            }
        };

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string &filepath);
        };

        Model(const std::string &filepath);
        Model(const Model::Builder &builder);
        ~Model();

        static std::unique_ptr<Model> createModelfromFile(const std::string &filepath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        std::string getPath() { return m_Path; }

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

        std::unique_ptr<Buffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::unique_ptr<Buffer> indexBuffer;
        uint32_t indexCount;
        std::string m_Path;
    };
}


#endif //ENGINEEDITOR_MODEL_H
