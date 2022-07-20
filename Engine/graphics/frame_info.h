#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace Engine {
    struct FrameInfo {
        int frame_index;
        float frame_time;
        VkCommandBuffer command_buffer;
        VkDescriptorSet vk_global_descriptor_set;
        VkDescriptorSet vk_shadow_descriptor_set;
    };

    struct DirectionalLight {
        glm::mat4 mvp{1.0};
        glm::vec4 position{0.0f, 0.0f, 0.0f, 0.0f};
    };

    struct PointLight {
        glm::vec4 position{};  // ignore w
        glm::vec4 color{};     // w is intensity
    };

    struct GlobalUbo {
        glm::mat4 projection_matrix{1.0f};
        glm::mat4 view_matrix{1.0f};
        glm::vec4 camera_position{0.0f, 0.0f, 0.0f, 0.0f};
        PointLight point_lights[10];
        DirectionalLight directional_lights[10];
        int num_point_lights;
        int num_directional_lights;
        float screen_width;
        float screen_height;
    };
}