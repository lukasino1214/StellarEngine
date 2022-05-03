//
// Created by lukas on 08.11.21.
//

#include "App.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <chrono>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include "HelmetScript.h"

namespace Engine {
    FirstApp::FirstApp() {
        m_Window = std::make_shared<Window>(WIDTH, HEIGHT, "Stellar Engine");
        Core::Init();

        m_Renderer = std::make_unique<Renderer>(m_Window, Core::m_Device);
        m_EditorScene = std::make_shared<Scene>();
        m_ContentBrowserPanel = std::make_unique<ContentBrowserPanel>();
        m_SceneHierarchyPanel = std::make_shared<SceneHierarchyPanel>();
        m_DockSpacePanel = std::make_unique<DockSpacePanel>();
        m_OffScreenRenderingSystem = std::make_unique<OffScreen>(m_ViewportSize.x , m_ViewportSize.y);
        m_SimpleRenderSystem = std::make_unique<RenderSystem>(m_OffScreenRenderingSystem->GetRenderPass());
        m_PointLightSystem = std::make_unique<PointLightSystem>(m_OffScreenRenderingSystem->GetRenderPass());
        m_PostProcessingSystem = std::make_unique<PostProcessingSystem>(m_ViewportSize.x , m_ViewportSize.y);
        m_Camera = std::make_shared<Camera>(glm::vec3(5, 10, 5), glm::vec3(10, 10, 0));
        m_Imgui = std::make_unique<ImGuiLayer>(*m_Window, m_Renderer->getSwapChainRenderPass(), m_Renderer->getImageCount());


        Physics::Init();

        BodyInterface &body_interface = Physics::m_PhysicsSystem->GetBodyInterface();
        BoxShapeSettings floor_shape_settings(Vec3(100.0f, 1.0f, 100.0f));
        ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
        ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()
        BodyCreationSettings floor_settings(floor_shape, Vec3(0.0f, 0.0f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
        Body *floor = body_interface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr
        body_interface.AddBody(floor->GetID(), EActivation::DontActivate);
        BodyCreationSettings sphere_settings(new SphereShape(0.5f), Vec3(0.0f, 14.0f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
        sphere_id = body_interface.CreateAndAddBody(sphere_settings, EActivation::Activate);
        body_interface.SetLinearVelocity(sphere_id, Vec3(0.0f, -5.0f, 0.0f));
        Vec3 position = body_interface.GetCenterOfMassPosition(sphere_id);
        Vec3 velocity = body_interface.GetLinearVelocity(sphere_id);
        floor_id = floor->GetID();




        auto entity = m_EditorScene->CreateEntity("Helmet");
        auto helmet = std::make_shared<Model>("assets/models/SciFiHelmet/glTF/SciFiHelmet.gltf");
        auto script = std::make_shared<HelmetScript>(entity.GetHandle(), m_EditorScene);
        entity.GetComponent<TransformComponent>().Translation = {0, 8, 0};
        entity.AddComponent<ModelComponent>(helmet);
        entity.AddComponent<ScriptComponent>(script);
        entity.AddComponent<PhysicsComponent>(sphere_id);

        SceneSerializer serializer(m_EditorScene);
        serializer.Deserialize("assets/Example.scene");

        m_SceneHierarchyPanel->SetContext(m_EditorScene);
    }

    FirstApp::~FirstApp() {}

    void FirstApp::run() {
        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto & uboBuffer : uboBuffers) {
            uboBuffer = std::make_unique<Buffer>(
                    sizeof(GlobalUbo),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffer->map();
        }

        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            DescriptorWriter(*Core::m_GlobalSetLayout, *Core::m_GlobalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        BodyInterface &body_interface = Physics::m_PhysicsSystem->GetBodyInterface();

        {
            VkDescriptorImageInfo image_info = {};
            image_info.sampler = m_OffScreenRenderingSystem->GetSampler();
            image_info.imageView = m_OffScreenRenderingSystem->GetImageView();
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            DescriptorWriter(*Core::m_PostProcessingLayout, *Core::m_GlobalPool)
                .writeImage(0, &image_info)
                .build(PostProcessingSet);
        }

        auto image = ImGui_ImplVulkan_AddTexture(m_PostProcessingSystem->GetSampler(), m_PostProcessingSystem->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        auto currentTime = std::chrono::high_resolution_clock::now();

        bool resize = false;

        int m_GizmoType = 0;

        while (!m_Window->shouldClose()) {
            glfwPollEvents();

            if(resize) {
                m_OffScreenRenderingSystem->SetViewportSize(m_ViewportSize);
                m_PostProcessingSystem->SetViewportSize(m_ViewportSize);
                image = ImGui_ImplVulkan_AddTexture(m_PostProcessingSystem->GetSampler(), m_PostProcessingSystem->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                VkDescriptorImageInfo image_info = {};
                image_info.sampler = m_OffScreenRenderingSystem->GetSampler();
                image_info.imageView = m_OffScreenRenderingSystem->GetImageView();
                image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                DescriptorWriter(*Core::m_PostProcessingLayout, *Core::m_GlobalPool)
                        .writeImage(0, &image_info)
                        .build(PostProcessingSet);
                resize = false;
            }

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            m_Camera->Move(m_Window->getGLFWwindow(), frameTime);
            if(body_interface.IsActive(sphere_id) && startPhysics) {
                Physics::m_PhysicsSystem->Update();
            }

            if (auto commandBuffer = m_Renderer->beginFrame()) {
                int frameIndex = m_Renderer->getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, globalDescriptorSets[frameIndex]};

                // update
                GlobalUbo ubo{};
                ubo.projectionMat = m_Camera->getProjection();
                ubo.viewMat = m_Camera->getView();
                ubo.cameraPos = glm::vec4(m_Camera->getPosition(), 1.0f);
                ubo.numLights = 0;

                m_EditorScene->UpdateLightsUbo(ubo);
                m_EditorScene->OnUpdate(frameTime);

                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                m_OffScreenRenderingSystem->Start(frameInfo);
                m_SimpleRenderSystem->renderGameObjects(frameInfo, m_EditorScene);
                m_PointLightSystem->renderGameObjects(frameInfo, m_EditorScene);
                m_OffScreenRenderingSystem->End(frameInfo);
                m_PostProcessingSystem->Render(frameInfo, PostProcessingSet);

                m_Imgui->newFrame();
                m_Renderer->beginSwapChainRenderPass(commandBuffer); // VKBeginRenderPass

                m_DockSpacePanel->OnImGuiRender();
                m_ContentBrowserPanel->OnImGuiRender();
                m_SceneHierarchyPanel->OnImGuiRender();

                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
                ImGui::Begin("Viewport");
                ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
                if (m_ViewportSize != *((glm::vec2*)&viewportPanelSize)) {
                    resize = true;
                    std::cout << "test" << std::endl;
                    m_Camera->SetProjection(viewportPanelSize.x , viewportPanelSize.y);
                    m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
                }

                ImGui::Image(image, viewportPanelSize);

                if (glfwGetKey(m_Window->getGLFWwindow(), GLFW_KEY_U) == GLFW_PRESS) {
                    if (!ImGuizmo::IsUsing())
                        m_GizmoType = -1;
                }

                if (glfwGetKey(m_Window->getGLFWwindow(), GLFW_KEY_I) == GLFW_PRESS) {
                    if (!ImGuizmo::IsUsing())
                        m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                }

                if (glfwGetKey(m_Window->getGLFWwindow(), GLFW_KEY_O) == GLFW_PRESS) {
                    if (!ImGuizmo::IsUsing())
                        m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                }

                if (glfwGetKey(m_Window->getGLFWwindow(), GLFW_KEY_P) == GLFW_PRESS) {
                    if (!ImGuizmo::IsUsing())
                        m_GizmoType = ImGuizmo::OPERATION::SCALE;
                }

                Entity selectedEntity = m_SceneHierarchyPanel->GetSelectedEntity();
                if (selectedEntity && m_GizmoType != -1)
                {
                    ImGuizmo::SetOrthographic(false);
                    ImGuizmo::SetDrawlist();

                    auto& tc = selectedEntity.GetComponent<TransformComponent>();

                    float windowWidth = (float)ImGui::GetWindowWidth();
                    float windowHeight = (float)ImGui::GetWindowHeight();
                    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
                    glm::mat4 cameraView = glm::inverse(m_Camera->getView());
                    glm::mat4 cameraProj = m_Camera->getProjection();
                    TransformComponent transform = selectedEntity.GetComponent<TransformComponent>();

                    glm::mat4 mod_mat = transform.mat4();
                    ImGuizmo::Manipulate(glm::value_ptr(m_Camera->getView()), glm::value_ptr(cameraProj), (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(mod_mat));

                    if (ImGuizmo::IsUsing()) {
                        glm::vec3 translation, rotation, scale;
                        Math::DecomposeTransform(mod_mat, translation, rotation, scale);

                        tc.Translation += translation - transform.Translation;
                        tc.Rotation += rotation - transform.Rotation;
                        tc.Scale += scale - transform.Scale;
                    }
                }
                ImGui::End();
                ImGui::PopStyleVar();

                ImGui::Begin("Window");
                if(ImGui::Button("save scene")) {
                    SceneSerializer serializer(m_EditorScene);
                    serializer.Serialize("./assets/Example.scene");
                }
                if(ImGui::Button("Start Physics")) {
                    startPhysics = true;
                }
                ImGui::End();


                ImGuiIO &io = ImGui::GetIO();
                io.DisplaySize = ImVec2((float)1280, (float)720);

                m_Imgui->render(commandBuffer);
                m_Renderer->endSwapChainRenderPass(commandBuffer);  //vkEndRenderPass
                m_Renderer->endFrame();
            }
        }

        body_interface.RemoveBody(sphere_id);

        // Destroy the sphere. After this the sphere ID is no longer valid.
        body_interface.DestroyBody(sphere_id);

        // Remove and destroy the floor
        body_interface.RemoveBody(floor_id);
        body_interface.DestroyBody(floor_id);

        vkDeviceWaitIdle(Core::m_Device->device());
    }
}
