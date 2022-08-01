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
        postprocessing_system = std::make_unique<PostProcessingSystem>(device, WIDTH, HEIGHT);
        //shadow_system = std::make_unique<ShadowSystem>(device);
        camera = std::make_shared<Camera>(glm::vec3(5, 10, 5), glm::vec3(10, 10, 0));
        imgui_layer = std::make_unique<ImGuiLayer>(device, *window, renderer->get_swapchain_renderpass(), renderer->get_image_count());
        viewport_panel = std::make_shared<ViewportPanel>(scene_hierarchy_panel, camera, window, offscreen_system->get_sampler(), offscreen_system->get_image_view());

        deferred_rendering_system = std::make_unique<DeferredRenderingSystem>(device, WIDTH, HEIGHT);
        point_light_system = std::make_unique<PointLightSystem>(device, offscreen_system->get_renderpass());
        grid_system = std::make_unique<GridSystem>(device, offscreen_system->get_renderpass());

        pbr_system = std::make_unique<PBRSystem>(device, deferred_rendering_system->get_renderpass());

        auto helmet = std::make_shared<Model>(device, "assets/models/SciFiHelmet/glTF/SciFiHelmet.gltf");
        auto damaged_helmet = std::make_shared<Model>(device, "assets/models/DamagedHelmet/glTF/DamagedHelmet.gltf");

        /*auto entity = editor_scene->create_entity("HELP");
        auto script = std::make_shared<HelmetScript>(entity.get_handle(), editor_scene);
        entity.get_component<TransformComponent>().translation = {10, 8, 0};
        entity.add_component<ModelComponent>(helmet);
        entity.add_component<ScriptComponent>(script);*/

        auto test = editor_scene->create_entity("Test");
        test.get_component<TransformComponent>().set_translation(glm::vec3{10.0f, 5.0f, 0.0f});
        test.add_component<ModelComponent>(damaged_helmet);
        test.get_component<ModelComponent>().transparent = true;

        /*auto test1 = editor_scene->create_entity("Test 1");
        test1.get_component<RelationshipComponent>().parent = test;
        test1.get_component<TransformComponent>().set_translation(glm::vec3{5.0f, 3.0f, 0.0f});
        test1.add_component<ModelComponent>(helmet);

        auto test2 = editor_scene->create_entity("Test 2");
        test2.get_component<RelationshipComponent>().parent = test;
        test2.get_component<TransformComponent>().set_translation(glm::vec3{-5.0f, 3.0f, 0.0f});
        test2.add_component<ModelComponent>(helmet);

        test.get_component<RelationshipComponent>().children.push_back(test1);
        test.get_component<RelationshipComponent>().children.push_back(test2);*/

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

            VkDescriptorImageInfo irradiance_image_info = {};
            irradiance_image_info.sampler = pbr_system->get_sampler();
            irradiance_image_info.imageView = pbr_system->get_irradiance_image_view();
            irradiance_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkDescriptorImageInfo BRDFLUT_image_info = {};
            BRDFLUT_image_info.sampler = pbr_system->get_sampler();
            BRDFLUT_image_info.imageView = pbr_system->get_BRDFLUT_image_view();
            BRDFLUT_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkDescriptorImageInfo prefilteredMap_image_info = {};
            prefilteredMap_image_info.sampler = pbr_system->get_sampler();
            prefilteredMap_image_info.imageView = pbr_system->get_prefiltered_map_image_view();
            prefilteredMap_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkDescriptorImageInfo env_map_image_info = {};
            env_map_image_info.sampler = pbr_system->get_sampler();
            env_map_image_info.imageView = pbr_system->get_environment_map_image_view();
            env_map_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            DescriptorWriter(*Core::global_descriptor_set_layout, *Core::global_descriptor_pool)
                    .write_buffer(0, &buffer_info)
                    .write_image(1, &irradiance_image_info)
                    .write_image(2, &BRDFLUT_image_info)
                    .write_image(3, &prefilteredMap_image_info)
                    .write_image(4, &env_map_image_info)
                    .build(device, vk_global_descriptor_sets[i]);
        }

        /*VkDescriptorSet vk_shadow_descriptor_set;

        {
            VkDescriptorImageInfo image_info = {};
            image_info.sampler = shadow_system->get_sampler();
            image_info.imageView = shadow_system->get_image_view();
            image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

            DescriptorWriter(*Core::shadow_descriptor_set_layout, *Core::global_descriptor_pool)
                    .write_image(0, &image_info)
                    .build(device, vk_shadow_descriptor_set);
        }*/


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
            }

            auto new_time = std::chrono::high_resolution_clock::now();
            float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
            current_time = new_time;
            timer += frame_time;

            if (auto command_buffer = renderer->begin_frame()) {
                int frame_index = renderer->get_frame_index();
                FrameInfo frameInfo{frame_index, frame_time, command_buffer, vk_global_descriptor_sets[frame_index]};

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

                frameInfo.ubo = ubo;

                //shadow_system->render(frameInfo, editor_scene);
                deferred_rendering_system->start(frameInfo, editor_scene);
                pbr_system->render_skybox(frameInfo);
                deferred_rendering_system->end(frameInfo);

                /*offscreen_system->start(frameInfo);
                rendering_system->render(frameInfo, editor_scene);
                point_light_system->render(frameInfo, editor_scene);
                if (is_grid_enabled) {
                    grid_system->render(frameInfo);
                }
                offscreen_system->end(frameInfo);*/
                //postprocessing_system->render(frameInfo, offscreen_system->get_present_descriptor_set());
                postprocessing_system->render(frameInfo, deferred_rendering_system->get_present_descriptor_set());
                //postprocessing_system->render(frameInfo, pbr_system->vk_BRDFLUT_descriptor_set);
                //postprocessing_system->render(frameInfo, pbr_system->hdr_set);

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
