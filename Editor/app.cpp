#include "app.h"
#include "glm/ext/matrix_transform.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <chrono>
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include "helmet_script.h"
#include "../Engine/pgepch.h"


namespace Engine {
    App::App() {
        window = std::make_shared<Window>(WIDTH, HEIGHT, "Stellar Engine");
        device = std::make_shared<Device>(window.get());
        Core::init(device);
        InputManager::init(window->get_GLFWwindow());

        renderer = std::make_unique<Renderer>(window, device);
        editor_scene = std::make_shared<Scene>();
        content_browser_panel = std::make_unique<ContentBrowserPanel>(device);
        scene_hierarchy_panel = std::make_shared<SceneHierarchyPanel>();
        dock_space_panel = std::make_unique<DockSpacePanel>();
        offscreen_system = std::make_unique<OffScreenSystem>(device, WIDTH, HEIGHT);
        rendering_system = std::make_unique<RenderSystem>(device, offscreen_system->get_renderpass());
        point_light_system = std::make_unique<PointLightSystem>(device, offscreen_system->get_renderpass());
        postprocessing_system = std::make_unique<PostProcessingSystem>(device, WIDTH, HEIGHT);
        grid_system = std::make_unique<GridSystem>(device, offscreen_system->get_renderpass());
        shadow_system = std::make_unique<ShadowSystem>(device);
        camera = std::make_shared<Camera>(glm::vec3(5, 10, 5), glm::vec3(10, 10, 0));
        imgui_layer = std::make_unique<ImGuiLayer>(device, *window, renderer->get_swapchain_renderpass(), renderer->get_image_count());
        viewport_panel = std::make_shared<ViewportPanel>(scene_hierarchy_panel, camera, window, offscreen_system->get_sampler(), offscreen_system->get_image_view());


        auto helmet = std::make_shared<Model>(device, "assets/models/SciFiHelmet/glTF/SciFiHelmet.gltf");

        auto entity = editor_scene->create_entity("HELP");
        auto script = std::make_shared<HelmetScript>(entity.get_handle(), editor_scene);
        entity.get_component<TransformComponent>().translation = {10, 8, 0};
        entity.add_component<ModelComponent>(helmet);
        entity.add_component<ScriptComponent>(script);

        auto camera = editor_scene->create_entity("Camera");

        camera.add_component<CameraComponent>();
        camera.get_component<TransformComponent>().translation = glm::vec3{10.0f, 16.0f, 0.0f};

        auto test = editor_scene->create_entity("Test");
        test.get_component<TransformComponent>().set_translation(glm::vec3{10.0f, 5.0f, 0.0f});
        test.add_component<ModelComponent>(helmet);

        auto test1 = editor_scene->create_entity("Test 1");
        test1.get_component<RelationshipComponent>().parent = test;
        test1.get_component<TransformComponent>().set_translation(glm::vec3{5.0f, 3.0f, 0.0f});
        test1.add_component<ModelComponent>(helmet);

        auto test2 = editor_scene->create_entity("Test 2");
        test2.get_component<RelationshipComponent>().parent = test;
        test2.get_component<TransformComponent>().set_translation(glm::vec3{-5.0f, 3.0f, 0.0f});
        test2.add_component<ModelComponent>(helmet);

        test.get_component<RelationshipComponent>().children.push_back(test1);
        test.get_component<RelationshipComponent>().children.push_back(test2);

        SceneSerializer serializer(editor_scene);
        serializer.deserialize(device, "assets/Example.scene");

        scene_hierarchy_panel->set_context(editor_scene);
    }

    App::~App() {}

    void App::run() {
        std::vector<std::unique_ptr<Buffer>> ubo_buffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto &uboBuffer: ubo_buffers) {
            uboBuffer = std::make_unique<Buffer>(device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffer->map();
        }

        std::vector<VkDescriptorSet> vk_global_descriptor_sets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < vk_global_descriptor_sets.size(); i++) {
            auto buffer_info = ubo_buffers[i]->get_descriptor_info();
            DescriptorWriter(*Core::global_descriptor_set_layout, *Core::global_descriptor_pool)
                    .write_buffer(0, &buffer_info)
                    .build(device, vk_global_descriptor_sets[i]);
        }

