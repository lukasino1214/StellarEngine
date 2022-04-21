//
// Created by lukas on 08.11.21.
//

#include "new_model.h"

#include "../Utils/utils.h"

#define TINYOBJECTLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>
#include <cstring>
#include <unordered_map>
#include <iostream>

#include <fx/gltf.h>

namespace std {
    template <>
    struct hash<Engine::NewModel::Vertex> {
        size_t operator()(Engine::NewModel::Vertex const &vertex) const {
            size_t seed = 0;
            Engine::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace Engine {

    NewModel::~NewModel() {}

    void NewModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        uint32_t vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        Buffer stagingBuffer{
            m_Device,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)vertices.data());

        vertexBuffer = std::make_unique<Buffer>(
                m_Device,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_Device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }

    void NewModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
        uint32_t indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if(!hasIndexBuffer) {
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        Buffer stagingBuffer{
            m_Device,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)indices.data());

        indexBuffer = std::make_unique<Buffer>(
                m_Device,
                indexSize,
                indexCount,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                );

        m_Device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }

    void NewModel::draw(VkCommandBuffer commandBuffer) {
        uint32_t vertexOffset = 0;
        uint32_t indexOffset = 0;

        for(auto& primitive : primitives) {
            if(hasIndexBuffer) {
                vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1, indexOffset, vertexOffset, 0);
            }

            else {
                vkCmdDraw(commandBuffer, primitive.vertexCount, 1, 0, 0);
            }

            vertexOffset += primitive.vertexCount;
            indexOffset += primitive.indexCount;

        }
    }

    void NewModel::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if(hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    NewModel::NewModel(Device &device, const std::string &filepath) : m_Device{device}, m_Path{filepath} {
        fx::gltf::Document doc = fx::gltf::LoadFromText(filepath);

        for(auto& mesh : doc.meshes) {
            for(auto& primitive : mesh.primitives) {
                uint32_t vertexCount = 0;
                uint32_t indexCount = 0;

                const float *positionBuffer = nullptr;
                const float *normalsBuffer = nullptr;
                const float *texCoordsBuffer = nullptr;

                for (auto const & attrib : primitive.attributes) {
                    if (attrib.first == "POSITION") {
                        const fx::gltf::Accessor &accessor = doc.accessors[primitive.attributes.find("POSITION")->second];
                        const fx::gltf::BufferView &view = doc.bufferViews[accessor.bufferView];
                        positionBuffer = reinterpret_cast<const float *>(&(doc.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                        vertexCount = accessor.count;
                    }

                    if (attrib.first == "NORMAL") {
                        const fx::gltf::Accessor &accessor = doc.accessors[primitive.attributes.find("NORMAL")->second];
                        const fx::gltf::BufferView &view = doc.bufferViews[accessor.bufferView];
                        normalsBuffer = reinterpret_cast<const float *>(&(doc.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    }

                    if (attrib.first == "TEXCOORD_0") {
                        const fx::gltf::Accessor &accessor = doc.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                        const fx::gltf::BufferView &view = doc.bufferViews[accessor.bufferView];
                        texCoordsBuffer = reinterpret_cast<const float *>(&(doc.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    }
                }

                for (size_t v = 0; v < vertexCount; v++) {
                    NewModel::Vertex vertex{};
                    auto position = glm::make_vec3(&positionBuffer[v * 3]);
                    vertex.position = position;
                    vertex.color = glm::vec3(1.0);
                    vertex.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
                    vertex.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec2(0.0f);
                    vertices.push_back(vertex);
                }

                {
                    const fx::gltf::Accessor& accessor = doc.accessors[primitive.indices];
                    const fx::gltf::BufferView& bufferView = doc.bufferViews[accessor.bufferView];
                    const fx::gltf::Buffer& buffer = doc.buffers[bufferView.buffer];

                    indexCount += static_cast<uint32_t>(accessor.count);

                    switch (accessor.componentType) {
                        case fx::gltf::Accessor::ComponentType::UnsignedInt: {
                            const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++) {
                                indices.push_back(buf[index]);
                            }
                            break;
                        }
                        case fx::gltf::Accessor::ComponentType::UnsignedShort: {
                            const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++) {
                                indices.push_back(buf[index]);
                            }
                            break;
                        }
                        case fx::gltf::Accessor::ComponentType::UnsignedByte: {
                            const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++) {
                                indices.push_back(buf[index]);
                            }
                            break;
                        }
                        default:
                            return;
                    }
                }

                Primitive mesh_primitive{};
                mesh_primitive.firstVertex = static_cast<uint32_t>(vertices.size());;
                mesh_primitive.vertexCount = vertexCount;
                mesh_primitive.indexCount = indexCount;
                mesh_primitive.firstIndex = static_cast<uint32_t>(indices.size());
                primitives.push_back(mesh_primitive);

            }
        }

        createVertexBuffers(vertices);
        createIndexBuffers(indices);

    }

    std::vector<VkVertexInputBindingDescription> NewModel::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> NewModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

        return attributeDescriptions;
    }

}