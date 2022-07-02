//
// Created by lukas on 08.11.21.
//

#ifndef ENGINEEDITOR_PIPELINE_H
#define ENGINEEDITOR_PIPELINE_H

#include "Device.h"
#include "../pgepch.h"
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>

namespace Engine {

    class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
        shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) {
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
                size_t size = in.tellg();
                if (size != -1) {
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

        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    struct ShaderPaths {
        std::string vertPath = "";
        std::string fragPath = "";
        std::string geomPath = "";
    };

    class Pipeline {
    public:
        Pipeline(std::shared_ptr<Device> device, const PipelineConfigInfo &configInfo, const ShaderPaths &paths);

        ~Pipeline();

        Pipeline(const Pipeline &) = delete;

        Pipeline &operator=(const Pipeline &) = delete;

        void bind(VkCommandBuffer commandBuffer);

        static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

        static void enebleAlphaBlending(PipelineConfigInfo &configInfo);

    private:
        static std::string readFile(const std::string &filepath);

        void createGraphicsPipeline(const PipelineConfigInfo &configInfo);

        std::vector<uint32_t> compileShader(const std::string &shaderFilepath, shaderc_shader_kind shaderType);
        void createShaderModule(const std::vector<uint32_t> &code, VkShaderModule *shaderModule);

        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;

        ShaderPaths m_Paths;

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        std::shared_ptr<Device> m_Device;

        friend class NEShaderIncluder;
    };
}


#endif //ENGINEEDITOR_PIPELINE_H