        {
            VkDescriptorImageInfo image_info = {};
            image_info.sampler = offscreen_system->get_sampler();
            image_info.imageView = offscreen_system->get_image_view();
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            DescriptorWriter(*Core::postprocessing_descriptor_set_layout, *Core::global_descriptor_pool)
                    .write_image(0, &image_info)
                    .build(device, vk_post_processing_descriptor_set);
        }

        VkDescriptorSet vk_shadow_descriptor_set;

        {
            VkDescriptorImageInfo image_info = {};
            image_info.sampler = shadow_system->get_sampler();
            image_info.imageView = shadow_system->get_image_view();
            image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

            DescriptorWriter(*Core::shadow_descriptor_set_layout, *Core::global_descriptor_pool)
                    .write_image(0, &image_info)
                    .build(device, vk_shadow_descriptor_set);
        }


        bool is_grid_enabled = true;

        auto current_time = std::chrono::high_resolution_clock::now();

        float timer = 0.0;

        glm::vec3 light = {10.0, 10.0, 10.0};

        while (!window->should_close()) {
            glfwPollEvents();

            if (viewport_panel->resized()) {
                offscreen_system->resize(viewport_panel->get_viewport_size().x, viewport_panel->get_viewport_size().y);
                postprocessing_system->resize(viewport_panel->get_viewport_size().x, viewport_panel->get_viewport_size().y);
                viewport_panel->update_image(postprocessing_system->get_sampler(), postprocessing_system->get_image_view());

                VkDescriptorImageInfo image_info = {};
                image_info.sampler = offscreen_system->get_sampler();
                image_info.imageView = offscreen_system->get_image_view();
                image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                DescriptorWriter(*Core::postprocessing_descriptor_set_layout, *Core::global_descriptor_pool)
                        .write_image(0, &image_info)
                        .build(device, vk_post_processing_descriptor_set);
            }

            auto new_time = std::chrono::high_resolution_clock::now();
            float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
            current_time = new_time;
            timer += frame_time;

            if (auto command_buffer = renderer->begin_frame()) {
                int frame_index = renderer->get_frame_index();
                FrameInfo frameInfo{frame_index, frame_time, command_buffer, vk_global_descriptor_sets[frame_index], vk_shadow_descriptor_set};

                GlobalUbo ubo = {};
                ubo.projection_matrix = camera->getProjection();
                ubo.view_matrix = camera->getView();
                ubo.camera_position = glm::vec4(camera->getPosition(), 1.0f);
                ubo.num_point_lights = 0;
                ubo.num_directional_lights = 1;
                ubo.directional_lights[0].position = {50.0, 180.0, 50.0, 0.0};
                ubo.directional_lights[0].mvp = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 1000.0f) * glm::lookAt(light, glm::vec3(0.0f), glm::vec3(0, 1, 0));

                ubo.screen_width = viewport_panel->get_viewport_size().x;
                ubo.screen_height = viewport_panel->get_viewport_size().y;

                editor_scene->update_lights_ubo(ubo);
                editor_scene->update(frame_time);
                editor_scene->update_transforms();

                ubo_buffers[frame_index]->write_to_buffer(&ubo);
                ubo_buffers[frame_index]->flush();

                shadow_system->render(frameInfo, editor_scene);
                offscreen_system->start(frameInfo);
                rendering_system->render(frameInfo, editor_scene);
                point_light_system->render(frameInfo, editor_scene);
                if (is_grid_enabled) {
                    grid_system->render(frameInfo);
                }
                offscreen_system->end(frameInfo);
                postprocessing_system->render(frameInfo, vk_post_processing_descriptor_set);

                imgui_layer->new_frame();
                renderer->begin_swapchain_renderpass(command_buffer);

                dock_space_panel->render();
                content_browser_panel->render();
                scene_hierarchy_panel->render();
                viewport_panel->render(frame_time);

                ImGui::Begin("Window");
                if (ImGui::Button("save scene")) {
                    SceneSerializer serializer(editor_scene);
                    serializer.serialize("./assets/Example.scene");
                }

                ImGui::Checkbox("Grid", &is_grid_enabled);
                ImGui::End();

                ImGuiIO &io = ImGui::GetIO();
                io.DisplaySize = ImVec2((float) 1280, (float) 720);

                imgui_layer->render(command_buffer);
                renderer->end_swapchain_renderpass(command_buffer);
                renderer->end_frame();
            }
        }

        vkDeviceWaitIdle(device->vk_device);
    }
}
