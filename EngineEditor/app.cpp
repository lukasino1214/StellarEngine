//
// Created by lukas on 08.11.21.
//

#include "app.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <math.h>

namespace Engine {

    struct GlobalUbo {
        glm::mat4 projectionView{1.0f};
        glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f};
        glm::vec3 lightPosition{-1.0f};
        alignas(16) glm::vec4 lightColor{1.0f};
    };

    FirstApp::FirstApp() {
        globalPool = DescriptorPool::Builder(m_Device)
                .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                .build();

        m_ActiveScene = CreateRef<Scene>();

        Entity test = m_ActiveScene->CreateEntity("Test");
        test.GetComponent<TransformComponentLegacy>().SetRotation({glm::radians(180.0f), glm::radians(90.0f), 0.0f});
        test.GetComponent<TransformComponentLegacy>().SetTranslation({0.0f, 0.0f, 3.5f});

        std::shared_ptr<Model> model = Model::createModelfromFile(m_Device, "assets/models/sphere.obj");

        test.AddComponent<ModelComponent>(model);
        test.AddComponent<RigidBodyComponent>();
        test.GetComponent<RigidBodyComponent>().changeStatus(true);
        test.GetComponent<RigidBodyComponent>().acceleration = {0.0f, 5.0f, 0.0f};

        Entity test2 = m_ActiveScene->CreateEntity("Test 2");
        test2.GetComponent<TransformComponentLegacy>().SetRotation({glm::radians(180.0f), glm::radians(90.0f), 0.0f});
        test2.GetComponent<TransformComponentLegacy>().SetTranslation({1.1f, 3.0f, 3.5f});
        test2.AddComponent<ModelComponent>(model);
        test2.AddComponent<RigidBodyComponent>();
        test2.GetComponent<RigidBodyComponent>().changeStatus(false);
        test2.GetComponent<RigidBodyComponent>().acceleration = {-5.0f, 5.0f, 0.0f};

        Entity test3 = m_ActiveScene->CreateEntity("Test 3");
        test3.GetComponent<TransformComponentLegacy>().SetRotation({glm::radians(180.0f), glm::radians(90.0f), 0.0f});
        test3.GetComponent<TransformComponentLegacy>().SetTranslation({-1.3f, 3.0f, 3.5f});
        test3.AddComponent<ModelComponent>(model);
        test3.AddComponent<RigidBodyComponent>();
        test3.GetComponent<RigidBodyComponent>().changeStatus(false);
        test3.GetComponent<RigidBodyComponent>().acceleration = {5.0f, -5.0f, 0.0f};

        Entity test4 = m_ActiveScene->CreateEntity("Test 4");
        test4.GetComponent<TransformComponentLegacy>().SetRotation({glm::radians(180.0f), glm::radians(90.0f), 0.0f});
        test4.GetComponent<TransformComponentLegacy>().SetTranslation({1.3f, 5.0f, 3.5f});
        test4.AddComponent<ModelComponent>(model);
        test4.AddComponent<RigidBodyComponent>();
        test4.GetComponent<RigidBodyComponent>().changeStatus(false);
        test4.GetComponent<RigidBodyComponent>().acceleration = {0.0f, 5.0f, 5.0f};

        Entity test5 = m_ActiveScene->CreateEntity("Test 5");
        test5.GetComponent<TransformComponentLegacy>().SetRotation({glm::radians(180.0f), glm::radians(90.0f), 0.0f});
        test5.GetComponent<TransformComponentLegacy>().SetTranslation({-1.1f, 5.0f, 3.5f});
        test5.AddComponent<ModelComponent>(model);
        test5.AddComponent<RigidBodyComponent>();
        test5.GetComponent<RigidBodyComponent>().changeStatus(false);
        test5.GetComponent<RigidBodyComponent>().acceleration = {4.0f, 0.0f, 5.0f};


        HierarchyPanel.SetContext(m_ActiveScene);
    }

    FirstApp::~FirstApp() {}

    void FirstApp::run() {
        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<Buffer>(
                    m_Device,
                    sizeof(GlobalUbo),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout = DescriptorSetLayout::Builder(m_Device)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            DescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        RenderSystem simpleRenderSystem{m_Device, m_Renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        Camera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(0.f, 0.f, 2.5f));

        Imgui m_Imgui{m_Window, m_Device, m_Renderer.getSwapChainRenderPass(), m_Renderer.getImageCount()};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!m_Window.shouldClose()) {
            glfwPollEvents();

            if(glfwGetKey(m_Window.getGLFWwindow(), GLFW_KEY_F) == GLFW_PRESS) {
                startPhysics = true;
            }

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            camera.Move(m_Window.getGLFWwindow(), frameTime);
            //camera.setViewYXZ();

            float aspect = m_Renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(90.0f), aspect, 0.01, 1000.0f);

            if (auto commandBuffer = m_Renderer.beginFrame()) {

                if(startPhysics) {
                    PhysicsSystem test{};
                    test.Update(m_ActiveScene, frameTime);
                }

                int frameIndex = m_Renderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex]};

                // update
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                m_Imgui.newFrame();
                m_Renderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo, m_ActiveScene);

                ImGui::Begin("Scene Info and Control");
                ImGui::Text("Frame Time: %f", frameTime);
                ImGui::End();

                HierarchyPanel.OnImGuiRender();

                m_Imgui.render(commandBuffer);
                m_Renderer.endSwapChainRenderPass(commandBuffer);
                m_Renderer.endFrame();

            }
        }

        vkDeviceWaitIdle(m_Device.device());
    }
}
