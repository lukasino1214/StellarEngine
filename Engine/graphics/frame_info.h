#pragma once

#include <glm/glm.hpp>

#include "device.h"

namespace Engine {
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
        [[maybe_unused]] int num_directional_lights;
        [[maybe_unused]] float screen_width;
        [[maybe_unused]] float screen_height;
    };
    struct FrameInfo {
        [[maybe_unused]] uint32_t frame_index{};
        [[maybe_unused]] float frame_time{};
        VkCommandBuffer command_buffer{};
        VkDescriptorSet vk_global_descriptor_set{};
        GlobalUbo ubo{};
    };
}