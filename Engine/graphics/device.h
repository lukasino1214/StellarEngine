#pragma once

#include "../core/window.h"
#include "../pgepch.h"

namespace Engine {
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    struct QueueFamilyIndices {
        uint32_t graphics_family{};
        uint32_t present_family{};
        bool graphics_family_has_value = false;
        bool present_family_has_value = false;
        [[nodiscard]] bool is_complete() const { return graphics_family_has_value && present_family_has_value; }
    };

    class Device {
    public:
#ifdef NDEBUG
        const bool enable_validation_layers = true;
#else
        const bool enable_validation_layers = true;
#endif

        explicit Device(Window* _window);
        ~Device();

        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;
        Device(Device &&) = delete;
        Device &operator=(Device &&) = delete;

        uint32_t get_graphics_queue_family() { return find_physical_queue_families().graphics_family; }

        SwapChainSupportDetails get_swapchain_support() { return query_swapchain_support(vk_physical_device); }
        uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const;
        QueueFamilyIndices find_physical_queue_families() { return find_queue_families(vk_physical_device); }
        VkFormat find_supported_format(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

        // Buffer Helper Functions
        void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory);
        VkCommandBuffer begin_single_time_command_buffer();
        void end_single_time_command_buffer(VkCommandBuffer command_buffer) const;
        void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
        void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count);

        void create_image_with_info(const VkImageCreateInfo &image_info, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &image_memory);

        VkPhysicalDeviceProperties properties;

        VkDevice vk_device = {};
        VkSurfaceKHR vk_surface_khr = {};
        VkQueue vk_graphics_queue = {};
        VkQueue vk_present_queue = {};
        VkPhysicalDevice vk_physical_device = {};
        VkCommandPool vk_command_pool = {};
        VkInstance vk_instance = {};
    private:
        void create_instance();
        void setup_debug_messenger();
        void create_surface();
        void pick_physical_device();
        void create_logical_device();
        void create_command_pool();

        // helper functions
        bool is_device_suitable(VkPhysicalDevice device);
        std::vector<const char *> get_required_extensions() const;
        bool check_validation_layer_support();
        QueueFamilyIndices find_queue_families(VkPhysicalDevice device) const;
        void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info);
        void has_gflw_required_instance_extensions();
        bool check_device_extension_support(VkPhysicalDevice device);
        SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice device) const;

        VkDebugUtilsMessengerEXT vk_debug_messenger;
        Window* window;

        const std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };
}