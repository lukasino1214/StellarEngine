//
// Created by lukas on 07.11.21.
//

#include "SceneHierarchyPanel.h"

#include "../../Vendor/imgui/imgui.h"
#include "../../Vendor/imgui/imgui_internal.h"

#include <glm/gtc/type_ptr.hpp>

#include <cstring>
#include <iostream>

#include <filesystem>
#include <string>

namespace Engine {

    SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene> &context) {
        SetContext(context);
    }

    void SceneHierarchyPanel::SetContext(const std::shared_ptr<Scene> &context) {
        m_Context = context;
        m_SelectionContext = {};
    }

    void SceneHierarchyPanel::OnImGuiRender() {
        ImGui::Begin("Scene Hierarchy");

        if (m_Context) {
            m_Context->m_Registry.each([&](auto entityID) {
                Entity entity{entityID, m_Context.get()};
                if (entity.HasComponent<RelationshipComponent>()) {
                    if (entity.GetComponent<RelationshipComponent>().parent == entt::null) {
                        DrawEntityNode(entity);
                    }
                }
            });

            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
                m_SelectionContext = {};

            // Right-click on blank space
            if (ImGui::BeginPopupContextWindow(nullptr, 1, false)) {
                if (ImGui::MenuItem("Create Empty Entity"))
                    m_Context->CreateEntity("Empty Entity");

                ImGui::EndPopup();
            }

        }
        ImGui::End();

        ImGui::Begin("Properties");
        if (m_SelectionContext) {
            DrawComponents(m_SelectionContext);
        }

        ImGui::End();
    }

    void SceneHierarchyPanel::SetSelectedEntity(Entity entity) {
        m_SelectionContext = entity;
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity) {
        auto& tag = entity.GetComponent<TagComponent>().Tag;
        //std::basic_string<char> &tag = std::string("test");
        /*if(entity.HasComponent<TagComponent>()) {
            tag = entity.GetComponent<TagComponent>().Tag;
        }*/

        ImGuiTreeNodeFlags flags =
                ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        bool opened = ImGui::TreeNodeEx((void *) (uint64_t) (uint32_t) entity, flags, "%s", tag.c_str());
        if (ImGui::IsItemClicked()) {
            m_SelectionContext = entity;
        }

        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Create Empty Entity")) {
                auto newEntity = m_Context->CreateEntity("Empty Entity");
                newEntity.GetComponent<RelationshipComponent>().parent = entity;
                entity.GetComponent<RelationshipComponent>().children.push_back(newEntity);
            }

            if (ImGui::MenuItem("Delete Entity")) {
                entityDeleted = true;
            }

            ImGui::EndPopup();
        }

        if (opened) {
            for (auto &entityID: entity.GetComponent<RelationshipComponent>().children) {
                Entity child{entityID, m_Context.get()};
                DrawEntityNode(child);
            }

            ImGui::TreePop();
        }

        if (entityDeleted) {
            if (entity.HasComponent<RelationshipComponent>()) {
                {
                    auto &children = entity.GetComponent<RelationshipComponent>().children;
                    for (auto &childID: children) {
                        m_Context->m_Registry.destroy(childID);
                    }
                    children.clear();
                }

                auto &parentID = entity.GetComponent<RelationshipComponent>().parent;
                if (parentID != entt::null) {
                    Entity parent{parentID, m_Context.get()};
                    auto &children = parent.GetComponent<RelationshipComponent>().children;
                    children.erase(std::remove(children.begin(), children.end(), entity), children.end());
                }

            }

            m_Context->DestroyEntity(entity);
            if (m_SelectionContext == entity)
                m_SelectionContext = {};
        }


        /*auto &tag = entity.GetComponent<TagComponent>().Tag;

        if(ImGui::TreeNodeEx(tag.c_str())) {
            if (ImGui::IsItemClicked()) {
                m_SelectionContext = entity;
            }

            bool entityDeleted = false;
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Delete Entity"))
                    entityDeleted = true;

                ImGui::EndPopup();
            }

            if (entityDeleted) {
                //entt::entity parentID = entity.GetComponent<RelationshipComponent>().parent;
                if(parentID != entt::null) {
                    Entity parent{parentID, m_Context.get()};
                    auto& children = parent.GetComponent<RelationshipComponent>().children;
                    children.erase(std::remove(children.begin(), children.end(), entity.GetHandle()), children.end());

                if(entity.HasComponent<RelationshipComponent>()) {
                    auto& children = entity.GetComponent<RelationshipComponent>().children;
                    for(auto& childID : children) {
                       m_Context->m_Registry.destroy(childID);
                    }
                    children.clear();
                }

                m_Context->DestroyEntity(entity);
                if (m_SelectionContext == entity)
                    m_SelectionContext = {};
            }

            for(auto& entityID : entity.GetComponent<RelationshipComponent>().children) {
                Entity child{entityID, m_Context.get()};
                DrawEntityNode(child);
            }
            ImGui::TreePop();
        }*/
    }

    static void
    DrawVec3Control(const std::string &label, glm::vec3 &values, float resetValue = 0.0f, float columnWidth = 100.0f) {
        ImGuiIO &io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
            values.z = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);

        ImGui::PopID();
    }

    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string &name, Entity entity, UIFunction uiFunction) {
        const ImGuiTreeNodeFlags treeNodeFlags =
                ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth |
                ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
        if (entity.HasComponent<T>()) {
            auto &component = entity.GetComponent<T>();
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
            float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx((void *) typeid(T).hash_code(), treeNodeFlags, "%s", name.c_str());
            ImGui::PopStyleVar(
            );
            ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
            if (ImGui::Button("+", ImVec2{lineHeight, lineHeight})) {
                ImGui::OpenPopup("ComponentSettings");
            }

            bool removeComponent = false;
            if (ImGui::BeginPopup("ComponentSettings")) {
                if (ImGui::MenuItem("Remove component"))
                    removeComponent = true;

                ImGui::EndPopup();
            }

            if (open) {
                uiFunction(component);
                ImGui::TreePop();
            }

            if (removeComponent)
                entity.RemoveComponent<T>();
        }
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity) {
        if (entity.HasComponent<TagComponent>()) {
            auto &tag = entity.GetComponent<TagComponent>().Tag;

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            std::strncpy(buffer, tag.c_str(), sizeof(buffer));
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
                tag = std::string(buffer);
            }
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);

        if (ImGui::Button("Add Component"))
            ImGui::OpenPopup("AddComponent");

        if (ImGui::BeginPopup("AddComponent")) {
            if (ImGui::MenuItem("Point Light")) {
                if (!m_SelectionContext.HasComponent<PointLightComponent>())
                    m_SelectionContext.AddComponent<PointLightComponent>();
                else
                    std::cout << "screw this" << std::endl;
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Camera Component")) {
                if (!m_SelectionContext.HasComponent<CameraComponent>()) {
                    m_SelectionContext.AddComponent<CameraComponent>();
                    std::cout << "test this" << std::endl;
                } else
                    std::cout << "screw this" << std::endl;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::PopItemWidth();

        DrawComponent<TransformComponent>("Transform", entity, [](auto &component) {
            DrawVec3Control("Translation", component.Translation);
            glm::vec3 rotation = glm::degrees(component.Rotation);
            DrawVec3Control("Rotation", rotation);
            component.Rotation = glm::radians(rotation);
            DrawVec3Control("Scale", component.Scale, 1.0f);
            if(ImGui::IsWindowHovered()) {
                component.isDirty = true;
            }
        });

        DrawComponent<RigidBodyComponent>("RigidBody", entity, [](auto &component) {
            DrawVec3Control("Translation", component.acceleration);
            DrawVec3Control("Velocity", component.velocity);
            ImGui::SliderFloat("Radius", &component.radius, 0.0f, 100.0f);
            ImGui::SliderFloat("Mass", &component.mass, 0.0f, 100.0f);
        });

        DrawComponent<PointLightComponent>("PointLight", entity, [](auto &component) {
            ImGui::ColorPicker3("Light Color", &component.color.r, ImGuiColorEditFlags_DisplayRGB);
            ImGui::SliderFloat("Light Intensity", &component.intensity, 0.0f, 100.0f);
        });

        DrawComponent<ModelComponent>("Model", entity, [](auto &component) {
            auto &path = component.path;
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            std::strncpy(buffer, path.c_str(), sizeof(buffer));
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
                path = std::string(buffer);
            }
            ImGui::SameLine();
            if (ImGui::Button("Load")) {
                //auto model = std::make_shared<Model>(path);
                //component.model = model;
            }
        });

        DrawComponent<CameraComponent>("PointLight", entity, [](auto &component) {
            ImGui::Checkbox("mainCamera", &component.mainCamera);
        });
    }
}
