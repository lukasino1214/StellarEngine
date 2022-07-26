#pragma once

#include "device.h"
#include "buffer.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>

// std
#include "../pgepch.h"
#include "texture.h"
#include "descriptor_set.h"
#include "frame_info.h"

namespace Engine {
    class Model {
    public:
        struct PBRParameters {
            glm::vec4 base_color_factor;
            glm::vec3 emissive_factor;
            f32 metallic_factor;
            f32 roughness_factor;
            f32 scale;
            f32 strength;
            f32 alphaCutoff;
            f32 alphaMode;

            int has_base_color_texture;
            int has_metallic_roughness_texture;
            int has_normal_texture;
            int has_occlusion_texture;
            int has_emissive_texture;
        };

        struct PBRMaterial {
            std::shared_ptr<Texture> base_color_texture;
            std::shared_ptr<Texture> metallic_roughness_texture;
            std::shared_ptr<Texture> normal_texture;
            std::shared_ptr<Texture> occlusion_texture;
            std::shared_ptr<Texture> emissive_texture;
            PBRParameters pbr_parameters = {};

            std::shared_ptr<Buffer> pbr_parameters_buffer = {};
            VkDescriptorSet descriptor_set = {};
        };

        struct Material {
            std::shared_ptr<Texture> albedoTexture;
            std::shared_ptr<Texture> normalTexture;
            std::shared_ptr<Texture> metallicRoughnessTexture;
            VkDescriptorSet descriptorSet;
        };

        struct Primitive {
            uint32_t firstIndex;
            uint32_t firstVertex;
            uint32_t indexCount;
            uint32_t vertexCount;
            Material material;
        };

        struct Vertex {
            glm::vec3 position{};
            glm::vec3 normal{};
            glm::vec4 tangent{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex &other) const {
                return position == other.position && normal == other.normal && uv == other.uv;
            }
        };

        Model(std::shared_ptr<Device> device, const std::string &filepath);
        ~Model();

        void bind(VkCommandBuffer commandBuffer);
        void draw(FrameInfo frameInfo, VkPipelineLayout pipelineLayout);

        std::string getPath() { return m_Path; }

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Primitive> primitives;
        std::vector<std::shared_ptr<Texture>> images;
    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);

        void createIndexBuffers(const std::vector<uint32_t> &indices);

        std::unique_ptr<Buffer> vertexBuffer;

        bool hasIndexBuffer = false;
        std::unique_ptr<Buffer> indexBuffer;
        std::string m_Path;
        std::shared_ptr<Device> m_Device;
    };
}