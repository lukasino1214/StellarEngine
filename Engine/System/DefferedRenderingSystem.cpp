//
// Created by lukas on 18.6.22.
//

#include "DefferedRenderingSystem.h"

/*

namespace Engine {

    DefferedRenderingSystem::DefferedRenderingSystem(std::shared_ptr<Device> device, int width, int height) : m_Width(width), m_Height(height), m_Device(device) {



        attachments.width = width;
		attachments.height = height;

		createGBufferAttachments();

		std::array<VkAttachmentDescription, 5> attachments{};
		// Color attachment
		attachments[0].format = VK_FORMAT_B8G8R8A8_SRGB;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Deferred attachments
		// Position
		attachments[1].format = this->attachments.position.format;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// Normals
		attachments[2].format = this->attachments.normal.format;
		attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// Albedo
		attachments[3].format = this->attachments.albedo.format;
		attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[3].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


        VkFormat fbDepthFormat = m_Device->findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

		// Depth attachment
		attachments[4].format = fbDepthFormat;
		attachments[4].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[4].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[4].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Three subpasses
		std::array<VkSubpassDescription,3> subpassDescriptions{};

		// First subpass: Fill G-Buffer components
		// ----------------------------------------------------------------------------------------

		VkAttachmentReference colorReferences[4];
		colorReferences[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		colorReferences[1] = { 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		colorReferences[2] = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		colorReferences[3] = { 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		VkAttachmentReference depthReference = { 4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescriptions[0].colorAttachmentCount = 4;
		subpassDescriptions[0].pColorAttachments = colorReferences;
		subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

		// Second subpass: Final composition (using G-Buffer components)
		// ----------------------------------------------------------------------------------------

		VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkAttachmentReference inputReferences[3];
		inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		inputReferences[1] = { 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		inputReferences[2] = { 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		uint32_t preserveAttachmentIndex = 1;

		subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescriptions[1].colorAttachmentCount = 1;
		subpassDescriptions[1].pColorAttachments = &colorReference;
		subpassDescriptions[1].pDepthStencilAttachment = &depthReference;
		// Use the color attachments filled in the first pass as input attachments
		subpassDescriptions[1].inputAttachmentCount = 3;
		subpassDescriptions[1].pInputAttachments = inputReferences;

		// Third subpass: Forward transparency
		// ----------------------------------------------------------------------------------------
		colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		subpassDescriptions[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescriptions[2].colorAttachmentCount = 1;
		subpassDescriptions[2].pColorAttachments = &colorReference;
		subpassDescriptions[2].pDepthStencilAttachment = &depthReference;
		// Use the color/depth attachments filled in the first pass as input attachments
		subpassDescriptions[2].inputAttachmentCount = 1;
		subpassDescriptions[2].pInputAttachments = inputReferences;

		// Subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 4> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// This dependency transitions the input attachment from color attachment to shader read
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = 1;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[2].srcSubpass = 1;
		dependencies[2].dstSubpass = 2;
		dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[2].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[3].srcSubpass = 2;
		dependencies[3].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[3].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[3].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[3].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[3].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
		renderPassInfo.pSubpasses = subpassDescriptions.data();
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		vkCreateRenderPass(device->device(), &renderPassInfo, nullptr, &renderPass);
    }

    void DefferedRenderingSystem::Render(FrameInfo &frameInfo, const std::shared_ptr<Scene> &Scene) {
        VkClearValue clearValues[5];
        clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
        clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
        clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
        clearValues[3].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
        clearValues[4].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = m_Width;
        renderPassBeginInfo.renderArea.extent.height = m_Height;
        renderPassBeginInfo.clearValueCount = 5;
        renderPassBeginInfo.pClearValues = clearValues;

        // Set target frame buffer
        renderPassBeginInfo.framebuffer = frameBuffer;

        vkCmdBeginRenderPass(frameInfo.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = vks::initializers::viewport((float)m_Width, (float)m_Height, 0.0f, 1.0f);
        vkCmdSetViewport(frameInfo.commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = vks::initializers::rect2D(m_Width, m_Height, 0, 0);
        vkCmdSetScissor(frameInfo.commandBuffer, 0, 1, &scissor);

        VkDeviceSize offsets[1] = { 0 };

        // First sub pass
        // Renders the components of the scene to the G-Buffer attachments
        {

            vkCmdBindPipeline(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.offscreen);
            vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.offscreen, 0, 1, &descriptorSets.scene, 0, NULL);
            //models.scene.draw(frameInfo.commandBuffer);

            //vks::debugmarker::endRegion(drawCmdBuffers[i]);
        }

        // Second sub pass
        // This subpass will use the G-Buffer components that have been filled in the first subpass as input attachment for the final compositing
        {
            //vks::debugmarker::beginRegion(drawCmdBuffers[i], "Subpass 1: Deferred composition", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

            vkCmdNextSubpass(frameInfo.commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.composition);
            vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.composition, 0, 1, &descriptorSets.composition, 0, NULL);
            vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0);

            //vks::debugmarker::endRegion(drawCmdBuffers[i]);
        }

        // Third subpass
        // Render transparent geometry using a forward pass that compares against depth generated during G-Buffer fill
        {

            vkCmdNextSubpass(frameInfo.commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.transparent);
            vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.transparent, 0, 1, &descriptorSets.transparent, 0, NULL);
            //models.transparent.draw(drawCmdBuffers[i]);

        }


        vkCmdEndRenderPass(frameInfo.commandBuffer);
    }

    void DefferedRenderingSystem::clearAttachment(FrameBufferAttachment* attachment)
	{
		vkDestroyImageView(m_Device->device(), attachment->view, nullptr);
		vkDestroyImage(m_Device->device(), attachment->image, nullptr);
		vkFreeMemory(m_Device->device(), attachment->mem, nullptr);
	}

    void DefferedRenderingSystem::createAttachment(VkFormat format, VkImageUsageFlags usage, FrameBufferAttachment *attachment)
	{
		if (attachment->image != VK_NULL_HANDLE) {
			clearAttachment(attachment);
		}

		VkImageAspectFlags aspectMask = 0;
		VkImageLayout imageLayout;

		attachment->format = format;

		if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		{
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		assert(aspectMask > 0);

		VkImageCreateInfo image = {};
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = format;
		image.extent.width = attachments.width;
		image.extent.height = attachments.height;
		image.extent.depth = 1;
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		// VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT flag is required for input attachments
		image.usage = usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkMemoryAllocateInfo memAlloc = {};
		VkMemoryRequirements memReqs;

		vkCreateImage(m_Device->device(), &image, nullptr, &attachment->image);
        vkGetImageMemoryRequirements(m_Device->device(), attachment->image, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = m_Device->findMemoryType(memReqs.memoryTypeBits,
                                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        //memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (vkAllocateMemory(m_Device->device(), &memAlloc, nullptr, &attachment->mem) != VK_SUCCESS) {
            std::cout << "Failed to allocate memory" << std::endl;
        }
        if (vkBindImageMemory(m_Device->device(), attachment->image, attachment->mem, 0) != VK_SUCCESS) {
            std::cout << "Failed to bind memory" << std::endl;
        }

		VkImageViewCreateInfo imageView = {};
		imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageView.format = format;
		imageView.subresourceRange = {};
		imageView.subresourceRange.aspectMask = aspectMask;
		imageView.subresourceRange.baseMipLevel = 0;
		imageView.subresourceRange.levelCount = 1;
		imageView.subresourceRange.baseArrayLayer = 0;
		imageView.subresourceRange.layerCount = 1;
		imageView.image = attachment->image;
		vkCreateImageView(m_Device->device(), &imageView, nullptr, &attachment->view);
	}

	// Create color attachments for the G-Buffer components
	void DefferedRenderingSystem::createGBufferAttachments()
	{
		createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &attachments.position);	// (World space) Positions
		createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &attachments.normal);		// (World space) Normals
		createAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &attachments.albedo);			// Albedo (color)
	}

	// Override framebuffer setup from base class, will automatically be called upon setup and if a window is resized
	void DefferedRenderingSystem::setupFrameBuffer()
	{
		// If the window is resized, all the framebuffers/attachments used in our composition passes need to be recreated
		if (attachments.width != m_Width || attachments.height != m_Height) {
			attachments.width = m_Width;
			attachments.height = m_Height;
			createGBufferAttachments();
			// Since the framebuffers/attachments are referred in the descriptor sets, these need to be updated too
			// Composition pass
			std::vector< VkDescriptorImageInfo> descriptorImageInfos = {
				vks::initializers::descriptorImageInfo(VK_NULL_HANDLE, attachments.position.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
				vks::initializers::descriptorImageInfo(VK_NULL_HANDLE, attachments.normal.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
				vks::initializers::descriptorImageInfo(VK_NULL_HANDLE, attachments.albedo.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
			};
			std::vector<VkWriteDescriptorSet> writeDescriptorSets;
			for (size_t i = 0; i < descriptorImageInfos.size(); i++) {
				writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(descriptorSets.composition, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, i, &descriptorImageInfos[i]));
			}
			vkUpdateDescriptorSets(m_Device->device(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
			// Forward pass
			writeDescriptorSets = {
				vks::initializers::writeDescriptorSet(descriptorSets.transparent, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, &descriptorImageInfos[0]),
			};
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}

		VkImageView attachments[5];

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.renderPass = renderPass;
		frameBufferCreateInfo.attachmentCount = 5;
		frameBufferCreateInfo.pAttachments = attachments;
		frameBufferCreateInfo.width = width;
		frameBufferCreateInfo.height = height;
		frameBufferCreateInfo.layers = 1;

		// Create frame buffers for every swap chain image
        attachments[0] = swapChain.buffers[i].view;
        attachments[1] = this->attachments.position.view;
        attachments[2] = this->attachments.normal.view;
        attachments[3] = this->attachments.albedo.view;
        attachments[4] = depthStencil.view;
        VK_CHECK_RESULT(vkCreateFramebuffer(m_Device->device(), &frameBufferCreateInfo, nullptr, &frameBuffer));
	}
}

*/