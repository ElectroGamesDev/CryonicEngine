#include "CanvasEditor.h"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imnodes/imnodes.h"
#include "ThirdParty/imgui/IconsFontAwesome6.h"
#include "Utilities/FontManager.h"
#include <fstream>
#include "Utilities/ConsoleLogger.h"

namespace CanvasEditor
{
    bool windowOpen = false;
    nlohmann::json canvasData = nullptr;
    static nlohmann::json* selectedObject = nullptr;
    static bool hierarchyObjectClicked = false;
    static nlohmann::json* objectInProperties = nullptr;
    static nlohmann::json* objectInHierarchyContextMenu = nullptr;
    static bool hierarchyContextMenuOpen = false;

    void Render()
    {
        //if (!windowOpen) return;
        ////ImGui::SetNextWindowSize(ImVec2(180, 180));
        ////ImGui::SetNextWindowPos(ImVec2((RaylibWrapper::GetScreenWidth() - 180) / 2, (RaylibWrapper::GetScreenHeight() - 180) / 2));

        //static nlohmann::json* selectedNode = nullptr;

        //// Todo: Add zoom support

        //if (ImGui::Begin((ICON_FA_BRUSH + std::string(" Canvas Editor")).c_str(), &windowOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        //{
        //    bool update = false;
        //    ImNodes::BeginNodeEditor();
        //    if (canvasData.is_null())
        //    {
        //        // Todo: Fade background and make it so user can't move grid
        //        // Todo: Add a button to create a Canvas and have it open up a file explorer to set path
        //        ImGui::PushFont(FontManager::GetFont("Familiar-Pro-Bold", 25, false));
        //        ImVec2 textSize = ImGui::CalcTextSize("No Canvas selected. Select or create one in the Content Browser.");
        //        ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - textSize.x) * 0.5f, (ImGui::GetWindowHeight() - textSize.y) * 0.5f));
        //        ImGui::Text("No Canvas selected. Select or create one in the Content Browser.");
        //        ImGui::PopFont();
        //        ImNodes::EndNodeEditor();
        //        ImGui::End();
        //        selectedNode = nullptr;
        //        return;
        //    }

        //    ImNodes::EndNodeEditor();

        //    if (update)
        //    {
        //        std::ofstream file(canvasData["path"].get<std::filesystem::path>());
        //        if (file.is_open())
        //        {
        //            file << std::setw(4) << canvasData << std::endl;
        //            file.close();
        //            update = false;
        //        }
        //    }

        //    // Todo: When saving, pop up saying the canvas file was deleted or moved. Asked to recreate it or delete it
        //}
        //ImGui::End();
    }

    bool RenderHierarchyNode(nlohmann::json* gameObject, bool normalColor, bool& childDoubleClicked)
    {
    //    childDoubleClicked = false;

    //    ImGui::TableNextRow();
    //    ImGui::TableSetColumnIndex(0);
    //    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, normalColor ? IM_COL32(36, 40, 43, 255) : IM_COL32(45, 48, 51, 255));

    //    //ImGui::PushStyleColor(ImGuiCol_Header, {64, 64, 64, 255});

    //    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;

    //    if (selectedObject != nullptr && (*selectedObject)["id"].get<int>() == (*gameObject)["id"].get<int>())
    //        flags |= ImGuiTreeNodeFlags_Selected;

    //    flags |= gameObject->GetChildren().empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnArrow;

    //    //std::string icon = ICON_FA_CUBE;
    //    //if (gameObject->GetComponent<CameraComponent>()) // Todo: Not sure if this is a good idea to check if it has teh Camera component every frame.
    //    //    icon = ICON_FA_CAMERA;

    //    // This is coded a bit different than I normally would since TreeNodeEx() doesn't return true if the game object has children but its not expanded
    //    bool nodeOpen = ImGui::TreeNodeEx((ICON_FA_CUBE + std::string(" ") + (*gameObject)["name"].get<std::string>() + "##" + std::to_string((*gameObject)["id"].get<int>())).c_str(), flags);

    //    bool currentNodeDoubleClicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

    //    if (ImGui::IsItemClicked())
    //    {
    //        hierarchyObjectClicked = true;
    //        objectInProperties = selectedObject = gameObject;
    //    }
    //    else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    //        objectInHierarchyContextMenu = gameObject;

    //    if (nodeOpen)
    //    {
    //        for (GameObject* child : gameObject->GetChildren())
    //        {
    //            bool childClicked = false;
    //            normalColor = RenderHierarchyNode(child, !normalColor, childClicked);
    //            childDoubleClicked |= childClicked;
    //        }
    //        ImGui::TreePop();
    //    }

    //    childDoubleClicked |= currentNodeDoubleClicked;

    //    return normalColor; // Todo: If has child and child is open, return the lowest color rather than this node's color
        return false;
    }

