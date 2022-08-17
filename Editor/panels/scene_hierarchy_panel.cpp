#include "scene_hierarchy_panel.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <cstring>
#include <iostream>

#include <filesystem>
#include <string>

namespace Engine {
    SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene> &context) {
        set_context(context);
    }

    void SceneHierarchyPanel::set_context(const std::shared_ptr<Scene> &_context) {
        context = _context;
        selection_context = {};
    }

    void SceneHierarchyPanel::render() {
        ImGui::Begin("Scene Hierarchy");

        if (context) {
            context->registry.each([&](auto entityID) {
                Entity entity{entityID, context.get()};
                if (entity.has_component<RelationshipComponent>()) {
                    if (entity.get_component<RelationshipComponent>().parent == entt::null) {
                        draw_entity_node(entity);
                    }
                }
            });

            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
                selection_context = {};

            // Right-click on blank space
            if (ImGui::BeginPopupContextWindow(nullptr, 1, false)) {
                if (ImGui::MenuItem("Create Empty Entity"))
                    context->create_entity("Empty Entity");

                ImGui::EndPopup();
            }

        }
        ImGui::End();

        ImGui::Begin("Properties");
        if (selection_context) {
            draw_components(selection_context);
        }

        ImGui::End();
    }

    void SceneHierarchyPanel::set_selected_entity(Entity entity) {
        selection_context = entity;
    }

    void SceneHierarchyPanel::draw_entity_node(Entity entity) {
        auto& tag = entity.get_component<TagComponent>().tag;

        ImGuiTreeNodeFlags flags = ((selection_context == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        bool opened = ImGui::TreeNodeEx(reinterpret_cast<void *>(static_cast<uint64_t>(static_cast<uint32_t>(entity))), flags, "%s", tag.c_str());
        if (ImGui::IsItemClicked()) {
            selection_context = entity;
        }

        bool entity_deleted = false;
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Create Empty Entity")) {
                auto new_entity = context->create_entity("Empty Entity");
                new_entity.get_component<RelationshipComponent>().parent = entity;
                entity.get_component<RelationshipComponent>().children.push_back(new_entity);
            }

            if (ImGui::MenuItem("Delete Entity")) {
                entity_deleted = true;
            }

            ImGui::EndPopup();
        }

        if (opened) {
            for (auto &entityID: entity.get_component<RelationshipComponent>().children) {
                Entity child{entityID, context.get()};
                draw_entity_node(child);
            }

            ImGui::TreePop();
        }

        if (entity_deleted) {
            if (entity.has_component<RelationshipComponent>()) {
                {
                    auto &children = entity.get_component<RelationshipComponent>().children;
                    for (auto &childID: children) {
                        context->registry.destroy(childID);
                    }
                    children.clear();
                }

                auto &parentID = entity.get_component<RelationshipComponent>().parent;
                if (parentID != entt::null) {
                    Entity parent{parentID, context.get()};
                    auto &children = parent.get_component<RelationshipComponent>().children;
                    children.erase(std::remove(children.begin(), children.end(), entity), children.end());
                }

            }

            context->destroy_entity(entity);
            if (selection_context == entity)
                selection_context = {};
        }
    }

    static void DrawVec3Control(const std::string &label, glm::vec3 &values, float reset_value = 0.0f, float column_width = 100.0f) {
        ImGuiIO &io = ImGui::GetIO();
        auto bold_font = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, column_width);
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
        ImGui::PushFont(bold_font);
        if (ImGui::Button("X", buttonSize))
            values.x = reset_value;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
        ImGui::PushFont(bold_font);
        if (ImGui::Button("Y", buttonSize))
            values.y = reset_value;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
        ImGui::PushFont(bold_font);
        if (ImGui::Button("Z", buttonSize))
            values.z = reset_value;
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
        const ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
        if (entity.has_component<T>()) {
            auto &component = entity.get_component<T>();
            ImVec2 content_region_available = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
            float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(T).hash_code()), tree_node_flags, "%s", name.c_str());
            ImGui::PopStyleVar();
            ImGui::SameLine(content_region_available.x - line_height * 0.5f);
            if (ImGui::Button("+", ImVec2{line_height, line_height})) {
                ImGui::OpenPopup("ComponentSettings");
            }

            bool remove_component = false;
            if (ImGui::BeginPopup("ComponentSettings")) {
                if (ImGui::MenuItem("Remove component"))
                    remove_component = true;

                ImGui::EndPopup();
            }

            if (open) {
                uiFunction(component);
                ImGui::TreePop();
            }

            if (remove_component)
                entity.remove_component<T>();
        }
    }

    void SceneHierarchyPanel::draw_components(Entity entity) {
        if (entity.has_component<TagComponent>()) {
            auto &tag = entity.get_component<TagComponent>().tag;

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
                if (!selection_context.has_component<PointLightComponent>())
                    selection_context.add_component<PointLightComponent>();
                else
                    std::cout << "screw this" << std::endl;
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Camera Component")) {
                if (!selection_context.has_component<CameraComponent>()) {
                    selection_context.add_component<CameraComponent>();
                    std::cout << "test this" << std::endl;
                } else
                    std::cout << "screw this" << std::endl;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::PopItemWidth();

        DrawComponent<TransformComponent>("Transform", entity, [](auto &component) {
            DrawVec3Control("Translation", component.translation);
            glm::vec3 rotation = glm::degrees(component.rotation);
            DrawVec3Control("Rotation", rotation);
            component.rotation = glm::radians(rotation);
            DrawVec3Control("Scale", component.scale, 1.0f);
            if(ImGui::IsWindowHovered()) {
                component.is_dirty = true;
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
    }
}
