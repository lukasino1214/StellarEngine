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
        glm::mat4 projectionMat{1.0f};
        glm::mat4 viewMat{1.0f};
        glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f};
        glm::vec3 lightPosition{-1.0f};
        alignas(16) glm::vec4 lightColor{1.0f};
        glm::vec3 cameraPos{0.0f, 0.0f, 0.0f};
    };

    FirstApp::FirstApp() {
        globalPool = DescriptorPool::Builder(m_Device)
                .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                .build();

        m_EditorScene = CreateRef<Scene>();


        Entity test = m_EditorScene->CreateEntity("Test");
        test.GetComponent<TransformComponent>().SetRotation({glm::radians(180.0f), glm::radians(90.0f), 0.0f});
        test.GetComponent<TransformComponent>().SetTranslation({0.0f, 0.0f, 3.5f});

        //std::shared_ptr<Model> model = Model::createModelfromFile(m_Device, "assets/models/sphere.obj");
        Ref<Model> model = CreateRef<Model>(m_Device, "assets/models/sphere.obj");

        test.AddComponent<ModelComponent>(model);
        test.AddComponent<RigidBodyComponent>();
        test.GetComponent<RigidBodyComponent>().acceleration = {0.0f, 5.0f, 0.0f};

        Entity test2 = m_EditorScene->CreateEntity("Test 2");
        test2.GetComponent<TransformComponent>().SetRotation({glm::radians(180.0f), glm::radians(90.0f), 0.0f});
        test2.GetComponent<TransformComponent>().SetTranslation({1.1f, 3.0f, 3.5f});
        test2.AddComponent<ModelComponent>(model);
        test2.AddComponent<RigidBodyComponent>();
        test2.GetComponent<RigidBodyComponent>().acceleration = {-5.0f, 5.0f, 0.0f};

        Entity test3 = m_EditorScene->CreateEntity("Test 3");
        test3.GetComponent<TransformComponent>().SetRotation({glm::radians(180.0f), glm::radians(90.0f), 0.0f});
        test3.GetComponent<TransformComponent>().SetTranslation({-1.3f, 3.0f, 3.5f});
        test3.AddComponent<ModelComponent>(model);
        test3.AddComponent<RigidBodyComponent>();
        test3.GetComponent<RigidBodyComponent>().acceleration = {5.0f, -5.0f, 0.0f};

        Entity test4 = m_EditorScene->CreateEntity("Test 4");
        test4.GetComponent<TransformComponent>().SetRotation({glm::radians(180.0f), glm::radians(90.0f), 0.0f});
        test4.GetComponent<TransformComponent>().SetTranslation({1.3f, 5.0f, 3.5f});
        test4.AddComponent<ModelComponent>(model);
        test4.AddComponent<RigidBodyComponent>();
        test4.GetComponent<RigidBodyComponent>().acceleration = {0.0f, 5.0f, 5.0f};

        Entity test5 = m_EditorScene->CreateEntity("Test 5");
        test5.GetComponent<TransformComponent>().SetRotation({glm::radians(180.0f), glm::radians(90.0f), 0.0f});
        test5.GetComponent<TransformComponent>().SetTranslation({-1.1f, 5.0f, 3.5f});
        test5.AddComponent<ModelComponent>(model);
        test5.AddComponent<RigidBodyComponent>();
        test5.GetComponent<RigidBodyComponent>().acceleration = {4.0f, 0.0f, 5.0f};

        /*SceneSerializer serializer(m_EditorScene);
        serializer.Deserialize("Example.scene");*/

        /*SceneSerializer serializer(m_EditorScene);
        serializer.Deserialize("Example.scene", m_Device);*/


        HierarchyPanel.SetContext(m_EditorScene);
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

        OffScreen screen(m_Device);

        //RenderSystem simpleRenderSystem{m_Device, m_Renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        //GridSystem gridsystem{m_Device,m_Renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};

        //GridSystem ScreenGridsystem{m_Device,screen.GetRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        RenderSystem ScreenSimpleRenderSystem{m_Device, screen.GetRenderPass(), globalSetLayout->getDescriptorSetLayout()};

        Camera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(0.f, 0.f, 2.5f));

        Imgui m_Imgui{m_Window, m_Device, m_Renderer.getSwapChainRenderPass(), m_Renderer.getImageCount()};

        auto image = ImGui_ImplVulkan_AddTexture(screen.GetSampler(), screen.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        auto currentTime = std::chrono::high_resolution_clock::now();
        bool scenePlaying = false;

        bool p_open = true;
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

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
                int frameIndex = m_Renderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex]};

                // update
                GlobalUbo ubo{};
                ubo.projectionMat = camera.getProjection();
                ubo.viewMat = camera.getView();
                ubo.cameraPos = camera.getPosition();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                //TODO: RENDERING
                screen.Start(frameInfo);
                ScreenSimpleRenderSystem.renderGameObjects(frameInfo, m_EditorScene);
                screen.End(frameInfo);

                // render
                m_Imgui.newFrame();
                m_Renderer.beginSwapChainRenderPass(commandBuffer); // VKBeginRenderPass
                // Docking
                {
                    if (opt_fullscreen) {
                        const ImGuiViewport* viewport = ImGui::GetMainViewport();
                        ImGui::SetNextWindowPos(viewport->WorkPos);
                        ImGui::SetNextWindowSize(viewport->WorkSize);
                        ImGui::SetNextWindowViewport(viewport->ID);
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
                    }
                    else {
                        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
                    }

                    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                        window_flags |= ImGuiWindowFlags_NoBackground;

                    if (!opt_padding)
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
                    ImGui::Begin("DockSpace Demo", &p_open, window_flags);
                    if (!opt_padding)
                        ImGui::PopStyleVar();

                    if (opt_fullscreen)
                        ImGui::PopStyleVar(2);

                    // DockSpace
                    ImGuiIO& io = ImGui::GetIO();
                    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
                        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
                    }
                    else {

                    }

                    if (ImGui::BeginMenuBar()) {
                        if (ImGui::BeginMenu("File")) {

                            ImGui::EndMenu();
                        }

                        ImGui::EndMenuBar();
                    }
                }

                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                ImGui::Begin("Viewport");
                auto viewportSize = ImGui::GetContentRegionAvail();
                if (m_ViewportSize != *((glm::vec2*)&viewportSize)) {
                    //screen.SetViewportSize({m_ViewportSize.x , m_ViewportSize.y});
                    //m_Camera->SetProjection(viewportSize.x, viewportSize.y);
                    camera.SetProjection(m_ViewportSize.x , m_ViewportSize.y);
                    m_ViewportSize = { viewportSize.x, viewportSize.y };
                }
                ImGui::Image(image, { viewportSize.x, viewportSize.y });
                ImGui::End();
                ImGui::PopStyleVar();

                ImGui::Begin("Scene Info and Control");
                ImGui::Text("Frame Time: %f", frameTime);
                if(!scenePlaying) {
                    //simpleRenderSystem.renderGameObjects(frameInfo, m_EditorScene);
                    if(ImGui::Button("Play")) {
                        scenePlaying = true;
                    }
                }

                else {
                    //PhysicsSystem test{};
                    //test.Update(m_EditorScene, frameTime);
                    //simpleRenderSystem.renderGameObjects(frameInfo, m_EditorScene);
                    if(ImGui::Button("Stop")) {
                        scenePlaying = false;
                        m_EditorScene = m_EditorScene;
                    }
                }
                if(ImGui::Button("Save")) {
                    SceneSerializer serializer(m_EditorScene);
                    serializer.Serialize("Example.scene");
                }

                if(ImGui::Button("Load")) {
                    std::cout << "0" << std::endl;
                    m_EditorScene = CreateRef<Scene>();
                    SceneSerializer serializer(m_EditorScene);
                    serializer.Deserialize("Example.scene", m_Device);
                    HierarchyPanel.SetContext(m_EditorScene);
                }
                ImGui::End();

                ImGui::Begin("File Browser");
                ImGui::Text("Work in progress");
                ImGui::End();


                HierarchyPanel.OnImGuiRender();

                ImGuiIO &io = ImGui::GetIO();
                io.DisplaySize = ImVec2((float)1280, (float)720);

                m_Imgui.render(commandBuffer);
                /*if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                    GLFWwindow *backup_current_context = glfwGetCurrentContext();
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                    glfwMakeContextCurrent(backup_current_context);
                }*/

                m_Renderer.endSwapChainRenderPass(commandBuffer);  //vkEndRenderPass
                m_Renderer.endFrame();

            }
        }

        vkDeviceWaitIdle(m_Device.device());
    }
}
