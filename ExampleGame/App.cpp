//
// Created by lukas on 08.11.21.
//

#include "App.h"
#include "Jolt/Physics/Character/Character.h"
#include "glm/ext/matrix_transform.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <chrono>
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

namespace Engine {
    FirstApp::FirstApp() {
        m_Window = std::make_shared<Window>(WIDTH, HEIGHT, "Stellar Engine");
        m_Device = std::make_shared<Device>(m_Window.get());
        Core::Init(m_Device);
        Physics::Init();
        InputManager::Init(m_Window->getGLFWwindow());

        m_Renderer = std::make_unique<Renderer>(m_Window, m_Device);
        m_EditorScene = std::make_shared<Scene>();
        m_OffScreenRenderingSystem = std::make_unique<OffScreen>(m_Device, WIDTH, HEIGHT);
        m_SimpleRenderSystem = std::make_unique<RenderSystem>(m_Device, m_OffScreenRenderingSystem->GetRenderPass());
        m_PointLightSystem = std::make_unique<PointLightSystem>(m_Device, m_OffScreenRenderingSystem->GetRenderPass());
        m_PostProcessingSystem = std::make_unique<PostProcessingSystem>(m_Device, WIDTH, HEIGHT);
        m_ShadowSystem = std::make_unique<ShadowSystem>(m_Device);
        m_Camera = std::make_shared<Camera>(glm::vec3(5, 10, 5), glm::vec3(10, 10, 0));


        BodyInterface &body_interface = Physics::m_PhysicsSystem->GetBodyInterface();
        BoxShapeSettings floor_shape_settings(Vec3(100.0f, 1.0f, 100.0f));
        ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
        ShapeRefC floor_shape = floor_shape_result.Get();
        BodyCreationSettings floor_settings(floor_shape, Vec3(0.0f, -1.0f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
        Body *floor = body_interface.CreateBody(floor_settings);
        body_interface.AddBody(floor->GetID(), EActivation::DontActivate);
        floor_id = floor->GetID();
        BodyCreationSettings sphere_settings(new SphereShape(1.25f), Vec3(10.0f, 10.0f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
        sphere_id = body_interface.CreateAndAddBody(sphere_settings, EActivation::Activate);
        body_interface.SetLinearVelocity(sphere_id, Vec3(0.0f, -5.0f, 0.0f));


        auto entity = m_EditorScene->CreateEntity("HELP");
        auto helmet = std::make_shared<Model>(m_Device, "assets/models/SciFiHelmet/glTF/SciFiHelmet.gltf");
        //auto script = std::make_shared<HelmetScript>(entity.GetHandle(), m_EditorScene);
        entity.GetComponent<TransformComponent>().Translation = {10, 8, 0};
        entity.AddComponent<ModelComponent>(helmet);
        //entity.AddComponent<ScriptComponent>(script);
        entity.AddComponent<PhysicsComponent>(sphere_id);


        auto camera = m_EditorScene->CreateEntity("Camera");

        camera.AddComponent<CameraComponent>();
        camera.GetComponent<TransformComponent>().Translation = glm::vec3{10.0f, 16.0f, 0.0f};

        SceneSerializer serializer(m_EditorScene);
        serializer.Deserialize(m_Device, "assets/Example.scene");
    }

    FirstApp::~FirstApp() {}

    void FirstApp::run() {
        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto &uboBuffer: uboBuffers) {
            uboBuffer = std::make_unique<Buffer>(
                    m_Device,
                    sizeof(GlobalUbo),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffer->map();
        }

        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            DescriptorWriter(*Core::m_GlobalSetLayout, *Core::m_GlobalPool)
                    .writeBuffer(0, &bufferInfo)
                    .build(m_Device, globalDescriptorSets[i]);
        }

        BodyInterface &body_interface = Physics::m_PhysicsSystem->GetBodyInterface();

        {
            VkDescriptorImageInfo image_info = {};
            image_info.sampler = m_OffScreenRenderingSystem->GetSampler();
            image_info.imageView = m_OffScreenRenderingSystem->GetImageView();
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            DescriptorWriter(*Core::m_PostProcessingLayout, *Core::m_GlobalPool)
                    .writeImage(0, &image_info)
                    .build(m_Device, PostProcessingSet);
        }

        VkDescriptorSet set;

        {
            VkDescriptorImageInfo image_info = {};
            image_info.sampler = m_ShadowSystem->GetSampler();
            image_info.imageView = m_ShadowSystem->GetImageView();
            image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

            DescriptorWriter(*Core::m_ShadowLayout, *Core::m_GlobalPool)
                    .writeImage(0, &image_info)
                    .build(m_Device, set);
        }


        bool grid = true;

        auto currentTime = std::chrono::high_resolution_clock::now();

        float timer = 0.0;

        glm::vec3 light = {10.0, 10.0, 10.0};

        while (!m_Window->shouldClose()) {
            glfwPollEvents();

            /*if (m_ViewPortPanel->ShouldResize()) {
                m_OffScreenRenderingSystem->SetViewportSize(m_ViewPortPanel->GetViewPortSize());
                m_PostProcessingSystem->SetViewportSize(m_ViewPortPanel->GetViewPortSize());
                m_ViewPortPanel->UpdateImage(m_PostProcessingSystem->GetSampler(), m_PostProcessingSystem->GetImageView());

                VkDescriptorImageInfo image_info = {};
                image_info.sampler = m_OffScreenRenderingSystem->GetSampler();
                image_info.imageView = m_OffScreenRenderingSystem->GetImageView();
                image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                DescriptorWriter(*Core::m_PostProcessingLayout, *Core::m_GlobalPool)
                        .writeImage(0, &image_info)
                        .build(m_Device, PostProcessingSet);
            }*/

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            timer += frameTime;

            if(body_interface.IsActive(sphere_id) && m_StartPhysics) {
                Physics::m_PhysicsSystem->Update();
            }

            m_Camera->Move(m_Window->getGLFWwindow(), frameTime);

            glm::mat4 view;
            //view = glm::rotate(view, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

           /* m_EditorScene->m_Registry.each([&](auto entityID) {
                Entity entity = {entityID, m_EditorScene.get()};
                if (!entity)
                    return;

                if (entity.HasComponent<CameraComponent>()) {
                    glm::vec3 pos;
                    glm::vec3 rot;
                    if (entity.HasComponent<TransformComponent>()) {
                        auto trans = entity.GetComponent<TransformComponent>();
                        //view = script.View(glm::vec3{10.0f, 16.0f, 0.0f}, glm::vec3(0.0f, 0.0f, 0.0f));
                        pos = trans.Translation;
                        rot = trans.Rotation;
                    }

                    auto script = entity.GetComponent<CameraComponent>();
                    if(script.mainCamera) {
                        view = script.View(pos, rot);
                    }
                }
            });*/

            if (auto commandBuffer = m_Renderer->beginFrame()) {
                int frameIndex = m_Renderer->getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, globalDescriptorSets[frameIndex], set};
            
                GlobalUbo ubo{};
                ubo.projectionMat = m_Camera->getProjection();
                ubo.viewMat = m_Camera->getView();
                //ubo.viewMat = view;
                ubo.cameraPos = glm::vec4(m_Camera->getPosition(), 1.0f);
                ubo.numPointLights = 0;
                ubo.numDirectionalLights = 1;
                ubo.directionalLights[0].position = {50.0, 180.0, 50.0, 0.0};
                ubo.directionalLights[0].mvp = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 1000.0f) *
                                               glm::lookAt(light, glm::vec3(0.0f), glm::vec3(0, 1, 0));

                m_EditorScene->UpdateLightsUbo(ubo);
                m_EditorScene->OnUpdate(frameTime);

                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                m_ShadowSystem->Render(frameInfo, m_EditorScene);
                m_OffScreenRenderingSystem->Start(frameInfo);
                m_SimpleRenderSystem->renderGameObjects(frameInfo, m_EditorScene);
                m_PointLightSystem->renderGameObjects(frameInfo, m_EditorScene);
                m_OffScreenRenderingSystem->End(frameInfo);
                //m_PostProcessingSystem->Render(frameInfo, PostProcessingSet);

                m_Renderer->beginSwapChainRenderPass(commandBuffer); // VKBeginRenderPass
                m_PostProcessingSystem->RenderWithoutRenderpass(frameInfo, PostProcessingSet);
                m_Renderer->endSwapChainRenderPass(commandBuffer);  //vkEndRenderPass
                m_Renderer->endFrame();
            }
        }

        // Remove the sphere from the physics system. Note that the sphere itself keeps all of its state and can be re-added at any time.
        body_interface.RemoveBody(sphere_id);

        // Destroy the sphere. After this the sphere ID is no longer valid.
        body_interface.DestroyBody(sphere_id);

        // Remove and destroy the floor
        body_interface.RemoveBody(floor_id);
        body_interface.DestroyBody(floor_id);

        vkDeviceWaitIdle(m_Device->device());
    }
}
