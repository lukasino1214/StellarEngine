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
#include <chrono>

namespace Engine {

    FirstApp::FirstApp() { loadGameObjects(); }

    FirstApp::~FirstApp() {}

    void FirstApp::run() {
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
        Camera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(0.f, 0.f, 2.5f));

        Imgui lveImgui{lveWindow, lveDevice, lveRenderer.getSwapChainRenderPass(), lveRenderer.getImageCount()};

        auto viewerObject = GameObject::createGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = lveRenderer.getAspectRatio();
            //camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.setPerspectiveProjection(glm::radians(90.0f), aspect, 0.01, 1000.0f);

            if (auto commandBuffer = lveRenderer.beginFrame()) {
                lveImgui.newFrame();
                lveRenderer.beginSwapChainRenderPass(commandBuffer);

                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
                ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!" and append into it.
                ImGui::Text("This is some useful text.");  // Display some text (you can use a format strings too)
                ImGui::End();
                lveImgui.render(commandBuffer);

                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lveDevice.device());
    }



    void FirstApp::loadGameObjects() {
        std::shared_ptr<Model> model = Model::createModelfromFile(lveDevice, "models/smooth_vase.obj");

        auto gameObj = GameObject::createGameObject();
        gameObj.model = model;
        gameObj.transform.translation = {0.0f, 0.0f, 1.5f};
        gameObj.transform.scale = glm::vec3(3.0f);
        gameObjects.push_back(std::move(gameObj));
    }
}