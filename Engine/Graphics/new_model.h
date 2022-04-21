//
// Created by lukas on 08.11.21.
//

#ifndef ENGINEEDITOR_NEWMODEL_H
#define ENGINEEDITOR_NEWMODEL_H

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
    class NewModel {
    public:
        struct Primitive {
            uint32_t firstIndex;
            uint32_t firstVertex;
            uint32_t indexCount;
            uint32_t vertexCount;
        };


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

        NewModel(Device &device, const std::string &filepath);
        ~NewModel();


        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Primitive> primitives;
    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

        Device &m_Device;
        std::unique_ptr<Buffer> vertexBuffer;

        bool hasIndexBuffer = false;
        std::unique_ptr<Buffer> indexBuffer;
        std::string m_Path;
    };
}


#endif //ENGINEEDITOR_NEWMODEL_H
