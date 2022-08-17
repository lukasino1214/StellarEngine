#include "pipeline.h"

#include "model.h"
#include "core.h"
#include <cstdint>
#include <shaderc/shaderc.hpp>
#include <utility>

namespace Engine {
    Pipeline::Pipeline(std::shared_ptr<Device> _device, const PipelineConfigInfo &config_info, ShaderFilepaths paths) : device{std::move(_device)}, shader_filepaths{std::move(paths)} {
        assert(config_info.vk_pipeline_layout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
        assert(config_info.vk_renderpass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no renderPass provided in configInfo");

        options.SetIncluder(std::make_unique<ShaderIncluder>());
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        //options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

        u32 num_shaders = 0;
        if(!shader_filepaths.vertex.empty()) {
            num_shaders += 1;
            auto code = compile_shader(shader_filepaths.vertex, shaderc_vertex_shader);
            create_shader_module(code, &vk_vertex_shader_module);
        }

        if(!shader_filepaths.fragment.empty()) {
            num_shaders += 1;
            auto code = compile_shader(shader_filepaths.fragment, shaderc_fragment_shader);
            create_shader_module(code, &vk_fragment_shader_module);
        }

        VkPipelineShaderStageCreateInfo shader_stages[num_shaders];

        if(!shader_filepaths.vertex.empty()) {
            shader_stages[0] = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                    .module = vk_vertex_shader_module,
                    .pName = "main",
                    .pSpecializationInfo = nullptr
            };
        }

        if(!shader_filepaths.fragment.empty()) {
            shader_stages[1] = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .module = vk_fragment_shader_module,
                    .pName = "main",
                    .pSpecializationInfo = nullptr
            };
        }

        auto &attribute_descriptions = config_info.attribute_descriptions;
        auto &binding_descriptions = config_info.binding_descriptions;

        VkPipelineVertexInputStateCreateInfo vk_pipeline_vertex_input_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size()),
                .pVertexBindingDescriptions = binding_descriptions.data(),
                .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size()),
                .pVertexAttributeDescriptions = attribute_descriptions.data(),
        };

        VkGraphicsPipelineCreateInfo vk_graphics_pipeline_create_info = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stageCount = num_shaders,
                .pStages = shader_stages,
                .pVertexInputState = &vk_pipeline_vertex_input_state_create_info,
                .pInputAssemblyState = &config_info.input_assembly_info,
                .pTessellationState = nullptr,
                .pViewportState = &config_info.viewport_info,
                .pRasterizationState = &config_info.rasterization_info,
                .pMultisampleState = &config_info.multisample_info,
                .pDepthStencilState = &config_info.depth_stencil_info,
                .pColorBlendState = &config_info.color_blend_info,
                .pDynamicState = &config_info.dynamic_state_info,
                .layout = config_info.vk_pipeline_layout,
                .renderPass = config_info.vk_renderpass,
                .subpass = config_info.subpass,
                .basePipelineHandle = {},
                .basePipelineIndex = -1,
        };

        if (vkCreateGraphicsPipelines(device->vk_device, VK_NULL_HANDLE, 1, &vk_graphics_pipeline_create_info, nullptr, &vk_pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline");
        }
    }

    Pipeline::~Pipeline() {
        if(!shader_filepaths.vertex.empty()) {
            vkDestroyShaderModule(device->vk_device, vk_vertex_shader_module, nullptr);
        }

        if(!shader_filepaths.fragment.empty()) {
            vkDestroyShaderModule(device->vk_device, vk_fragment_shader_module, nullptr);
        }

        vkDestroyPipeline(device->vk_device, vk_pipeline, nullptr);
    }

    std::string Pipeline::read_file(const std::string &filepath) {
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

    std::vector<uint32_t> Pipeline::compile_shader(const std::string &shaderFilepath, shaderc_shader_kind shaderType) {
        std::string shaderCode = read_file(shaderFilepath);
        shaderc::PreprocessedSourceCompilationResult pre_result =
        compiler.PreprocessGlsl(shaderCode, shaderc_fragment_shader, shaderFilepath.c_str(), options);

        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(pre_result.begin(), shaderType, shaderFilepath.c_str(),
                                                                              options);

        if(pre_result.GetNumErrors() > 0) {
            std::cout << pre_result.GetErrorMessage() << std::endl;
        }

        if(result.GetNumErrors() > 0) {
            std::cout << result.GetErrorMessage() << std::endl;
        }

        return std::vector<uint32_t>(result.cbegin(), result.cend());
    }

    void Pipeline::create_shader_module(const std::vector<uint32_t> &code, VkShaderModule *shader_module) {
        VkShaderModuleCreateInfo vk_shader_module_create_info = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .codeSize = sizeof(uint32_t) * code.size(),
                .pCode = code.data()
        };

        if (vkCreateShaderModule(device->vk_device, &vk_shader_module_create_info, nullptr, shader_module) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module");
        }
    }

    void Pipeline::bind(VkCommandBuffer command_buffer) {
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);
    }

    void Pipeline::default_pipeline_config_info(PipelineConfigInfo &config_info) {
        config_info.input_assembly_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .primitiveRestartEnable = VK_FALSE
        };

        config_info.viewport_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .viewportCount = 1,
                .pViewports = nullptr,
                .scissorCount = 1,
                .pScissors = nullptr
        };

        config_info.rasterization_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .cullMode = VK_CULL_MODE_NONE,
                .frontFace = VK_FRONT_FACE_CLOCKWISE,
                .depthBiasEnable = VK_FALSE,
                .depthBiasConstantFactor = 0.0f,
                .depthBiasClamp = 0.0f,
                .depthBiasSlopeFactor = 0.0f,
                .lineWidth = 1.0f
        };

        config_info.multisample_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = 1.0f,
                .pSampleMask = nullptr,
                .alphaToCoverageEnable = VK_FALSE,
                .alphaToOneEnable = VK_FALSE
        };

        config_info.color_blend_attachment = {
                .blendEnable = VK_FALSE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        config_info.color_blend_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY,
                .attachmentCount = 1,
                .pAttachments = &config_info.color_blend_attachment,
                .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
        };

        config_info.depth_stencil_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .depthTestEnable = VK_TRUE,
                .depthWriteEnable = VK_TRUE,
                .depthCompareOp = VK_COMPARE_OP_LESS,
                .depthBoundsTestEnable = VK_FALSE,
                .stencilTestEnable = VK_FALSE,
                .front = {},
                .back = {},
                .minDepthBounds = 0.0f,
                .maxDepthBounds = 1.0f,
        };

        config_info.dynamic_state_enables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        config_info.dynamic_state_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .dynamicStateCount = static_cast<uint32_t>(config_info.dynamic_state_enables.size()),
                .pDynamicStates = config_info.dynamic_state_enables.data()
        };

        config_info.binding_descriptions = Model::Vertex::getBindingDescriptions();
        config_info.attribute_descriptions = Model::Vertex::getAttributeDescriptions();
    }

    void Pipeline::eneble_alpha_blending(PipelineConfigInfo &config_info) {
        config_info.color_blend_attachment = {
                .blendEnable = VK_TRUE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };
    }
}
