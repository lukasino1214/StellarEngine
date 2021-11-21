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
        test.GetComponent<RigidBodyComponent>().acceleration = {0.0f, 0.05f, 0.0f};

        Entity test2 = m_ActiveScene->CreateEntity("Test 2");
        test2.GetComponent<TransformComponentLegacy>().SetRotation({glm::radians(180.0f), glm::radians(90.0f), 0.0f});
        test2.GetComponent<TransformComponentLegacy>().SetTranslation({1.1f, 3.0f, 3.5f});
        test2.AddComponent<ModelComponent>(model);
        test2.AddComponent<RigidBodyComponent>();
        test2.GetComponent<RigidBodyComponent>().changeStatus(false);

        Entity test3 = m_ActiveScene->CreateEntity("Test 3");
        test3.GetComponent<TransformComponentLegacy>().SetRotation({glm::radians(180.0f), glm::radians(90.0f), 0.0f});
        test3.GetComponent<TransformComponentLegacy>().SetTranslation({-1.3f, 3.0f, 3.5f});
        test3.AddComponent<ModelComponent>(model);
        test3.AddComponent<RigidBodyComponent>();
        test3.GetComponent<RigidBodyComponent>().changeStatus(false);

        Entity test4 = m_ActiveScene->CreateEntity("Test 4");
        test4.GetComponent<TransformComponentLegacy>().SetRotation({glm::radians(180.0f), glm::radians(90.0f), 0.0f});
        test4.GetComponent<TransformComponentLegacy>().SetTranslation({1.3f, 5.0f, 3.5f});
        test4.AddComponent<ModelComponent>(model);
        test4.AddComponent<RigidBodyComponent>();
        test4.GetComponent<RigidBodyComponent>().changeStatus(false);

        Entity test5 = m_ActiveScene->CreateEntity("Test 5");
        test5.GetComponent<TransformComponentLegacy>().SetRotation({glm::radians(180.0f), glm::radians(90.0f), 0.0f});
        test5.GetComponent<TransformComponentLegacy>().SetTranslation({-1.1f, 5.0f, 3.5f});
        test5.AddComponent<ModelComponent>(model);
        test5.AddComponent<RigidBodyComponent>();
        test5.GetComponent<RigidBodyComponent>().changeStatus(false);


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

        Imgui lveImgui{m_Window, m_Device, m_Renderer.getSwapChainRenderPass(), m_Renderer.getImageCount()};

        auto viewerObject = GameObject::createGameObject();
        viewerObject.transform.translation = {0.0f, -2.0f, -4.0f};
        KeyboardMovementController cameraController{};


        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!m_Window.shouldClose()) {
            glfwPollEvents();

            if(glfwGetKey(m_Window.getGLFWwindow(), GLFW_KEY_F) == GLFW_PRESS) {
                startPhysics = true;
            }

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(m_Window.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = m_Renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(90.0f), aspect, 0.01, 1000.0f);

            if (auto commandBuffer = m_Renderer.beginFrame()) {

                if(startPhysics) {

                auto DoSphereOverlap = [](Entity entity, Entity target) {
                    auto transfrom1 = entity.GetComponent<TransformComponentLegacy>().GetTranslation();
                    auto transfrom2 = target.GetComponent<TransformComponentLegacy>().GetTranslation();

                    auto radius1 = entity.GetComponent<RigidBodyComponent>().getRadius();
                    auto radius2 = target.GetComponent<RigidBodyComponent>().getRadius();

                    return glm::abs((transfrom2.x - transfrom1.x) * (transfrom2.x - transfrom1.x) + (transfrom2.y - transfrom1.y) * (transfrom2.y - transfrom1.y) + (transfrom2.z - transfrom1.z) * (transfrom2.z - transfrom1.z)) <= (radius1 + radius2) * (radius1 + radius2);
                };

                m_ActiveScene->m_Registry.each([&](auto entityID) {
                    Entity entity = {entityID, m_ActiveScene.get()};
                    if (!entity)
                        return;

                    auto acceleration = entity.GetComponent<RigidBodyComponent>().acceleration;
                    auto velocity = entity.GetComponent<RigidBodyComponent>().velocity;

                    acceleration.x = acceleration.x * 0.99f;
                    acceleration.y = acceleration.y * 0.99f;
                    acceleration.z = acceleration.z * 0.99f;

                    velocity.x = acceleration.x * (frameTime + 1);
                    velocity.y = acceleration.y * (frameTime + 1);
                    velocity.z = acceleration.z * (frameTime + 1);

                    auto position = entity.GetComponent<TransformComponentLegacy>().Translation;

                    position.x += velocity.x * (frameTime + 1);
                    position.y += velocity.y * (frameTime + 1);
                    position.z += velocity.z * (frameTime + 1);

                    entity.GetComponent<RigidBodyComponent>().acceleration = acceleration;
                    entity.GetComponent<RigidBodyComponent>().velocity = velocity;
                    entity.GetComponent<TransformComponentLegacy>().SetTranslation(position);
                });

                std::vector<std::pair<Entity*, Entity*>> pair;

                m_ActiveScene->m_Registry.each([&](auto entityID) {
                    Entity entity = { entityID, m_ActiveScene.get() };
                    if (!entity)
                        return;


                        m_ActiveScene->m_Registry.each([&](auto targetID) {
                            Entity target = {targetID, m_ActiveScene.get()};
                            if (!target)
                                return;

                            if(entity.GetComponent<IDComponent>().ID != target.GetComponent<IDComponent>().ID) {
                                //std::cout << entity.GetComponent<RigidBodyComponent>().hasStatic() << std::endl;
                                if(DoSphereOverlap(entity, target)) {

                                    pair.push_back({&entity, &target});

                                    auto transfrom1 = entity.GetComponent<TransformComponentLegacy>().GetTranslation();
                                    auto transfrom2 = target.GetComponent<TransformComponentLegacy>().GetTranslation();

                                    auto radius1 = entity.GetComponent<RigidBodyComponent>().getRadius();
                                    auto radius2 = target.GetComponent<RigidBodyComponent>().getRadius();

                                    float distance = glm::sqrt((transfrom2.x - transfrom1.x) * (transfrom2.x - transfrom1.x) + (transfrom2.y - transfrom1.y) * (transfrom2.y - transfrom1.y) + (transfrom2.z - transfrom1.z) * (transfrom2.z - transfrom1.z));
                                    float overlap = 0.5 * (distance - radius1 - radius2);

                                    auto entity_t = entity.GetComponent<TransformComponentLegacy>().GetTranslation();
                                    auto target_t = target.GetComponent<TransformComponentLegacy>().GetTranslation();

                                    entity_t.x -= overlap * (entity_t.x - target_t.x) / distance;
                                    entity_t.y -= overlap * (entity_t.y - target_t.y) / distance;
                                    entity_t.z -= overlap * (entity_t.z - target_t.z) / distance;

                                    target_t.x += overlap * (entity_t.x - target_t.x) / distance;
                                    target_t.y += overlap * (entity_t.y - target_t.y) / distance;
                                    target_t.z += overlap * (entity_t.z - target_t.z) / distance;

                                    entity.GetComponent<TransformComponentLegacy>().SetTranslation(entity_t);
                                    target.GetComponent<TransformComponentLegacy>().SetTranslation(target_t);



                                    auto entity_v = entity.GetComponent<RigidBodyComponent>().velocity;
                                    auto target_v = target.GetComponent<RigidBodyComponent>().velocity;

                                    auto nv1 = entity.GetComponent<RigidBodyComponent>().velocity;
                                    nv1 += projectUonV(target_v, target_t - entity_t);
                                    nv1 -= projectUonV(entity_v, entity_t - target_t);
                                    auto nv2 = target.GetComponent<RigidBodyComponent>().velocity;
                                    nv2 += projectUonV(entity_v, target_t - entity_t);
                                    nv2 -= projectUonV(target_v, entity_t - target_t);

                                    entity.GetComponent<RigidBodyComponent>().acceleration = nv1;
                                    target.GetComponent<RigidBodyComponent>().acceleration = nv2;
                                }
                            }
                        });
                });

                }
                /*for(auto c : pair) {
                    std::cout << "adwadwadaw" << std::endl;
                    Entity *entity = c.first;
                    Entity *target = c.second;

                    auto entity_t = entity->GetComponent<TransformComponentLegacy>().GetTranslation();
                    auto target_t = target->GetComponent<TransformComponentLegacy>().GetTranslation();

                    auto entity_v = entity->GetComponent<RigidBodyComponent>().velocity;
                    auto target_v = target->GetComponent<RigidBodyComponent>().velocity;

                    float distance = glm::sqrt((target_t.x - entity_t.x) * (target_t.x - entity_t.x) + (target_t.y - entity_t.y) * (target_t.y - entity_t.y) + (target_t.z - entity_t.z) * (target_t.z - entity_t.z));


                    // Normal
                    float nx = (target_t.x - entity_t.x) / distance;
                    float ny = (target_t.y - entity_t.y) / distance;
                    float nz = (target_t.z - entity_t.z) / distance;

                    //auto tan = glm::tan({nx, ny, nz});

                    auto nv1 = entity->GetComponent<RigidBodyComponent>().velocity;
                    nv1 += projectUonV(target_v, target_t - entity_t);
                    nv1 -= projectUonV(entity_v, entity_t - target_t);
                    auto nv2 = target->GetComponent<RigidBodyComponent>().velocity;
                    nv2 += projectUonV(entity_v, target_t - entity_t);
                    nv2 -= projectUonV(target_v, entity_t - target_t);

                    entity->GetComponent<RigidBodyComponent>().velocity = nv1;
                    target->GetComponent<RigidBodyComponent>().velocity = nv2;
                }*/




                int frameIndex = m_Renderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex]};

                // update
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                lveImgui.newFrame();
                m_Renderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo, m_ActiveScene);

                ImGui::Begin("Frame Info");
                ImGui::Text("Frame Time: %f", frameTime);
                ImGui::End();

                HierarchyPanel.OnImGuiRender();

                lveImgui.render(commandBuffer);
                m_Renderer.endSwapChainRenderPass(commandBuffer);
                m_Renderer.endFrame();

            }
        }

        vkDeviceWaitIdle(m_Device.device());
    }
}