    void RenderHierarchy()
    {
    //    hierarchyObjectClicked = false;
    //    ImGui::Begin((ICON_FA_SITEMAP + std::string(" Hierarchy##CanvasEditor")).c_str(), nullptr, ImGuiWindowFlags_NoCollapse);

    //    if (ImGui::BeginTable("HierarchyTable##CanvasEditor", 1))
    //    {
    //        bool hierarchyRowColor = true;

    //        for (auto& gameObjectJson : canvasData["GameObjects"])
    //            if (gameObjectJson["parent_id"].get<int>() == -1)
    //            {
    //                bool childDoubleClicked = false;
    //                hierarchyRowColor = !RenderHierarchyNode(&gameObjectJson, hierarchyRowColor, childDoubleClicked);
    //            }

    //        hierarchyRowColor = false;
    //        ImGui::EndTable();
    //    }

    //    if (hierarchyContextMenuOpen || (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)))
    //    {
    //        hierarchyContextMenuOpen = true;
    //        if (ImGui::BeginPopupContextWindow("HierarchyContextMenuCanvasEditor"))
    //        {
    //            ImGui::Separator();

    //            struct ObjectItem
    //            {
    //                std::string menuName;
    //                std::string name;
    //                std::string type;
    //            };

    //            static const std::vector<ObjectItem> menuObjects = {
    //                {"Create Label", "Label", "Label"},
    //            };

    //            ObjectItem objectToCreate = { "", "", ""};

    //            for (const ObjectItem& item : menuObjects)
    //            {
    //                if (ImGui::MenuItem(item.menuName.c_str()))
    //                {
    //                    hierarchyContextMenuOpen = false;
    //                    objectToCreate = item;
    //                }
    //            }

    //            if (objectInHierarchyContextMenu != nullptr && ImGui::MenuItem("Delete"))
    //            {
    //                hierarchyObjectClicked = true;
    //                hierarchyContextMenuOpen = false;

    //                if (objectInProperties->GetId() == objectInHierarchyContextMenu->GetId())
    //                    objectInProperties = nullptr;

    //                if (selectedObject != nullptr && objectInHierarchyContextMenu->GetId() == selectedObject->GetId())
    //                    selectedObject = nullptr;

    //                SceneManager::GetActiveScene()->RemoveGameObject(objectInHierarchyContextMenu);
    //                objectInHierarchyContextMenu = nullptr;
    //            }

    //            if (!objectToCreate.name.empty())
    //            {
    //                hierarchyObjectClicked = true;

    //                GameObject* gameObject = SceneManager::GetActiveScene()->AddGameObject();
    //                gameObject->transform.SetPosition({ 0,0,0 });
    //                gameObject->transform.SetScale({ 1,1,1 });
    //                gameObject->transform.SetRotation(Quaternion::Identity());
    //                gameObject->SetName(objectToCreate.name);
    //                if (objectToCreate.name == "Camera")
    //                {
    //                    gameObject->AddComponentInternal<CameraComponent>();
    //                    if (!ProjectManager::projectData.is3D)
    //                    {
    //                        gameObject->transform.SetPosition({ 0,0,0 });
    //                        gameObject->transform.SetRotationEuler({ 180, 0, 0 });
    //                    }
    //                }
    //                else if (objectToCreate.name == "Light")
    //                    gameObject->AddComponentInternal<Lighting>();
    //                else if (objectToCreate.name != "GameObject")
    //                {
    //                    if (ProjectManager::projectData.is3D)
    //                    {
    //                        MeshRenderer& meshRenderer = gameObject->AddComponentInternal<MeshRenderer>();
    //                        meshRenderer.SetModelPath(objectToCreate.name);
    //                        meshRenderer.SetModel(objectToCreate.model, objectToCreate.name, ShaderManager::LitStandard);
    //                    }
    //                    else
    //                    {
    //                        SpriteRenderer& spriteRenderer = gameObject->AddComponentInternal<SpriteRenderer>();
    //                        spriteRenderer.SetTexture(objectToCreate.name);

    //                        //gameObject->AddComponentInternal<Collider2D>(); // Todo: Set size and type
    //                    }
    //                }

    //                //SceneManager::GetActiveScene()->AddGameObject(gameObject);

    //                for (Component* component : gameObject->GetComponents())
    //                    component->gameObject = SceneManager::GetActiveScene()->GetGameObjects().back();

    //                if (objectInHierarchyContextMenu != nullptr)
    //                {
    //                    gameObject->SetParent(objectInHierarchyContextMenu);
    //                    gameObject->transform.SetLocalPosition({ 0,0,0 });
    //                    gameObject->transform.SetLocalRotationEuler({ 0,0,0 });
    //                    gameObject->transform.SetLocalScale({ 0,0,0 });
    //                }

    //                selectedObject = gameObject;
    //                objectInProperties = gameObject;
    //            }

    //            ImGui::EndPopup();
    //        }
    //        if (hierarchyContextMenuOpen && !ImGui::IsPopupOpen("HierarchyContextMenu##CanvasEditor"))
    //        {
    //            objectInHierarchyContextMenu = nullptr;
    //            hierarchyContextMenuOpen = false;
    //        }
    //    }
    //    else if (!hierarchyObjectClicked && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
    //    {
    //        if (selectedObject)
    //        {
    //            selectedObject = nullptr;
    //        }
    //        if (std::holds_alternative<GameObject*>(objectInProperties))
    //            objectInProperties = std::monostate{};
    //    }
    //    ImGui::End();
    }
}