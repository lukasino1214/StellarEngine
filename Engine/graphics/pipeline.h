#pragma once

#include "device.h"
#include "../pgepch.h"
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>

namespace Engine {
    class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
        shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) override {
            //BS
            std::string msg = std::string(requesting_source);
            msg += std::to_string(type);
            msg += static_cast<char>(include_depth);

            const std::string name = std::string(requested_source);
            const std::string contents = readFile(name);

            auto container = new std::array<std::string, 2>;
            (*container)[0] = name;
            (*container)[1] = contents;

            auto data = new shaderc_include_result;

            data->user_data = container;

            data->source_name = (*container)[0].data();
            data->source_name_length = (*container)[0].size();

            data->content = (*container)[1].data();
            data->content_length = (*container)[1].size();

            return data;
        };

        void ReleaseInclude(shaderc_include_result* data) override {
            delete static_cast<std::array<std::string, 2>*>(data->user_data);
            delete data;
        };

        static std::string readFile(const std::string &filepath) {
            std::string code;
            std::ifstream in(filepath, std::ios::in | std::ios::binary);
            if (in) {
                in.seekg(0, std::ios::end);
                u32 size = static_cast<u32>(in.tellg());
                if (size != 0) {
                    code.resize(size);
                    in.seekg(0, std::ios::beg);
                    in.read(&code[0], size);
                } else {
                    std::cout << "bruh" << std::endl;
                }
            } else {
                std::cout << "bruh" << std::endl;
            }
            return code;
        }
    };

    struct PipelineConfigInfo {
        PipelineConfigInfo() = default;

        PipelineConfigInfo(const PipelineConfigInfo &) = delete;
        PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

        std::vector<VkVertexInputBindingDescription> binding_descriptions;
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
        VkPipelineViewportStateCreateInfo viewport_info;
        VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
        VkPipelineRasterizationStateCreateInfo rasterization_info;
        VkPipelineMultisampleStateCreateInfo multisample_info;
        VkPipelineColorBlendAttachmentState color_blend_attachment;
        VkPipelineColorBlendStateCreateInfo color_blend_info;
        VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
        std::vector<VkDynamicState> dynamic_state_enables;
        VkPipelineDynamicStateCreateInfo dynamic_state_info;
        VkPipelineLayout vk_pipeline_layout = nullptr;
        VkRenderPass vk_renderpass = nullptr;
        uint32_t subpass = 0;
    };

    struct ShaderFilepaths {
        std::string vertex;
        std::string fragment;
        std::string geometry;
    };

    class Pipeline {
    public:
        Pipeline(std::shared_ptr<Device> _device, const PipelineConfigInfo &config_info, const ShaderFilepaths &paths);
        ~Pipeline();

        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        void bind(VkCommandBuffer commandBuffer);

        static void default_pipeline_config_info(PipelineConfigInfo &config_info);
        static void eneble_alpha_blending(PipelineConfigInfo &config_info);

    private:
        static std::string read_file(const std::string &filepath);

        std::vector<u32> compile_shader(const std::string &filepath, shaderc_shader_kind shader_type);
        void create_shader_module(const std::vector<u32> &code, VkShaderModule *shader_module);

        VkPipeline vk_pipeline = {};
        VkShaderModule vk_vertex_shader_module = {};
        VkShaderModule vk_fragment_shader_module = {};

        ShaderFilepaths shader_filepaths;

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        std::shared_ptr<Device> device;

        friend class NEShaderIncluder;
    };
}