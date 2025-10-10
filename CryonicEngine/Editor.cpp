#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <variant>
#include <unordered_map>
#include <thread>
#include "Editor.h"
#include "FontManager.h"
#include "ConsoleLogger.h"
#include "Scenes/SceneManager.h"
#include <imgui_internal.h>
#include "IconsFontAwesome6.h"
#include <ImGuiNotify.hpp>
#include <fstream>
#include "Utilities.h"
#include <any>
#include "Components/MeshRenderer.h"
#include "Components/ScriptComponent.h"
#include "Components/CameraComponent.h"
#include "Components/Lighting.h"
#include "Components/SpriteRenderer.h"
#include "Components/Collider2D.h"
#include "Components/Rigidbody2D.h"
#include "Components/Collider3D.h"
#include "Components/Rigidbody3D.h"
#include "Components/AnimationPlayer.h"
#include "Components/AudioPlayer.h"
#include "Components/TilemapRenderer.h"
#include "Components/Label.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/CanvasRenderer.h"
#include "IconManager.h"
#include "ShaderManager.h"
#include "ShadowManager.h"
#include "ProjectManager.h"
#include "RaylibModelWrapper.h"
#include "RaylibDrawWrapper.h"
#include "imnodes.h"
#include <random>
#include <cmath>
#include "EventSystem.h"
#include "ImGuiPopup.h"
#include "CanvasEditor.h"
#include "FileWatcher.h"
#include "AssetManager.h"
#include "RenderableTexture.h"
#include "EditorWindow.h"
#include "EventSheetEditor.h"
#include "PrefabEditor.h"
#include "MainThreadQueue.h"
#include "ScriptingTools/ScriptHeaderGenerator.h"

//#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Jolt/Jolt.h"
JPH_SUPPRESS_WARNINGS

RaylibWrapper::Camera Editor::camera = { 0 };

// Todo: Switch these to use DEG2RAD and RAD2DEG from CryonicCore
const float DEG = 180.0f / 3.14159265358979323846f;
const float RAD = 3.14159265358979323846f / 180.0f;

ImGuiID dockspaceID;

bool closeEditor = false;
bool viewportOpen = true;
bool gameViewOpen = true;
bool viewportFocused = false;
bool viewportHovered = false;
bool rmbDown = false;
RaylibWrapper::RenderTexture ViewTexture;
RaylibWrapper::RenderTexture cameraRenderTexture;
RaylibWrapper::Texture2D GridTexture = { 0 };
//ShadowManager shadowManager;

bool viewportOpened = true;
Vector4 Editor::viewportPosition;

std::variant<std::monostate, GameObject*, DataFile> Editor::objectInProperties = std::monostate{}; // Make a struct or something that holds a Path and ifstream String. Not specific to material so prefabs and stuff can use
GameObject* selectedObject = nullptr;
bool movingObjectX = false;
bool movingObjectY = false;
bool movingObjectZ = false;
bool cameraSelected = false;

RaylibWrapper::Vector2 lastMousePosition = { 0 };
bool animationGraphHovered = false;

bool explorerContextMenuOpen = false;
std::filesystem::path explorerContextMenuFile;
bool hierarchyContextMenuOpen = false;
GameObject* objectInHierarchyContextMenu = nullptr;
bool hierarchyObjectClicked = false;
bool componentsWindowOpen = false;
bool scriptCreateWinOpen = false;
bool animationGraphWinOpen = false;
bool projectSettingsWinOpen = false;

bool resetComponentsWin = true;
bool resetPropertiesWin = true;
bool resetViewportWin = true;
bool resetFileExplorerWin = true;
bool resetHierarchy = true;
bool resetCameraView = true;
bool resetProjectSettings = true;

std::vector<EditorWindow*> windows;

bool guiVisible = false;

std::string focusContentBrowserFile = "";

nlohmann::json animationGraphData = nullptr;

std::filesystem::path fileExplorerPath;
enum DragTypes {None, ImageFile, ModelFile, Folder, Other};
std::pair<DragTypes, std::unordered_map<std::string, std::any>> dragData;

std::vector<RaylibWrapper::Texture2D*> tempTextures;
std::vector<RaylibWrapper::RenderTexture2D*> tempRenderTextures;

float cameraSpeed = 1;
float oneSecondDelay = 1;

RaylibModel materialPreviewMesh;
bool playModeActive = false;
Utilities::JobHandle playModeJobHandle;

std::unordered_map<std::filesystem::path, std::pair<RaylibWrapper::Texture, float>> cachedTextures; // The float is the GetTime() since the texture was last used

enum Tool
{
    Move,
    Rotate,
    Scale
}; Tool toolSelected = Move;

// Todo: Move this to another class
RaylibWrapper::RenderTexture2D* Editor::CreateModelPreview(std::filesystem::path modelPath, int textureSize)
{
    // Todo: Don't load and unload the models every frame if the file path is the same
    
    // Load the 3D model

    RaylibModel model;
    model.Create(Custom, modelPath.string().c_str(), ShaderManager::LitStandard, ProjectManager::projectData.path);

    // Set up camera
    RaylibWrapper::Camera modelCamera = {
        {0.0f, 0.0f, 6.0f},  // position
        {0.0f, 0.0f, 0.0f},  // target
        {0.0f, 1.0f, 0.0f},  // up
        45.0f,               // fovy
        RaylibWrapper::CAMERA_PERSPECTIVE // projection
    };

    // Create a render texture
    tempRenderTextures.push_back(new RaylibWrapper::RenderTexture2D(RaylibWrapper::LoadRenderTexture(textureSize, textureSize)));

    // Render model to texture
    BeginTextureMode(*tempRenderTextures.back());
    RaylibWrapper::ClearBackground(RaylibWrapper::Color{ 63, 63, 63, 255 });
    RaylibWrapper::BeginMode3D(modelCamera);
    model.DrawModelWrapper(0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 255, 255, 255, 255);
    RaylibWrapper::EndMode3D();
    RaylibWrapper::EndTextureMode();

    //std::cout << "Path: " << modelPath << ", Texture Size: " << textureSize << std::endl;

    // Unload the model
    model.Unload();

    //tempTextures.push_back(new Texture2D(target.texture));

    //UnloadRenderTexture(target);

    // Return the generated texture
    return tempRenderTextures.back();
}

// Todo: Move this to another class
RaylibWrapper::RenderTexture2D* Editor::CreateMaterialPreview(std::filesystem::path materialPath, int textureSize)
{
    // Todo: make sure to show melatic, roughness, opacity, emission, etc.

    nlohmann::json jsonData;
    std::ifstream file;

    try {
        file.open(materialPath);
        file >> jsonData;
    }
    catch (const std::exception& e)
    {
        file.close();
        std::filesystem::path corruptedFilePath = materialPath.parent_path() / (materialPath.stem().string() + " Corrupted.mat");
        std::filesystem::copy(materialPath, corruptedFilePath); // Todo: Will this crash if a file with that name already exists?
        if (Utilities::CreateDataFile(materialPath))
        {
            try {
                std::ifstream file(materialPath);
                file >> jsonData;
                ConsoleLogger::WarningLog("Your material " + materialPath.stem().string() + " was corrupted. A new material has been made and we backed up the corrupted file to " + corruptedFilePath.stem().string() + ".");
            }
            catch (const std::exception& e) {
                // If the new material file fails to open, we can assume its an issue other than corruption. Since we can create the file, we can also assume its not a permissions issue.
                ConsoleLogger::WarningLog("Your material " + materialPath.stem().string() + " failed to open. This is not a corruption or permission issue. Try restarting the game engine and if the issue continues then try to update to a newer engine version if one is available.");
                std::filesystem::copy(corruptedFilePath, materialPath);
                std::filesystem::remove(corruptedFilePath);
                return nullptr;
            }
        }
        else
        {
            ConsoleLogger::WarningLog("Your material " + materialPath.stem().string() + " was corrupted and we failed to create a new one. Your project may be in a directory the engine can't access without proper permissions. We recommend having the engine in the Documents directory.");
            //std::filesystem::copy(corruptedFilePath, materialPath);
            std::filesystem::remove(corruptedFilePath);
            return nullptr;
        }
    }

    auto& publicData = jsonData["public"];

    // Extract material properties
    RaylibWrapper::Color albedoColor = {
        static_cast<unsigned char>(publicData["albedoColor"][0]),
        static_cast<unsigned char>(publicData["albedoColor"][1]),
        static_cast<unsigned char>(publicData["albedoColor"][2]),
        static_cast<unsigned char>(publicData["albedoColor"][3])
    };

    float metallic = publicData.value("metallic", 0.0f);
    float roughness = publicData.value("roughness", 0.5f);
    float emission = publicData.value("emission", 0.0f);

    // Load textures
    auto loadTexture = [](const std::string& path) -> RaylibWrapper::Texture2D* {
        if (path.empty() || !std::filesystem::exists(ProjectManager::projectData.path / "Assets" / path))
            return &Material::whiteTexture;

        return new RaylibWrapper::Texture2D(RaylibWrapper::LoadTexture(std::filesystem::path(ProjectManager::projectData.path / "Assets" / path).string().c_str()));
    };

    // Todo: This will have issues if some of the maps arent set or valid. We're also loading the texture even if its already loaded as a Sprite

    // Todo: These textures may already be loaded if a model in-scene is using them. Once I do this, make sure not to unload the textures at the end of the function
    RaylibWrapper::Texture2D* albedoTexture = loadTexture(publicData["albedoTexture"][2].get<std::string>());
    RaylibWrapper::Texture2D* normalTexture = loadTexture(publicData["normalTexture"][2].get<std::string>());
    RaylibWrapper::Texture2D* metallicTexture = loadTexture(publicData["metallicTexture"][2].get<std::string>());
    RaylibWrapper::Texture2D* roughnessTexture = loadTexture(publicData["roughnessTexture"][2].get<std::string>());
    RaylibWrapper::Texture2D* emissionTexture = loadTexture(publicData["emissionTexture"][2].get<std::string>());

    RaylibWrapper::Color normalColor = normalTexture == &Material::whiteTexture
        ? RaylibWrapper::Color{ 128, 128, 255 }
        : RaylibWrapper::Color{ 255, 255, 255 };

    // Set material properties
    materialPreviewMesh.SetMaterialMap(0, RaylibWrapper::MATERIAL_MAP_ALBEDO, *albedoTexture, albedoColor, 1);
    materialPreviewMesh.SetMaterialMap(0, RaylibWrapper::MATERIAL_MAP_NORMAL, *normalTexture, normalColor, 1);
    materialPreviewMesh.SetMaterialMap(0, RaylibWrapper::MATERIAL_MAP_ROUGHNESS, *roughnessTexture, RaylibWrapper::WHITE, roughness);
    materialPreviewMesh.SetMaterialMap(0, RaylibWrapper::MATERIAL_MAP_METALNESS, *metallicTexture, RaylibWrapper::WHITE, metallic);
    materialPreviewMesh.SetMaterialMap(0, RaylibWrapper::MATERIAL_MAP_EMISSION, *emissionTexture, RaylibWrapper::WHITE, emission);

    // Set up camera
    RaylibWrapper::Camera previewCamera = {
        {0.0f, 0.0f, 2.0f},  // position
        {0.0f, 0.0f, 0.0f},  // target
        {0.0f, 1.0f, 0.0f},  // up
        45.0f,               // fovy
        RaylibWrapper::CAMERA_PERSPECTIVE // projection
    };

    // Create a render texture
    tempRenderTextures.push_back(new RaylibWrapper::RenderTexture2D(RaylibWrapper::LoadRenderTexture(textureSize, textureSize)));

    // Render model to texture
    BeginTextureMode(*tempRenderTextures.back());
    RaylibWrapper::ClearBackground(RaylibWrapper::Color{ 63, 63, 63, 255 });
    RaylibWrapper::BeginMode3D(previewCamera);
    materialPreviewMesh.DrawModelWrapper(0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 255, 255, 255, 255);
    RaylibWrapper::EndMode3D();
    RaylibWrapper::EndTextureMode();

    if (albedoTexture != &Material::whiteTexture)
    {
        RaylibWrapper::UnloadTexture(*albedoTexture);
        delete albedoTexture;
    }

    if (normalTexture != &Material::whiteTexture)
    {
        RaylibWrapper::UnloadTexture(*normalTexture);
        delete normalTexture;
    }

    if (metallicTexture != &Material::whiteTexture)
    {
        RaylibWrapper::UnloadTexture(*metallicTexture);
        delete metallicTexture;
    }

    if (roughnessTexture != &Material::whiteTexture)
    {
        RaylibWrapper::UnloadTexture(*roughnessTexture);
        delete roughnessTexture;
    }

    if (emissionTexture != &Material::whiteTexture)
    {
        RaylibWrapper::UnloadTexture(*emissionTexture);
        delete emissionTexture;
    }

    return tempRenderTextures.back();
}

void Editor::RenderViewport()
{
    if (!viewportOpen)
        return;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.11f, 0.12f, 1.00f));
    // Todo: Use resize events

    ImGui::SetNextWindowClass(&EditorWindow::defaultWindowClass);
    if (ImGui::Begin((ICON_FA_CUBES + std::string(" Viewport")).c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar))
    {
        ImVec2 contentSize = ImGui::GetContentRegionAvail();

        viewportPosition = { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y };
        viewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
        rlImGuiImageRenderTextureFit(&ViewTexture, true);

        // Render GUI from components
        if (guiVisible)
        {
            for (GameObject* gameObject : SceneManager::GetActiveScene()->GetGameObjects())
            {
                if (!gameObject->IsActive())
                    continue;

                for (Component* component : gameObject->GetComponents())
                {
                    if (!component->IsActive())
                        continue;

                    component->RenderGui();
                }
            }

            // Outline
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();

            ImVec2 aspectRatioSize(ProjectManager::projectData.windowResolution.x, ProjectManager::projectData.windowResolution.y);

            // Calculate the scaling factor used by rlImGuiImageRenderTextureFit
            float scaleX = contentSize.x / ViewTexture.texture.width;
            float scaleY = contentSize.y / ViewTexture.texture.height;
            float scale = (ViewTexture.texture.height * scaleX > contentSize.y) ? scaleY : scaleX;

            // Compute the actual rendered position and size of the image
            ImVec2 renderSize = ImVec2(ViewTexture.texture.width * scale, ViewTexture.texture.height * scale);
            ImVec2 renderPos = ImVec2(
                (contentSize.x - renderSize.x) * 0.5f,
                (contentSize.y - renderSize.y) * 0.5f
            );
            //RaylibWrapper::Vector2 cameraPos = RaylibWrapper::GetWorldToScreen({0,0}, camera);
            //ConsoleLogger::InfoLog("X: " + std::to_string(camera.position.x) + " Y: " + std::to_string(camera.position.y));
            //renderPos.x += windowPos.x -  (camera.position.x / RaylibWrapper::GetScreenWidth() * windowSize.x);
            //renderPos.y += windowPos.y + (camera.position.y / RaylibWrapper::GetScreenHeight() * windowSize.y);
            renderPos.x += windowPos.x;
            renderPos.y += windowPos.y;

            // Adjust for the title bar height
            renderPos.y += windowSize.y - contentSize.y;

            // Calculate scaling to fit the aspect ratio size within the render size
            float aspectRatio = aspectRatioSize.x / aspectRatioSize.y;
            float renderAspectRatio = renderSize.x / renderSize.y;
            float finalScale = (renderAspectRatio > aspectRatio) ? renderSize.y / aspectRatioSize.y : renderSize.x / aspectRatioSize.x;

            // Compute the rectangle size and position based on the final scale
            ImVec2 rectSize = ImVec2(aspectRatioSize.x * finalScale, aspectRatioSize.y * finalScale);
            ImVec2 rectPos = ImVec2(
                renderPos.x + (renderSize.x - rectSize.x) * 0.5f,
                renderPos.y + (renderSize.y - rectSize.y) * 0.5f
            );

            // Add offset for the rectangle
            ImVec2 rectMin = ImVec2(rectPos.x, rectPos.y);
            ImVec2 rectMax = ImVec2(rectMin.x + rectSize.x, rectMin.y + rectSize.y);

            // Draw the rectangle
            ImGui::GetWindowDrawList()->AddRect(rectMin, rectMax, IM_COL32(255, 255, 0, 255), 0.0f, 0, 1.0f);
        }

        // Render tool buttons
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
        const char* tools[] = { "Move", "Rotate", "Scale" };
        for (int i = 0; i < 3; ++i)
        {
            ImGui::SetCursorPos(ImVec2(5 + i * 22, 27));
            if (RaylibWrapper::rlImGuiImageButtonSize(("##" + std::string(tools[i]) + "Tool").c_str(),
                IconManager::imageTextures[std::string(tools[i]) + "Tool" + ((toolSelected == static_cast<Tool>(i)) ? "Selected" : "") + "Icon"], { 20, 20 }))
                toolSelected = static_cast<Tool>(i);
        }

        // Render GUI Visibility button
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 32, 27));
        if (RaylibWrapper::rlImGuiImageButtonSize("##GuiVisibiltiy",
            IconManager::imageTextures["GuiVisibiltiy" + std::string(guiVisible ? "Selected" : "") + "Icon"], { 20, 20 }))
            guiVisible = !guiVisible;

        // Render Play Button
        RaylibWrapper::Texture* playIconTexture;
        if (playModeActive)
            playIconTexture = IconManager::imageTextures["ActivePlayIcon"];
        else
            playIconTexture = IconManager::imageTextures["PlayIcon"];

        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() / 2 - 20, 27));
        if (RaylibWrapper::rlImGuiImageButtonSize("##PlayButton", playIconTexture, ImVec2(30, 30)))
        {
            if (playModeActive)
                ExitPlayMode();
            else
                EnterPlayMode();
        }

        // Render Pause Button
        RaylibWrapper::Texture* pauseIconTexture;
        //if (paused) pauseIconTexture = activePauseIcon;
        //else if (playing) pauseIconTexture = pauseIcon;
        //else pauseIconTexture = grayedPauseIcon;

        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() / 2 + 20, 27));
        ImGui::BeginDisabled();
        if (RaylibWrapper::rlImGuiImageButtonSize("##PauseButton", IconManager::imageTextures["GrayedPauseIcon"], ImVec2(30, 30)))
        {
            // Todo: Implement this. Will need to pause the main loop and all threads
        }
        ImGui::EndDisabled();

        ImGui::PopStyleColor(3);

        viewportHovered = ImGui::IsWindowHovered();

        // Handle camera movement
        static float cameraSpeed = 5.0f;
        static float minCameraSpeed = 0.1f;
        static float maxCameraSpeed = 1000.0f;
        static bool firstRightClick = true;
        static RaylibWrapper::Vector2 savedMousePosition;

        if (ImGui::IsWindowHovered() && (RaylibWrapper::IsMouseButtonDown(RaylibWrapper::MOUSE_BUTTON_RIGHT) || !ProjectManager::projectData.is3D && RaylibWrapper::IsMouseButtonDown(RaylibWrapper::MOUSE_BUTTON_MIDDLE)))
        {
            if (ProjectManager::projectData.is3D && RaylibWrapper::IsMouseButtonDown(RaylibWrapper::MOUSE_BUTTON_RIGHT) && firstRightClick)
            {
                savedMousePosition = RaylibWrapper::GetMousePosition();
                RaylibWrapper::HideCursor();
                RaylibWrapper::DisableCursor();
                firstRightClick = false;
            }

            // Check for shift modifier for 2x speed
            int speedMultiplier = 1;
            if (RaylibWrapper::IsKeyDown(RaylibWrapper::KEY_LEFT_SHIFT) || RaylibWrapper::IsKeyDown(RaylibWrapper::KEY_RIGHT_SHIFT))
                speedMultiplier = 2;

            // Camera speed
            if (RaylibWrapper::IsMouseButtonDown(RaylibWrapper::MOUSE_BUTTON_RIGHT) && RaylibWrapper::GetMouseWheelMove() != 0)
            {
                float wheelMove = RaylibWrapper::GetMouseWheelMove();
                cameraSpeed += wheelMove * speedMultiplier * (1 + (cameraSpeed / maxCameraSpeed) * (cameraSpeed / maxCameraSpeed));

                if (cameraSpeed < minCameraSpeed)
                    cameraSpeed = minCameraSpeed;

                if (cameraSpeed > maxCameraSpeed)
                    cameraSpeed = maxCameraSpeed;
            }

            if (ProjectManager::projectData.is3D)
            {
                RaylibWrapper::Vector2 mousePositionDelta = RaylibWrapper::GetMouseDelta();
                float mouseSensitivity = 0.003f;

                // Keep cursor centered while right mouse buttonn is downn
                if (RaylibWrapper::IsMouseButtonDown(RaylibWrapper::MOUSE_BUTTON_RIGHT))
                    RaylibWrapper::SetMousePosition((viewportPosition.x + viewportPosition.z) / 2, (viewportPosition.y + viewportPosition.w) / 2);

                static float yaw = 0.0f;
                static float pitch = 0.0f;
                static bool yawPitchInitialized = false;

                if (!yawPitchInitialized)
                {
                    RaylibWrapper::Vector3 forward = RaylibWrapper::Vector3Normalize(RaylibWrapper::Vector3Subtract(camera.target, camera.position));
                    yaw = atan2f(forward.z, forward.x);
                    pitch = asinf(forward.y);
                    yawPitchInitialized = true;
                }

                yaw += mousePositionDelta.x * mouseSensitivity;
                pitch -= mousePositionDelta.y * mouseSensitivity;

                // Clamp pitch to prevent camera flipping
                if (pitch > 1.5f)
                    pitch = 1.5f;

                if (pitch < -1.5f)
                    pitch = -1.5f;

                camera.target.x = camera.position.x + cosf(yaw) * cosf(pitch);
                camera.target.y = camera.position.y + sinf(pitch);
                camera.target.z = camera.position.z + sinf(yaw) * cosf(pitch);

                RaylibWrapper::Vector3 forward = RaylibWrapper::Vector3Normalize(RaylibWrapper::Vector3Subtract(camera.target, camera.position));
                RaylibWrapper::Vector3 right = RaylibWrapper::Vector3Normalize(RaylibWrapper::Vector3CrossProduct(forward, { 0, 1, 0 }));

                float moveSpeed = cameraSpeed * speedMultiplier * RaylibWrapper::GetFrameTime();

                RaylibWrapper::Vector3 oldPosition = camera.position;

                if (RaylibWrapper::IsKeyDown(RaylibWrapper::KEY_W)) {
                    camera.position = RaylibWrapper::Vector3Add(camera.position, RaylibWrapper::Vector3Scale(forward, moveSpeed));
                }
                if (RaylibWrapper::IsKeyDown(RaylibWrapper::KEY_S)) {
                    camera.position = RaylibWrapper::Vector3Subtract(camera.position, RaylibWrapper::Vector3Scale(forward, moveSpeed));
                }
                if (RaylibWrapper::IsKeyDown(RaylibWrapper::KEY_A)) {
                    camera.position = RaylibWrapper::Vector3Subtract(camera.position, RaylibWrapper::Vector3Scale(right, moveSpeed));
                }
                if (RaylibWrapper::IsKeyDown(RaylibWrapper::KEY_D)) {
                    camera.position = RaylibWrapper::Vector3Add(camera.position, RaylibWrapper::Vector3Scale(right, moveSpeed));
                }

                if (RaylibWrapper::IsKeyDown(RaylibWrapper::KEY_E))
                    camera.position.y += moveSpeed;

                if (RaylibWrapper::IsKeyDown(RaylibWrapper::KEY_Q))
                    camera.position.y -= moveSpeed;

                camera.target.x = camera.position.x + cosf(yaw) * cosf(pitch);
                camera.target.y = camera.position.y + sinf(pitch);
                camera.target.z = camera.position.z + sinf(yaw) * cosf(pitch);
            }
            else {
                // 2D camera movement
                RaylibWrapper::Vector2 deltaMouse = RaylibWrapper::Vector2Subtract(RaylibWrapper::GetMousePosition(), lastMousePosition);
                float panSpeed = 0.1f * speedMultiplier * (cameraSpeed / 5.0f);

                camera.position.x += deltaMouse.x * panSpeed;
                camera.position.y -= deltaMouse.y * panSpeed;
                camera.target.x = camera.position.x;
                camera.target.y = camera.position.y;
            }
        }
        else if (!firstRightClick && RaylibWrapper::IsMouseButtonUp(RaylibWrapper::MOUSE_BUTTON_RIGHT) && ProjectManager::projectData.is3D) {
            // Restore mouse when right click is released
            RaylibWrapper::EnableCursor();
            RaylibWrapper::ShowCursor();
            RaylibWrapper::SetMousePosition(savedMousePosition.x, savedMousePosition.y);
            firstRightClick = true;
        }
        else if (ImGui::IsWindowHovered() && RaylibWrapper::GetMouseWheelMove() != 0 && !RaylibWrapper::IsMouseButtonDown(RaylibWrapper::MOUSE_BUTTON_RIGHT))
        {
            if (ProjectManager::projectData.is3D)
            {
                int multiplier = 1;
                if (RaylibWrapper::IsKeyDown(RaylibWrapper::KEY_LEFT_SHIFT) || RaylibWrapper::IsKeyDown(RaylibWrapper::KEY_RIGHT_SHIFT))
                    multiplier = 2;

                float zoomSpeed = cameraSpeed * multiplier;
                if (zoomSpeed > maxCameraSpeed)
                    zoomSpeed = maxCameraSpeed;

                RaylibWrapper::Vector3 forward = RaylibWrapper::Vector3Normalize(RaylibWrapper::Vector3Subtract(camera.target, camera.position));
                RaylibWrapper::Vector3 movement = RaylibWrapper::Vector3Scale(forward, RaylibWrapper::GetMouseWheelMove() * 2.0f * zoomSpeed);

                camera.position = RaylibWrapper::Vector3Add(camera.position, movement);
                camera.target = RaylibWrapper::Vector3Add(camera.target, movement);
            }
            else
            {
                int multiplier = 1;
                if (RaylibWrapper::IsKeyDown(RaylibWrapper::KEY_LEFT_SHIFT) || RaylibWrapper::IsKeyDown(RaylibWrapper::KEY_RIGHT_SHIFT))
                    multiplier = 2;

                float newZoom = camera.fovy - RaylibWrapper::GetMouseWheelMove() * 5 * multiplier;
                if (newZoom > 0 && newZoom < 250)
                    camera.fovy = newZoom;
            }
        }

        // Render 2D move tool arrows
        if (selectedObject != nullptr && !ProjectManager::projectData.is3D)
        {
            Vector3 pos = selectedObject->transform.GetPosition();
            RaylibWrapper::Vector2 position = RaylibWrapper::GetWorldToScreen({ pos.x, pos.y }, camera);
            position.x = position.x / RaylibWrapper::GetScreenWidth() * ImGui::GetWindowSize().x;
            position.y = position.y / RaylibWrapper::GetScreenHeight() * ImGui::GetWindowSize().y;

            // Y-axis arrow (Green) - Vertical
            ImGui::SetCursorPos(ImVec2(position.x - 10, position.y - 35));
            RaylibWrapper::rlImGuiImageSizeV(IconManager::imageTextures["GreenArrow"], { 20, 40 });
            if (!movingObjectY && ImGui::IsItemHovered() && RaylibWrapper::IsMouseButtonDown(RaylibWrapper::MOUSE_BUTTON_LEFT))
                movingObjectY = true;

            // X-axis arrow (Red) - Horizontal
            ImGui::SetCursorPos(ImVec2(position.x + 5, position.y));
            RaylibWrapper::rlImGuiImageSizeV(IconManager::imageTextures["RedArrow"], { 40, 20 });
            if (!movingObjectX && ImGui::IsItemHovered() && RaylibWrapper::IsMouseButtonDown(RaylibWrapper::MOUSE_BUTTON_LEFT))
                movingObjectX = true;

            // XY move tool (combined movement)
            ImGui::SetCursorPos(ImVec2(position.x + 3, position.y - 8));
            RaylibWrapper::rlImGuiImageSizeV(IconManager::imageTextures["XYMoveTool"], { 15, 15 });
            if (!movingObjectX && !movingObjectY && ImGui::IsItemHovered() && RaylibWrapper::IsMouseButtonDown(RaylibWrapper::MOUSE_BUTTON_LEFT))
                movingObjectX = movingObjectY = true;

            // Handle 2D movement
            if (movingObjectX || movingObjectY || movingObjectZ)
            {
                RaylibWrapper::Vector2 mousePosition = RaylibWrapper::GetMousePosition();
                RaylibWrapper::Vector2 deltaMouse = RaylibWrapper::Vector2Subtract(mousePosition, lastMousePosition);

                if (movingObjectX)
                    pos.x += deltaMouse.x / 15;
                if (movingObjectY)
                    pos.y -= deltaMouse.y / 15;
                if (movingObjectZ)
                    pos.z -= deltaMouse.x / 15;

                selectedObject->transform.SetPosition(pos);
            }
        }

        if (RaylibWrapper::IsMouseButtonReleased(RaylibWrapper::MOUSE_BUTTON_LEFT))
            movingObjectY = movingObjectX = false;

        // Select game object by clicking it
        // Todo: This does not work properly. It has an offset on the Y axis and for some reason different zoom levels (the camera fovy) effects the position. Rotation is also currently not considered.
        if (ImGui::IsWindowHovered() && !movingObjectX && !movingObjectY && !movingObjectZ && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            RaylibWrapper::Vector2 mousePosition = RaylibWrapper::GetMousePosition();
            mousePosition.x = (mousePosition.x - viewportPosition.x) / (viewportPosition.z - viewportPosition.x) * RaylibWrapper::GetScreenWidth();
            mousePosition.y = (mousePosition.y - viewportPosition.y) / (viewportPosition.w - viewportPosition.y) * RaylibWrapper::GetScreenHeight();

            RaylibWrapper::Vector3 rayPosition = RaylibWrapper::GetScreenToWorldRay(mousePosition, camera).position;

            //ConsoleLogger::InfoLog("X: " + std::to_string(rayPosition.x) + " Y: " + std::to_string(rayPosition.y));

            GameObject* closestObject = nullptr;
            float distance = FLT_MAX;

            if (ProjectManager::projectData.is3D)
            {
                // Todo: Implement this
            }
            else
            {
                for (GameObject* gameObject : SceneManager::GetActiveScene()->GetGameObjects())
                {
                    if (!gameObject->IsActive() || !gameObject->IsGlobalActive())
                        continue;

                    SpriteRenderer* spriteRenderer = gameObject->GetComponent<SpriteRenderer>();

                    if (!spriteRenderer || !spriteRenderer->IsActive())
                        continue;

                    if (spriteRenderer->IntersectsPoint({ rayPosition.x, rayPosition.y }))
                    {
                        float zPos = gameObject->transform.GetPosition().z;
                        if (zPos < distance)
                        {
                            distance = zPos;
                            closestObject = gameObject;
                        }
                    }
                }
            }

            if (closestObject)
            {
                selectedObject = closestObject;
                objectInProperties = selectedObject;
            }
            else if (selectedObject)
            {
                selectedObject = nullptr;
                objectInProperties = std::monostate{};
            }
        }

        lastMousePosition = RaylibWrapper::GetMousePosition();
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void Editor::RenderGameView()
{
    if (!gameViewOpen)
        return;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.11f, 0.12f, 1.00f));

    ImGui::SetNextWindowClass(&EditorWindow::defaultWindowClass);
    if (ImGui::Begin((ICON_FA_GAMEPAD + std::string(" Game")).c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar))
    {
        if (playModeActive && asyncPBODisplay.initialized)
        {
            asyncPBODisplay.Update();

            ImVec2 availSize = ImGui::GetContentRegionAvail();
            RaylibWrapper::Texture2D& texture = asyncPBODisplay.sharedTexture;

            float ratio = (float)texture.width / texture.height;

            float width = availSize.x;
            float height = width / ratio;
            if (height > availSize.y)
            {
                height = availSize.y;
                width = height * ratio;
            }

            // Center the image
            ImGui::SetCursorPos(ImVec2((availSize.x - width) * 0.5f, (availSize.y - height) * 0.5f));

            // Flip the Y-axis
            RaylibWrapper::Rectangle src = { 0, texture.height, (float)texture.width, -float(texture.height) };
            rlImGuiImageRect(&texture, width, height, src);
        }
        else
        {
            if (playModeActive)
                asyncPBODisplay.ConnectToSharedMemory();
            rlImGuiImageRenderTextureFit(&cameraRenderTexture, true);
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void Editor::UpdateViewport()
{
    if (!viewportOpen)
        return;

    if (RaylibWrapper::IsWindowResized())
    {
        UnloadRenderTexture(ViewTexture);
        ViewTexture = RaylibWrapper::LoadRenderTexture(RaylibWrapper::GetScreenWidth(), RaylibWrapper::GetScreenHeight());
    }

    // Load new models/textures on drag&drop
    //if (viewportHovered && RaylibWrapper::IsFileDropped())
    //{
    //    FilePathList droppedFiles = LoadDroppedFiles();

    //    if (droppedFiles.count == 1) // Todo: Add support for multiple files dropped.
    //    {
    //        if (IsFileExtension(droppedFiles.paths[0], ".obj") ||
    //            IsFileExtension(droppedFiles.paths[0], ".gltf") ||
    //            IsFileExtension(droppedFiles.paths[0], ".glb") ||
    //            IsFileExtension(droppedFiles.paths[0], ".vox") ||
    //            IsFileExtension(droppedFiles.paths[0], ".iqm") ||
    //            IsFileExtension(droppedFiles.paths[0], ".m3d"))       // Model file formats supported
    //        {
    //            std::filesystem::path folderPath = ProjectManager::projectData.path / "Assets" / "Models";
    //            if (!std::filesystem::exists(folderPath))
    //                std::filesystem::create_directories(folderPath);

    //            fileExplorerPath = folderPath;
    //            std::filesystem::path filePath = droppedFiles.paths[0];

    //            std::string filename = filePath.filename().string();
    //            std::string extension = filePath.extension().string();

    //            int counter = 1;
    //            while (std::filesystem::exists(folderPath / filename)) {
    //                // File with the same name exists, append a number to the filename
    //                filename = filePath.stem().string() + std::to_string(counter) + extension;
    //                counter++;
    //            }

    //            std::filesystem::copy_file(droppedFiles.paths[0], folderPath / filename);

    //            GameObject* gameObject = SceneManager::GetActiveScene()->AddGameObject();
    //            Material material;
    //            MeshRenderer& meshRenderer = gameObject->AddComponentInternal<MeshRenderer>();
    //            meshRenderer.SetModelPath(folderPath / filename);
    //            meshRenderer.SetModel(LoadModel(meshRenderer.GetModelPath().string().c_str()));
    //            gameObject->transform.SetPosition({ 0,0,0 });
    //            //gameObject->SetRealSize(gameObject->GetModel().s);
    //            gameObject->transform.SetScale({ 1,1,1 });
    //            gameObject->transform.SetRotation(QuaternionIdentity());
    //            gameObject->SetName((folderPath / filename).stem().string());
    //            //material.SetDiffuseMap();
    //            // gameObject->GetModel().materials[0].maps[MATERIAL_MAP_DIFFUSE].texture
    //            //gameObject->SetMaterial(.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture; // Set current map diffuse texture

    //            //SceneManager::GetActiveScene()->AddGameObject(gameObject);

    //            for (Component* component : SceneManager::GetActiveScene()->GetGameObjects().back()->GetComponents())
    //            {
    //                component->gameObject = SceneManager::GetActiveScene()->GetGameObjects().back();
    //            }

    //            selectedObject = SceneManager::GetActiveScene()->GetGameObjects().back();
    //            objectInProperties = SceneManager::GetActiveScene()->GetGameObjects().back();

    //            // TODO: Move camera position from target enough distance to visualize model properly
    //        }
    //        else if (IsFileExtension(droppedFiles.paths[0], ".png"))  // Texture file formats supported
    //        {
    //            // Unload current model texture and load new one
    //            //UnloadTexture(texture);
    //            //texture = LoadTexture(droppedFiles.paths[0]);
    //            //model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    //        }
    //    }

    //    UnloadDroppedFiles(droppedFiles);    // Unload filepaths from memory
    //}

    //float period = 10;
    //float magnitude = 25;

    //camera.position.x = (float)(sinf((float)GetTime() / period) * magnitude);

    // Shadows
    //if (CameraComponent::main != nullptr)
    //{
    //    Vector3 mainCameraPos = CameraComponent::main->gameObject->transform.GetPosition();
    //    shadowManager.camera.position = { mainCameraPos.x + Lighting::main->gameObject->transform.GetPosition().x,
    //        mainCameraPos.y + Lighting::main->gameObject->transform.GetPosition().y,
    //        mainCameraPos.z + Lighting::main->gameObject->transform.GetPosition().z };
    //}

    // Shadows
    if (ProjectManager::projectData.is3D)
    {
        RaylibWrapper::rlEnableShader(ShadowManager::shader.id);

        int index = 1;
        for (Lighting* light : Lighting::lights)
        {
            if (index > 15) // OpenGL only supports upto 16 active textures. 0 is reserved, so 1-15 can be used for lighting. The rest of the lights do not get rendered
                break;

            if (light->IsActive() && light->gameObject->IsGlobalActive() && light->gameObject->IsActive())
            {
                light->RenderLight(index);
                index++;
            }
            else
                light->EditorUpdate(); // This is needed so when the light gets disabled, it can run Disable()
        }
    }

    RaylibWrapper::BeginTextureMode(ViewTexture);

    RaylibWrapper::ClearBackground(ProjectManager::projectData.is3D ? RaylibWrapper::Color{ 135, 206, 235, 255 } : RaylibWrapper::Color{ 128, 128, 128, 255 });


    // Todo: For all shadow code, check if its 3D (rendering, init, and cleanup)

    //RaylibWrapper::rlSetUniformMatrix(RaylibWrapper::GetShaderLocation(shadowManager.shader, "matLightVP"), matLightVP);
    //RaylibWrapper::SetShaderValueTexture(shadowManager.shader, RaylibWrapper::GetShaderLocation(shadowManager.shader, "texture_shadowmap"), shadowManager.shadowMapTexture.depth); // Todo: should the last be depth or texture

    RaylibWrapper::BeginMode3D(camera);

    // Handle 3D movement arrows
    if (selectedObject != nullptr && ProjectManager::projectData.is3D)
    {
        Vector3 pos = selectedObject->transform.GetPosition();
        RaylibWrapper::Vector3 position = { pos.x, pos.y, pos.z };

        // Movement handling
        if (movingObjectX || movingObjectY || movingObjectZ)
        {
            RaylibWrapper::Vector2 mousePosition = RaylibWrapper::GetMousePosition();
            RaylibWrapper::Vector2 deltaMouse = RaylibWrapper::Vector2Subtract(mousePosition, lastMousePosition);

            float sensitivity = 0.01f;

            if (movingObjectX)
            {
                Vector3 worldRight = { 1, 0, 0 };
                RaylibWrapper::Vector3 tempPos = RaylibWrapper::Vector3Add(position, RaylibWrapper::Vector3Scale({ worldRight.x, worldRight.y, worldRight.z }, deltaMouse.x * sensitivity));
                position = tempPos;
            }
            if (movingObjectY)
            {
                Vector3 worldUp = { 0, 1, 0 };
                RaylibWrapper::Vector3 tempPos = RaylibWrapper::Vector3Add(position, RaylibWrapper::Vector3Scale({ worldUp.x, worldUp.y, worldUp.z }, -deltaMouse.y * sensitivity));
                position = tempPos;
            }
            if (movingObjectZ)
            {
                Vector3 worldForward = { 0, 0, 1 };
                RaylibWrapper::Vector3 tempPos = RaylibWrapper::Vector3Add(position, RaylibWrapper::Vector3Scale({ worldForward.x, worldForward.y, worldForward.z }, -deltaMouse.x * sensitivity));
                position = tempPos;
            }

            // Update the object position
            selectedObject->transform.SetPosition({ position.x, position.y, position.z });
            pos = { position.x, position.y, position.z }; // Keep pos in sync
        }

        // Arrow drawing parameters
        float baseArrowLength = 1.0f;
        float baseArrowThickness = 0.05f;
        float baseArrowHeadSize = 0.2f;

        // Calculate distance to camera for consistent sizing
        float distanceToCamera = RaylibWrapper::Vector3Distance(position, camera.position);
        float scaleFactor = fmaxf(distanceToCamera * 0.1f, 0.5f);

        float arrowLength = baseArrowLength * scaleFactor;
        float arrowThickness = baseArrowThickness * scaleFactor;
        float arrowHeadSize = baseArrowHeadSize * scaleFactor;

        // X-axis arrow (Red)
        RaylibWrapper::Vector3 xEnd = RaylibWrapper::Vector3Add(position, { arrowLength, 0, 0 });
        RaylibWrapper::DrawCylinderEx(position, xEnd, arrowThickness, arrowThickness, 8, RaylibWrapper::RED);
        RaylibWrapper::DrawCylinderEx(xEnd, RaylibWrapper::Vector3Add(xEnd, { arrowHeadSize, 0, 0 }), arrowHeadSize, 0, 8, RaylibWrapper::RED);

        // Y-axis arrow (Green)
        RaylibWrapper::Vector3 yEnd = RaylibWrapper::Vector3Add(position, { 0, arrowLength, 0 });
        RaylibWrapper::DrawCylinderEx(position, yEnd, arrowThickness, arrowThickness, 8, RaylibWrapper::GREEN);
        RaylibWrapper::DrawCylinderEx(yEnd, RaylibWrapper::Vector3Add(yEnd, { 0, arrowHeadSize, 0 }), arrowHeadSize, 0, 8, RaylibWrapper::GREEN);

        // Z-axis arrow (Blue)
        RaylibWrapper::Vector3 zEnd = RaylibWrapper::Vector3Add(position, { 0, 0, arrowLength });
        RaylibWrapper::DrawCylinderEx(position, zEnd, arrowThickness, arrowThickness, 8, RaylibWrapper::BLUE);
        RaylibWrapper::DrawCylinderEx(zEnd, RaylibWrapper::Vector3Add(zEnd, { 0, 0, arrowHeadSize }), arrowHeadSize, 0, 8, RaylibWrapper::BLUE);

        // Plane squares
        float squareSize = arrowLength * 0.3f;
        float squareOffset = arrowLength * 0.2f;

        // XY plane square (Red-Green)
        RaylibWrapper::Vector3 xySquarePos = RaylibWrapper::Vector3Add(position, { squareOffset, squareOffset, 0 });
        RaylibWrapper::DrawCube(xySquarePos, squareSize, squareSize, 0.02f, { 253, 249, 0, 77 });
        RaylibWrapper::DrawCubeWires(xySquarePos, squareSize, squareSize, 0.02f, { 253, 249, 0, 255 });

        // XZ plane square (Red-Blue)
        RaylibWrapper::Vector3 xzSquarePos = RaylibWrapper::Vector3Add(position, { squareOffset, 0, squareOffset });
        RaylibWrapper::DrawCube(xzSquarePos, squareSize, 0.02f, squareSize, { 255, 0, 255, 77 });
        RaylibWrapper::DrawCubeWires(xzSquarePos, squareSize, 0.02f, squareSize, { 255, 0, 255, 255 });

        // YZ plane square (Green-Blue)
        RaylibWrapper::Vector3 yzSquarePos = RaylibWrapper::Vector3Add(position, { 0, squareOffset, squareOffset });
        RaylibWrapper::DrawCube(yzSquarePos, 0.02f, squareSize, squareSize, { 255, 161, 0, 77 });
        RaylibWrapper::DrawCubeWires(yzSquarePos, 0.02f, squareSize, squareSize, { 255, 161, 0, 255 });

        // Input
        if (RaylibWrapper::IsMouseButtonPressed(RaylibWrapper::MOUSE_BUTTON_LEFT))
        {
            RaylibWrapper::Ray mouseRay = RaylibWrapper::GetMouseRay(RaylibWrapper::GetMousePosition(), camera);
            float minDistance = 10000.0f;
            int selectedAxis = 0; // 0=none, 1=X, 2=Y, 3=Z, 4=XY, 5=XZ, 6=YZ

            // Calculate arrow dimensions for collision detection
            float distanceToCamera = RaylibWrapper::Vector3Distance(position, camera.position);
            float scaleFactor = fmaxf(distanceToCamera * 0.1f, 0.5f);
            float arrowLength = 1.0f * scaleFactor;
            float arrowThickness = 0.05f * scaleFactor;
            float arrowHeadSize = 0.2f * scaleFactor;
            float squareSize = arrowLength * 0.3f;
            float squareOffset = arrowLength * 0.2f;
            float collisionPadding = arrowThickness * 3;

            // Todo: Consider usingn GetRayCollisionMesh() instead

            // Check X-axis arrow
            RaylibWrapper::RayCollision xCollision = RaylibWrapper::GetRayCollisionBox(mouseRay,
                { position.x - collisionPadding, position.y - arrowThickness - collisionPadding, position.z - arrowThickness - collisionPadding,
                 position.x + arrowLength + arrowHeadSize + collisionPadding, position.y + arrowThickness + collisionPadding, position.z + arrowThickness + collisionPadding });
            if (xCollision.hit && xCollision.distance < minDistance) {
                minDistance = xCollision.distance;
                selectedAxis = 1;
            }

            // Check Y-axis arrow
            RaylibWrapper::RayCollision yCollision = RaylibWrapper::GetRayCollisionBox(mouseRay,
                { position.x - arrowThickness - collisionPadding, position.y - collisionPadding, position.z - arrowThickness - collisionPadding,
                 position.x + arrowThickness + collisionPadding, position.y + arrowLength + arrowHeadSize + collisionPadding, position.z + arrowThickness + collisionPadding });
            if (yCollision.hit && yCollision.distance < minDistance) {
                minDistance = yCollision.distance;
                selectedAxis = 2;
            }

            // Check Z-axis arrow
            RaylibWrapper::RayCollision zCollision = RaylibWrapper::GetRayCollisionBox(mouseRay,
                { position.x - arrowThickness - collisionPadding, position.y - arrowThickness - collisionPadding, position.z - collisionPadding,
                 position.x + arrowThickness + collisionPadding, position.y + arrowThickness + collisionPadding, position.z + arrowLength + arrowHeadSize + collisionPadding });
            if (zCollision.hit && zCollision.distance < minDistance) {
                minDistance = zCollision.distance;
                selectedAxis = 3;
            }

            // Check XY plane
            RaylibWrapper::Vector3 xySquarePos = RaylibWrapper::Vector3Add(position, { squareOffset, squareOffset, 0 });
            RaylibWrapper::RayCollision xyCollision = RaylibWrapper::GetRayCollisionBox(mouseRay,
                { xySquarePos.x - squareSize / 2, xySquarePos.y - squareSize / 2, xySquarePos.z - 0.01f * scaleFactor,
                 xySquarePos.x + squareSize / 2, xySquarePos.y + squareSize / 2, xySquarePos.z + 0.01f * scaleFactor });
            if (xyCollision.hit && xyCollision.distance < minDistance) {
                minDistance = xyCollision.distance;
                selectedAxis = 4;
            }

            // Check XZ plane
            RaylibWrapper::Vector3 xzSquarePos = RaylibWrapper::Vector3Add(position, { squareOffset, 0, squareOffset });
            RaylibWrapper::RayCollision xzCollision = RaylibWrapper::GetRayCollisionBox(mouseRay,
                { xzSquarePos.x - squareSize / 2, xzSquarePos.y - 0.01f, xzSquarePos.z - squareSize / 2 * scaleFactor,
                 xzSquarePos.x + squareSize / 2, xzSquarePos.y + 0.01f, xzSquarePos.z + squareSize / 2 * scaleFactor });
            if (xzCollision.hit && xzCollision.distance < minDistance) {
                minDistance = xzCollision.distance;
                selectedAxis = 5;
            }

            // Check YZ plane
            RaylibWrapper::Vector3 yzSquarePos = RaylibWrapper::Vector3Add(position, { 0, squareOffset, squareOffset });
            RaylibWrapper::RayCollision yzCollision = RaylibWrapper::GetRayCollisionBox(mouseRay,
                { yzSquarePos.x - 0.01f, yzSquarePos.y - squareSize / 2, yzSquarePos.z - squareSize / 2 * scaleFactor,
                 yzSquarePos.x + 0.01f, yzSquarePos.y + squareSize / 2, yzSquarePos.z + squareSize / 2 * scaleFactor });
            if (yzCollision.hit && yzCollision.distance < minDistance) {
                minDistance = yzCollision.distance;
                selectedAxis = 6;
            }

            // Set movement flags based on selected axis
            movingObjectX = movingObjectY = movingObjectZ = false;
            switch (selectedAxis) {
                case 1: movingObjectX = true; break;
                case 2: movingObjectY = true; break;
                case 3: movingObjectZ = true; break;
                case 4: movingObjectX = movingObjectY = true; break;
                case 5: movingObjectX = movingObjectZ = true; break;
                case 6: movingObjectY = movingObjectZ = true; break;
            }
        }
    }

    if (RaylibWrapper::IsMouseButtonReleased(RaylibWrapper::MOUSE_BUTTON_LEFT))
        movingObjectY = movingObjectX = movingObjectZ = false;

    if (ProjectManager::projectData.is3D)
        RaylibWrapper::DrawGrid(100, 10.0f);

    deltaTime = RaylibWrapper::GetFrameTime();

    for (GameObject* gameObject : SceneManager::GetActiveScene()->GetGameObjects())
    {
        if (!gameObject->IsActive() || !gameObject->IsGlobalActive())
            continue;

        for (Component* component : gameObject->GetComponents())
        {
            if (!component->IsActive())
                continue;

            component->EditorUpdate();

            if (component->runInEditor)
                component->Update();

            component->Render();
        }
    }

    for (RenderableTexture* texture : RenderableTexture::textures) // Renders Sprites and Tilemaps
        if (texture)
            texture->Render();

    switch (dragData.first)
    {
    case ImageFile:
        ImVec2 mousePos = ImGui::GetMousePos();
        viewportHovered = (mousePos.x >= viewportPosition.x && mousePos.x <= viewportPosition.z &&
            mousePos.y >= viewportPosition.y && mousePos.y <= viewportPosition.w); // Todo: This being set 3 times a frame. First in UpdateViewport (if dragging), then RenderViewport, then here.

        if (viewportHovered)
        {
            RaylibWrapper::Vector2 mousePosition = RaylibWrapper::GetMousePosition();
            mousePosition.x = (mousePosition.x - viewportPosition.x) / (viewportPosition.z - viewportPosition.x) * RaylibWrapper::GetScreenWidth();
            mousePosition.y = (mousePosition.y - viewportPosition.y) / (viewportPosition.w - viewportPosition.y) * RaylibWrapper::GetScreenHeight();

            RaylibWrapper::Vector3 position = RaylibWrapper::GetScreenToWorldRay(mousePosition, camera).position;
            RaylibWrapper::Texture2D texture = std::any_cast<RaylibWrapper::Texture2D>(dragData.second["Texture"]);
            float centerWidth = texture.width / 2.0f;
            float centerHeight = texture.height / 2.0f;

            RaylibWrapper::DrawTextureProFlipped(texture,
                { 0, 0, static_cast<float>(texture.width), static_cast<float>(texture.height) * -1 },
                { position.x, position.y, static_cast<float>(texture.width) / 10, static_cast<float>(texture.height) / 10},
                { static_cast<float>(texture.width) / 10 / 2, static_cast<float>(texture.height) / 10 / 2 },
                0,
                { 255, 255, 255, 255 });
        }
        break;
    }

    // Shadows
    if (ProjectManager::projectData.is3D)
    {
        RaylibWrapper::rlDisableShader();
        RaylibWrapper::rlActiveTextureSlot(0);
    }

    RaylibWrapper::EndMode3D();
    RaylibWrapper::EndTextureMode();
}

void Editor::RenderContentBrowser() // Todo: Handle if path is in a now deleted folder.
{
    if (resetFileExplorerWin)
    {
        resetFileExplorerWin = false;
        fileExplorerPath = ProjectManager::projectData.path / "Assets"; // Todo: Make sure Assets path exists, if not then create it.
    }

    static std::filesystem::path renamingFile = "";
    static char newFileName[256] = "";
    static bool fileRenameFirstFrame = true;

    // Checks if the directory changed, or if the file is deleted/moved
    if (!renamingFile.empty() && (fileExplorerPath != renamingFile.parent_path() || !std::filesystem::exists(renamingFile)))
        renamingFile = "";

    ImGuiWindowFlags windowFlags = ImGuiTableFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;
    float nextX = 310, nextY = 55;

    ImGui::SetNextWindowClass(&EditorWindow::defaultWindowClass);
    if (ImGui::Begin((ICON_FA_FOLDER_OPEN + std::string(" Content Browser")).c_str(), nullptr, windowFlags))
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));

        // Add Button
        ImGui::SetCursorPos(ImVec2(305, 22));
        if (RaylibWrapper::rlImGuiImageButtonSize("##FileAddButton", IconManager::imageTextures["AddIcon"], ImVec2(16, 16)))
        {
            explorerContextMenuOpen = true; // Todo: Not working
            explorerContextMenuFile = "";
        }

        // Breadcrumb navigation
        std::filesystem::path fileExplorerTempPath = fileExplorerPath;
        std::filesystem::path path = ProjectManager::projectData.path;
        ImGui::SetCursorPos(ImVec2(335, 22));
        bool relative = false;
        for (const auto& dir : fileExplorerTempPath)
        {
            if (!relative && dir == "Assets")
                relative = true;
            if (relative)
            {
                if (dir != "Assets")
                {
                    ImGui::Text(ICON_FA_ARROW_RIGHT);
                    ImGui::SameLine();
                }
                path /= dir;
                if (ImGui::Button((dir.string() + "##" + std::to_string(ImGui::GetCursorPosX())).c_str()))
                    fileExplorerPath = path;
                ImGui::SameLine();
            }
        }

        // Search box

        // Search Icon
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 260, 26));
        ImGui::PushFont(FontManager::GetFont("Roboto-Medium", 12, false));

        ImGui::SameLine(ImGui::GetCursorPosX());
        RaylibWrapper::rlImGuiImageSize(IconManager::imageTextures["SearchIcon"], 22, 22);

        // Input box
        ImGui::SetCursorPos({ ImGui::GetWindowWidth() - 240, 25 });
        ImGui::SetNextItemWidth(230);

        static char searchBuffer[256] = "";
        ImGui::InputText("##FileSearch", searchBuffer, sizeof(searchBuffer), 0);

        ImGui::PopFont();

        ImGui::GetWindowDrawList()->AddLine(ImVec2(ImGui::GetWindowPos().x + 300, ImGui::GetWindowPos().y + 45), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y + 45), IM_COL32(0, 0, 0, 255), 1);

        // Back Button if not in /Assets folder
        if (fileExplorerPath != ProjectManager::projectData.path / "Assets")
        {
            ImGui::PushID(fileExplorerPath.string().c_str());
            ImGui::SetCursorPosY(nextY);
            ImGui::SetCursorPosX(nextX);
            if (RaylibWrapper::rlImGuiImageButtonSize("##FileBackButton", IconManager::imageTextures["FolderIcon"], ImVec2(40, 40)))
            {
                fileExplorerPath = fileExplorerPath.parent_path();
                ImGui::PopStyleColor(2);
                ImGui::PopID();
                ImGui::End();
                return;
            }
            ImGui::SetCursorPosY(ImGui::GetCursorPosY());
            ImGui::SetCursorPosX(nextX + (40 / 2) - (ImGui::CalcTextSize("Back").x / 2) + ImGui::CalcTextSize("  ").x);
            ImGui::PushFont(FontManager::GetFont("Roboto-Medium", 12, false));
            ImGui::Text("Back");
            nextX += 60;
            ImGui::PopFont();
            ImGui::PopID();
        }

        std::filesystem::path folderHovering; // For file drag and drop
        if (!std::filesystem::exists(fileExplorerPath))
        {
            if (!std::filesystem::exists(ProjectManager::projectData.path / "Assets"))
                std::filesystem::create_directories(ProjectManager::projectData.path / "Assets");
            fileExplorerPath = ProjectManager::projectData.path / "Assets";
        }

        std::filesystem::path fileHovered;

        // File iteration
        std::vector<std::filesystem::directory_entry> filePaths;

        std::string search = searchBuffer;
        // Convnert the search to lowercase
        std::transform(search.begin(), search.end(), search.begin(), ::tolower);

        if (searchBuffer[0] == '\0')
        {
            for (const auto& entry : std::filesystem::directory_iterator(fileExplorerPath))
                filePaths.push_back(entry);
        }
        else // Search for all files recursively using the search
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(fileExplorerPath))
            {
                // Convert file name to lowercase
                std::string fileName = entry.path().filename().string();
                std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);

                if (fileName.find(search) != std::string::npos)
                    filePaths.push_back(entry);
            }
        }

        for (const auto& entry : filePaths)
        {
            ImGui::PushID(entry.path().string().c_str());
            std::string id = entry.path().string().c_str();
            std::string fileName = entry.path().stem().string();

            // Skip hidden files
            if (fileName[0] == '.')
            {
                ImGui::PopID();
                continue;
            }

            if (fileName.length() > 11)
                fileName = fileName.substr(0, 10) + "..";

            ImGui::SetCursorPosY(nextY);
            ImGui::SetCursorPosX(nextX);

            // Set focused item
            if (focusContentBrowserFile == entry.path().string()) // This doesn't show the navigation rectangle because apparently its hidden when navigating with a mouse (https://github.com/ocornut/imgui/issues/4054)
            {
                ImGui::SetKeyboardFocusHere();
                focusContentBrowserFile = "";
            }

            bool hovered = ImGui::IsMouseHoveringRect(ImGui::GetCursorScreenPos(), { ImGui::GetCursorScreenPos().x + 40, ImGui::GetCursorScreenPos().y + 38 }); // Using this since ImGui::IsItemHovered() won't work. ImGui::IsWindowHovered() also won't work for this. It seems these break when attempting to drag a button
            if (hovered) fileHovered = entry;

            // Todo: For ImageFile and ModelFile, add support for dropping into hierarchy
            // Todo: Support dropping and moving files usinng the contnent broswer tree and the "assets -> example -> example 2"
            auto handleDrag = [&](DragTypes dragType) {
                if (dragData.first == None && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 5)) // Todo: If the user holds down on nothing and moves mouse over a file, it will select that file
                {
                    dragData.first = dragType;
                    dragData.second["Path"] = entry.path();
                    if (dragType == ImageFile)
                        dragData.second["Texture"] = RaylibWrapper::LoadTexture(entry.path().string().c_str()); // Todo: Make sure this texture is being unloaded. It would be best to just use texture variable, but make sure to update the float in the textureCaches
                }
            };

            // Unified double-click handler for data files
            auto handleDataFileDoubleClick = [&](DataFileTypes type, bool createIfMissing = true) {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    std::string dataPath = entry.path().string() + (type == DataFileTypes::Material ? "" : ".data");
                    // Puts the data into objectInProperties so it is displayed in the properties window
                    if (!std::holds_alternative<DataFile>(objectInProperties) || std::filesystem::path(std::get<DataFile>(objectInProperties).path) != dataPath)
                    {
                        if (createIfMissing && !std::filesystem::exists(dataPath))
                            Utilities::CreateDataFile(entry.path());

                        DataFile dataFile;
                        dataFile.path = dataPath;
                        dataFile.type = type;
                        std::ifstream jsonFile(dataFile.path);
                        dataFile.json = nlohmann::json::parse(jsonFile);
                        objectInProperties = dataFile;
                    }
                }
            };

            if (entry.is_directory())
            {
                // Need to create custom hover check code since ImGui::IsItemHovered() won't trigger as when dragging a file the ImGui Mouse Position doesn't update
                //static bool hoveringButton = false;
                //ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetCursorScreenPos(), { ImGui::GetCursorScreenPos().x + 40, ImGui::GetCursorScreenPos().y + 38 }, IM_COL32(255,0,0,255));
                if (hovered && dragData.first != None)
                {
                    folderHovering = entry;
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.38f, 0.38f, 0.38f, 1.00f));
                }
                RaylibWrapper::rlImGuiImageButtonSize(("##" + id).c_str(), IconManager::imageTextures["FolderIcon"], ImVec2(40, 40));

                if (ImGui::IsItemHovered())
                {
                    handleDrag(Folder);
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        fileExplorerPath = entry.path();
                        if (hovered && dragData.first != None) ImGui::PopStyleColor(3);
                        else ImGui::PopStyleColor(2);
                        ImGui::PopID();
                        ImGui::End();
                        return;
                    }
                }
                if (hovered && dragData.first != None) ImGui::PopStyleColor();
            }
            else if (entry.is_regular_file())
            {
                std::string extension = entry.path().extension().string();

                // Define file type data
                struct FileTypeData {
                    const char* iconKey;
                    DragTypes dragType;
                    std::function<void()> doubleClickAction;
                };

                std::map<std::string, FileTypeData> fileTypes = {
                    {".cpp", {"CppIcon", Other, [&]() { std::system(("\"" + entry.path().string() + "\"").c_str()); }}},
                    {".h", {"HeaderIcon", Other, [&]() { std::system(("\"" + entry.path().string() + "\"").c_str()); }}},
                    {".mp3", {"SoundIcon", Other, [&]() { handleDataFileDoubleClick(DataFileTypes::Sound); }}},
                    {".wav", {"SoundIcon", Other, [&]() { handleDataFileDoubleClick(DataFileTypes::Sound); }}},
                    {".ogg", {"SoundIcon", Other, [&]() { handleDataFileDoubleClick(DataFileTypes::Sound); }}},
                    {".flac", {"SoundIcon", Other, [&]() { handleDataFileDoubleClick(DataFileTypes::Sound); }}},
                    {".qoa", {"SoundIcon", Other, [&]() { handleDataFileDoubleClick(DataFileTypes::Sound); }}},
                    {".xm", {"SoundIcon", Other, [&]() { handleDataFileDoubleClick(DataFileTypes::Sound); }}},
                    {".mod", {"SoundIcon", Other, [&]() { handleDataFileDoubleClick(DataFileTypes::Sound); }}},
                    {".ttf", {"FontIcon", Other, [&]() { handleDataFileDoubleClick(DataFileTypes::Sound); }}},
                    {".otf", {"FontIcon", Other, [&]() { handleDataFileDoubleClick(DataFileTypes::Sound); }}},
                    {".gltf", {"CubeIcon", ModelFile, []() {}}},
                    {".glb", {"CubeIcon", ModelFile, []() {}}},
                    {".obj", {"CubeIcon", ModelFile, []() {}}},
                    {".m3d", {"CubeIcon", ModelFile, []() {}}},
                    {".scene", {"SceneIcon", Other, [&]() {
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && SceneManager::GetActiveScene()->GetPath() != entry.path())
                        {
                            objectInProperties = std::monostate{};
                            SceneManager::SaveScene(SceneManager::GetActiveScene());
                            SceneManager::LoadScene(entry.path());
                        }
                    }}}
                };

                if (extension == ".png")  // Todo: Add jpg support. Will need to modify config.h and rebuild raylib
                {
                    if (auto it = cachedTextures.find(entry.path()); it != cachedTextures.end())
                        it->second.second = RaylibWrapper::GetTime();
                    else
                        cachedTextures[entry.path()] = std::make_pair(RaylibWrapper::LoadTexture(entry.path().string().c_str()), RaylibWrapper::GetTime());

                    RaylibWrapper::Texture& texture = cachedTextures[entry.path()].first;
                    float aspectRatio = (float)texture.width / (float)texture.height;
                    ImVec2 imageSize = ImVec2(aspectRatio > 1.0f ? 32 : 32 * aspectRatio, aspectRatio > 1.0f ? 32 / aspectRatio : 32);
                    int buttonSize = 32 + ImGui::GetStyle().FramePadding.x * 2;
                    ImVec2 cursorPos = ImGui::GetCursorPos();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                    ImGui::Button(("##" + id).c_str(), ImVec2(buttonSize, buttonSize));
                    ImGui::PopStyleColor();

                    if (ImGui::IsItemHovered())
                    {
                        handleDrag(ImageFile);
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                            std::system(("\"" + entry.path().string() + "\"").c_str());
                    }

                    ImVec2 newCursorPos = ImGui::GetCursorPos();
                    ImGui::SetCursorPos(ImVec2(cursorPos.x + (buttonSize - imageSize.x) / 2, cursorPos.y + (buttonSize - imageSize.y) / 2));
                    RaylibWrapper::rlImGuiImageSize(&texture, imageSize.x, imageSize.y);
                    ImGui::SetCursorPos(newCursorPos);
                }
                else if (extension == ".mat") // Material files
                {
                    RaylibWrapper::RenderTexture2D* materialIcon = CreateMaterialPreview(entry.path(), 32);
                    RaylibWrapper::Texture2D* texture = materialIcon ? &materialIcon->texture : IconManager::imageTextures["SoundIcon"];
                    RaylibWrapper::rlImGuiImageButtonSize(("##" + id).c_str(), texture, ImVec2(32, 32));
                    if (ImGui::IsItemHovered())
                    {
                        handleDrag(Other);
                        handleDataFileDoubleClick(DataFileTypes::Material, false);
                    }
                }
                else if (extension == ".ldtk") // LDtk files with complex logic
                {
                    RaylibWrapper::rlImGuiImageButtonSize(("##" + id).c_str(), IconManager::imageTextures["LDtkIcon"], ImVec2(40, 40));
                    if (ImGui::IsItemHovered())
                    {
                        handleDrag(Other);
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            std::string dataPath = entry.path().string() + ".data";

                            // Puts the data into objectInProperties so it is displayed in the properties window
                            if (!std::holds_alternative<DataFile>(objectInProperties) || std::filesystem::path(std::get<DataFile>(objectInProperties).path) != dataPath)
                            {
                                if (!std::filesystem::exists(dataPath))
                                {
                                    Utilities::CreateDataFile(entry.path());
                                    std::ofstream file(dataPath);
                                    if (file.is_open())
                                    {
                                        nlohmann::json jsonData = { {"public", nlohmann::json::object()}, {"private", {{"version", 1.0f}}} };

                                        // Opens the LDtk file, reads the tilesets, then adds the tilesets to File
                                        std::ifstream ldtkFile(entry.path());
                                        if (ldtkFile.is_open())
                                        {
                                            nlohmann::json data = nlohmann::json::parse(ldtkFile);
                                            for (int i = 0; i < data["defs"]["tilesets"].size(); i++)
                                                jsonData["public"][data["defs"]["tilesets"][i]["identifier"]] = { "file", { ".png", ".jpeg", ".jpg" }, "nullptr" };
                                        }
                                        file << std::setw(4) << jsonData << std::endl;
                                    }
                                    else
                                        ConsoleLogger::ErrorLog("Failed to open " + entry.path().filename().string());
                                }
                                else
                                {
                                    // Updates the data with the up-to-date LDtk data if needed
                                    std::ifstream fileR(dataPath);
                                    nlohmann::json jsonData = nlohmann::json::parse(fileR);
                                    fileR.close();
                                    std::ifstream ldtkFile(entry.path());
                                    if (ldtkFile.is_open()) // Todo: handle if the file doesn't open
                                    {
                                        nlohmann::json data = nlohmann::json::parse(ldtkFile);
                                        for (int i = 0; i < data["defs"]["tilesets"].size(); i++)
                                        {
                                            nlohmann::json id = data["defs"]["tilesets"][i]["identifier"];
                                            jsonData["public"][id] = jsonData["public"].contains(id) ?
                                                nlohmann::json{ "file", { ".png", ".jpeg", ".jpg" }, jsonData["public"][id][2] } :
                                                nlohmann::json{ "file", { ".png", ".jpeg", ".jpg" }, "nullptr" };
                                        }
                                        std::ofstream fileW(dataPath);
                                        if (fileW.is_open())
                                            fileW << std::setw(4) << jsonData << std::endl;
                                        else // Todo: Handle this
                                            ConsoleLogger::ErrorLog("Failed to open " + entry.path().filename().string());
                                    }
                                    else
                                        ConsoleLogger::ErrorLog("Failed to open " + entry.path().filename().string());
                                }
                                DataFile dataFile;
                                dataFile.path = dataPath;
                                dataFile.type = DataFileTypes::Tilemap;
                                std::ifstream jsonFile(dataFile.path);
                                dataFile.json = nlohmann::json::parse(jsonFile);
                                objectInProperties = dataFile;
                            }
                        }
                    }
                }
                //else if (extension == ".gltf" || extension == ".glb" || extension == ".obj" || extension == ".m3d")
                //{
                //    // Todo: This is causing errors if there are game objects with a MeshRenderer
                //    //if (RaylibWrapper::rlImGuiImageButtonSize(("##" + id).c_str(), &CreateModelPreview(entry.path(), 32)->texture, ImVec2(40, 40)))
                //    RaylibWrapper::rlImGuiImageButtonSize(("##" + id).c_str(), IconManager::imageTextures["CubeIcon"], ImVec2(40, 40));

                //    if (ImGui::IsItemHovered())
                //    {
                //        if (dragData.first == None && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 5))
                //        {
                //            dragData.first = ModelFile;
                //            dragData.second["Path"] = entry.path();
                //        }
                //    }
                //}
                else if (extension == ".animgraph")
                {
                    RaylibWrapper::rlImGuiImageButtonSize(("##" + id).c_str(), IconManager::imageTextures["AnimationGraphIcon"], ImVec2(40, 40));
                    if (ImGui::IsItemHovered())
                    {
                        handleDrag(Other);
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            // Todo: If something is already in the Animation Graph window and its not saved, popup asking to save, don't save, or cancel
                            // Todo: Create a AnimationGraph script and put OpenAnimationGraph() in it.

                            std::ifstream dataFile(entry.path());
                            if (dataFile.is_open())
                            {
                                // Todo: Check if the dataFile has data in it. If it doesn't then add default data to it so it won't crash.
                                dataFile >> animationGraphData;
                                animationGraphData["path"] = entry.path(); // Updating the path incase it changed.
                                if (animationGraphWinOpen) // If the animation graph is open, I need to focus it
                                {
                                    ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_PERSON_RUNNING + std::string(" Animation Graph")).c_str());
                                    if (window && window->DockNode && window->DockNode->TabBar)
                                        window->DockNode->TabBar->NextSelectedTabId = window->TabId;
                                }
                                else
                                    animationGraphWinOpen = true; // This should focus the animation graph window
                            }
                            else
                            {
                                // Todo: Send error message
                            }
                        }
                    }
                }
                else if (extension == ".es")
                {
                    RaylibWrapper::rlImGuiImageButtonSize(("##" + id).c_str(), IconManager::imageTextures["EventSheetIcon"], ImVec2(40, 40));
                    if (ImGui::IsItemHovered())
                    {
                        handleDrag(Other);
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            std::ifstream dataFile(entry.path());
                            if (dataFile.is_open())
                            {
                                bool foundWindow = false;
                                for (EditorWindow* window : windows)
                                {
                                    if (window->windowName == "EventSheet Editor" && window->windowID == entry.path().string())
                                    {
                                        ImGuiWindow* imGuiWindow = ImGui::FindWindowByName(window->fullWindowName.c_str());
                                        if (imGuiWindow && imGuiWindow->DockNode && imGuiWindow->DockNode->TabBar)
                                            imGuiWindow->DockNode->TabBar->NextSelectedTabId = imGuiWindow->TabId;
                                        foundWindow = true;
                                        break;
                                    }
                                }
                                if (!foundWindow)
                                {
                                    windows.push_back(new EventSheetEditor());
                                    windows.back()->Init("EventSheet Editor", entry.path().string(), true, ICON_FA_TABLE_COLUMNS, ImVec4(0.10f, 0.11f, 0.12f, 1.00f));
                                    // Todo: Check if the dataFile has data in it. If it doesn't then add default data to it so it won't crash.
                                    nlohmann::json data;
                                    dataFile >> data;
                                    static_cast<EventSheetEditor*>(windows.back())->LoadData(data, entry.path().string());

                                    // Todo: If something is already in the Canvas Editor window and its not saved, popup asking to save, don't save, or cancel

                                    //if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                                    //{
                                        //std::ifstream dataFile(entry.path());
                                        //if (dataFile.is_open())
                                        //{
                                        //    // Todo: Check if the dataFile has data in it. If it doesn't then add default data to it so it won't crash.
                                        //    dataFile >> CanvasEditor::canvasData;
                                        //    CanvasEditor::canvasData["path"] = entry.path(); // Updating the path incase it changed.

                                        //    if (CanvasEditor::windowOpen) // If the canvas editor is open, focus it
                                        //    {
                                        //        ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_BRUSH + std::string(" Canvas Editor")).c_str());
                                        //        if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                                        //            window->DockNode->TabBar->NextSelectedTabId = window->TabId;
                                        //    }
                                        //    else
                                        //        CanvasEditor::windowOpen = true; // This should focus the canvas editor window
                                        //}
                                        //else
                                        //{
                                        //    // Todo: Send error message
                                        //}
                                    //}
                                }
                            }
                        }
                    }
                }
                else if (extension == ".canvas")
                {
                    RaylibWrapper::rlImGuiImageButtonSize(("##" + id).c_str(), IconManager::imageTextures["CanvasIcon"], ImVec2(40, 40));
                    if (ImGui::IsItemHovered()) handleDrag(Other);
                }
                else if (auto it = fileTypes.find(extension); it != fileTypes.end())
                {
                    RaylibWrapper::rlImGuiImageButtonSize(("##" + id).c_str(), IconManager::imageTextures[it->second.iconKey], ImVec2(40, 40));
                    if (ImGui::IsItemHovered())
                    {
                        handleDrag(it->second.dragType);
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                            it->second.doubleClickAction();
                    }
                }
				else if (extension == ".prefab")
				{
					RaylibWrapper::rlImGuiImageButtonSize(("##" + id).c_str(), IconManager::imageTextures["PrefabIcon"], ImVec2(40, 40));
					if (ImGui::IsItemHovered())
					{
						handleDrag(Other);
						if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
						{
							std::ifstream dataFile(entry.path());
							if (dataFile.is_open())
							{
								bool foundWindow = false;
								for (EditorWindow* window : windows)
								{
									if (window->windowName == "Prefab Editor" && window->windowID == entry.path().string())
									{
										ImGuiWindow* imGuiWindow = ImGui::FindWindowByName(window->fullWindowName.c_str());
										if (imGuiWindow && imGuiWindow->DockNode && imGuiWindow->DockNode->TabBar)
											imGuiWindow->DockNode->TabBar->NextSelectedTabId = imGuiWindow->TabId;
										foundWindow = true;
										break;
									}
								}
								if (!foundWindow)
								{
									windows.push_back(new PrefabEditor());
									windows.back()->Init("EventSheet Editor", entry.path().string(), true, ICON_FA_TABLE_COLUMNS, ImVec4(0.10f, 0.11f, 0.12f, 1.00f));
									// Todo: Check if the dataFile has data in it. If it doesn't then add default data to it so it won't crash.
									nlohmann::json data;
									dataFile >> data;
									static_cast<PrefabEditor*>(windows.back())->LoadData(data, entry.path().string());

								}
							}
						}
					}
				}
                else if (extension != ".data")
                {
                    RaylibWrapper::rlImGuiImageButtonSize(("##" + id).c_str(), IconManager::imageTextures["UnknownFile"], ImVec2(40, 40));
                    if (ImGui::IsItemHovered())
                    {
                        handleDrag(Other);
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                            std::system(("\"" + entry.path().string() + "\"").c_str());
                    }
                }
                else
                {
                    ImGui::PopID();
                    continue;
                }
            }

            // File name rendering
            ImGui::PushFont(FontManager::GetFont("Roboto-Medium", 12, false));
            ImGui::SetCursorPosY(ImGui::GetCursorPosY());
            ImGui::SetCursorPosX(nextX + (40 / 2) - (ImGui::CalcTextSize(fileName.c_str()).x / 2) + ImGui::CalcTextSize("  ").x);

            if (!renamingFile.empty() && renamingFile.filename().string() == entry.path().filename().string())
            {
                ImGui::SetNextItemWidth(44);
                ImGui::SetKeyboardFocusHere(0);
                if (ImGui::InputText("##RenameInput", newFileName, sizeof(newFileName), ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (newFileName != "" || newFileName == renamingFile.stem())
                    {
                        // Todo: If a file already exists with that name and extension, popup a warning asking if they want to replace/overwrite or cancel
                        std::filesystem::rename(renamingFile, (renamingFile.parent_path() / (newFileName + renamingFile.extension().string())));
                        if (std::filesystem::exists(renamingFile.string() + ".data"))
                            std::filesystem::rename(renamingFile.string() + ".data", (renamingFile.parent_path() / (newFileName + renamingFile.extension().string() + ".data")));

                        // Check if a script file was renamed
                        if (renamingFile.extension() == ".cpp")
                        {
                            std::filesystem::path headerPath = renamingFile;
                            headerPath.replace_extension(".h");
                            if (std::filesystem::exists(headerPath))
                                std::filesystem::rename(headerPath, (renamingFile.parent_path() / (newFileName + std::string(".h"))));
                        }
                        else if (renamingFile.extension() == ".h")
                        {
                            std::filesystem::path cppPath = renamingFile;
                            cppPath.replace_extension(".cpp");
                            if (std::filesystem::exists(cppPath))
                                std::filesystem::rename(cppPath, (renamingFile.parent_path() / (newFileName + std::string(".cpp"))));
                        }
                        fileRenameFirstFrame = true;
                    }
                    renamingFile = "";
                    strcpy_s(newFileName, sizeof(newFileName), "");
                }
                if (!ImGui::IsItemClicked() && ImGui::IsKeyPressed(ImGuiKey_MouseLeft))
                {
                    renamingFile = "";
                    strcpy_s(newFileName, sizeof(newFileName), "");
                    fileRenameFirstFrame = true;
                }
            }
            else ImGui::Text(fileName.c_str());
            ImGui::PopFont();

            nextX += 60;
            if (nextX > ImGui::GetWindowWidth() - 32)
            {
                nextX = 310;
                nextY = nextY + 75;
            }
            ImGui::PopID();
        }
        ImGui::PopStyleColor(2);

        // Consolidated drag and drop handling
        auto setCursor = [](RaylibWrapper::MouseCursor cursor) {
            static RaylibWrapper::MouseCursor currentCursor = RaylibWrapper::MOUSE_CURSOR_DEFAULT;
            if (currentCursor != cursor) {
                RaylibWrapper::SetMouseCursor(cursor);
                currentCursor = cursor;
            }
        };

        auto isViewportHovered = []() {
            ImVec2 mousePos = ImGui::GetMousePos();
            return (mousePos.x >= viewportPosition.x && mousePos.x <= viewportPosition.z && mousePos.y >= viewportPosition.y && mousePos.y <= viewportPosition.w);
        };

        if (dragData.first != None)
        {
            bool validDrop = !folderHovering.empty() || (dragData.second.find("HoveringValidElement") != dragData.second.end() && std::any_cast<bool>(dragData.second["HoveringValidElement"]));

            // Todo: Show the model while placing it down, like with sprites
            if (dragData.first == ImageFile || dragData.first == ModelFile)
            {
                viewportHovered = isViewportHovered();
                setCursor((viewportHovered || validDrop) ? RaylibWrapper::MOUSE_CURSOR_DEFAULT : RaylibWrapper::MOUSE_CURSOR_NOT_ALLOWED);
            }
            else
            {
                setCursor(validDrop ? RaylibWrapper::MOUSE_CURSOR_DEFAULT : RaylibWrapper::MOUSE_CURSOR_NOT_ALLOWED);
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                if (!folderHovering.empty())
                {
                    try {
                        std::filesystem::path path = std::any_cast<std::filesystem::path>(dragData.second["Path"]);
                        std::filesystem::rename(path, folderHovering / path.filename());
                        if (std::filesystem::exists(path.string() + ".data"))
                            std::filesystem::rename(path.string() + ".data", folderHovering / (path.filename().string() + ".data"));
                    }
                    catch (const std::filesystem::filesystem_error& e) {
                        ConsoleLogger::ErrorLog(std::string("Error moving folder: ") + e.what(), false);
                    }
                    catch (const std::bad_any_cast& e) {
                        ConsoleLogger::ErrorLog(std::string("Error moving folder: ") + e.what());
                    }
                    catch (const std::exception& e) {
                        ConsoleLogger::ErrorLog(std::string("Error moving folder: ") + e.what());
                    }
                }
                else if ((dragData.first == ImageFile || dragData.first == ModelFile) && viewportHovered)
                {
                    // Handle viewport drops
                    RaylibWrapper::Vector2 mousePosition = RaylibWrapper::GetMousePosition();
                    mousePosition.x = (mousePosition.x - viewportPosition.x) / (viewportPosition.z - viewportPosition.x) * RaylibWrapper::GetScreenWidth();
                    mousePosition.y = (mousePosition.y - viewportPosition.y) / (viewportPosition.w - viewportPosition.y) * RaylibWrapper::GetScreenHeight();
                    RaylibWrapper::Vector3 position = RaylibWrapper::GetScreenToWorldRay(mousePosition, camera).position;

                    // Todo: Properly set the Z position using a raycast (that detects every game object, including ones without colliders)

                    std::filesystem::path filePath = std::any_cast<std::filesystem::path>(dragData.second["Path"]);
                    auto assetsPosition = filePath.string().find("Assets");
                    if (assetsPosition != std::string::npos)
                        filePath = filePath.string().substr(assetsPosition + 7);

                    GameObject* gameObject = SceneManager::GetActiveScene()->AddGameObject();
                    gameObject->transform.SetPosition({ position.x, position.y, dragData.first == ModelFile ? position.z : 0 });
                    gameObject->transform.SetScale({ 1,1,1 });
                    gameObject->transform.SetRotation(Quaternion::Identity());
                    gameObject->SetName(filePath.stem().string());

                    if (dragData.first == ImageFile)
                    {
                        std::string texturePathString = filePath.string();
                        for (char& c : texturePathString) // Reformatted the path for unix
                        {
                            if (c == '\\')
                                c = '/';
                        }

                        SpriteRenderer& spriteRenderer = gameObject->AddComponentInternal<SpriteRenderer>();
                        spriteRenderer.exposedVariables[1][0][2] = texturePathString;
                        gameObject->AddComponentInternal<Collider2D>(); // Todo: Set to convex/texture type
                    }
                    else if (dragData.first == ModelFile)
                    {
                        MeshRenderer& meshRenderer = gameObject->AddComponentInternal<MeshRenderer>();
                        meshRenderer.SetModelPath(filePath);
                        meshRenderer.SetModel(ModelType::Custom, filePath, ShaderManager::LitStandard);
                    }

                    for (Component* component : SceneManager::GetActiveScene()->GetGameObjects().back()->GetComponents())
                        component->gameObject = SceneManager::GetActiveScene()->GetGameObjects().back();

                    //if (selectedObject != nullptr)
                        //EventSystem::Invoke("ObjectDeselected", selectedObject);
                    selectedObject = SceneManager::GetActiveScene()->GetGameObjects().back();
                    objectInProperties = SceneManager::GetActiveScene()->GetGameObjects().back();
                    //EventSystem::Invoke("ObjectSelected", selectedObject);
                }
                else
                {
                    // Checks if hovering animation graph
                    ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_PERSON_RUNNING + std::string(" Animation Graph")).c_str());
                    // Todo: animationGraphHovered is always true if its open. It doesn't actually check if its being hovered. Look at dropping a model file into viewport and how im checking if viewport is hovered
                    if (animationGraphWinOpen && animationGraphHovered && window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL && window->DockNode->TabBar->SelectedTabId == window->TabId)
                    {
                        RaylibWrapper::Vector2 mousePosition = RaylibWrapper::GetMousePosition();
                        mousePosition.x = (mousePosition.x - viewportPosition.x) / (viewportPosition.z - viewportPosition.x) * RaylibWrapper::GetScreenWidth();
                        mousePosition.y = (mousePosition.y - viewportPosition.y) / (viewportPosition.w - viewportPosition.y) * RaylibWrapper::GetScreenHeight();
                        RaylibWrapper::Vector3 position = RaylibWrapper::GetScreenToWorldRay(mousePosition, camera).position;

                        // Todo: Place the nodes at the mouse position

                        bool updatedAnimations = false;

                        int index = 0;
                        std::random_device rd;
                        std::mt19937 gen(rd());
                        std::uniform_int_distribution<int> distribution(999999, 99999999);
                        // Todo: Popup a window with a progress bar
                        // Todo: This function is really slow to get animations
                        for (std::string& animationName : Utilities::GetGltfAnimationNames(std::any_cast<std::filesystem::path>(dragData.second["Path"])))
                        {
                            int id = 0;
                            while (id == 0)
                            {
                                id = distribution(gen);
                                for (auto& node : animationGraphData["nodes"])
                                {
                                    if (id == node["id"])
                                    {
                                        id = 0;
                                        break;
                                    }
                                }
                            }

                            nlohmann::json node = {
                                {"id", id},
                                {"name", animationName},
                                {"index", index},
                                {"x", 350 + (index * 30)},
                                {"y", 350 + (index * 30)},
                                {"loop", true},
                                {"speed", 1.0f}
                            };

                            animationGraphData["nodes"].push_back(node);
                            updatedAnimations = true;
                            index++;
                        }

                        if (updatedAnimations)
                        {
                            std::ofstream file(animationGraphData["path"].get<std::filesystem::path>());
                            if (file.is_open())
                            {
                                file << std::setw(4) << animationGraphData << std::endl;
                                file.close();
                            }
                        }
                    }
                }

                // Cleanup
                if (dragData.first == ImageFile)
                    RaylibWrapper::UnloadTexture(std::any_cast<RaylibWrapper::Texture2D>(dragData.second["Texture"]));
                dragData.first = None;
                dragData.second.clear();
                setCursor(RaylibWrapper::MOUSE_CURSOR_DEFAULT);
            }

            if (dragData.second.find("HoveringValidElement") != dragData.second.end())
                dragData.second["HoveringValidElement"] = false;
        }

        // Content Browser Tree
        int xSize = 300;
        ImGui::GetWindowDrawList()->AddLine(ImVec2(ImGui::GetWindowPos().x + xSize, ImGui::GetWindowPos().y), ImVec2(ImGui::GetWindowPos().x + xSize, ImGui::GetWindowPos().y + ImGui::GetWindowWidth()), IM_COL32(0, 0, 0, 255), 1);
        ImGui::SetCursorPos(ImVec2(0, 25));
        if (ImGui::BeginChild("##FileExplorerTree", ImVec2(xSize, ImGui::GetWindowHeight() - 35))) // Todo: Make this a dockable window or something so it resizes properly and can be resized. Check if I need this -35 (or if 35 is the best number)
        {
            ImGui::BeginTable("FileExplorerTreeTable", 1);
            RenderContentBrowserTreeNode(ProjectManager::projectData.path / "Assets", true);
            ImGui::EndTable();
        }
        ImGui::EndChild();

        // Context Menu
        auto closeContextMenu = [&]()
        {
            explorerContextMenuOpen = false;
            explorerContextMenuFile = "";
        };

        if (!explorerContextMenuOpen && (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)))
        {
            explorerContextMenuOpen = true;
            explorerContextMenuFile = fileHovered;
        }

        if (explorerContextMenuOpen)
        {
            explorerContextMenuOpen = true;
            if (ImGui::BeginPopupContextWindow())
            {
                // Create menu items
                struct MenuItem { const char* name; std::function<void()> action; };
                std::vector<MenuItem> createMenuItems = {
                    {"Create Script", [&]() { scriptCreateWinOpen = true; }},
                    {"Create EventSheet", [&]() {
                        std::filesystem::path filePath = Utilities::CreateUniqueFile(fileExplorerPath, "EventSheet", "es");
                        if (filePath != "")
                        {
                            std::ofstream file(filePath);
                            if (file.is_open())
                            {
                                nlohmann::json jsonData = {{"version", 1}, {"path", filePath}, {"events", nlohmann::json::array()}};
                                file << std::setw(4) << jsonData << std::endl;
                            }
                            else
                            {
                                // Todo: Properly handle if the file couldn't be opened. Maybe retry a few times, then popup with a message and delete the file.
                                std::filesystem::remove(filePath);
                            }
                            renamingFile = filePath;
                            strcpy_s(newFileName, sizeof(newFileName), filePath.stem().string().c_str());
                        }
                        else
                        {
                            // Todo: Handle if it wasn't created
                        }
                    }},
					{"Create Prefab", [&]() {
						std::filesystem::path filePath = Utilities::CreateUniqueFile(fileExplorerPath, "Prefab", "prefab");
						if (filePath != "")
						{
							std::ofstream file(filePath);
							if (file.is_open())
							{
								nlohmann::json jsonData = {{"version", 1}, {"path", filePath},
                                    {"gameObjects", {
                                        {"name", ""},
                                        {"position", {0,0,0}},
                                        {"size", {0,0,0}},
                                        {"rotation", {0,0,0}},
                                        {"id", -1},
                                        {"active", false},
                                        {"globalActive", false},
                                        {"components", nlohmann::json::array()}
                                    }}
                                };
								file << std::setw(4) << jsonData << std::endl;
							}
							else
							{
								// Todo: Properly handle if the file couldn't be opened. Maybe retry a few times, then popup with a message and delete the file.
								std::filesystem::remove(filePath);
							}
							renamingFile = filePath;
							strcpy_s(newFileName, sizeof(newFileName), filePath.stem().string().c_str());
						}
						else
						{
							// Todo: Handle if it wasn't created
						}
					}},
                    {"Create Scene", [&]() {
                        std::filesystem::path filePath = Utilities::CreateUniqueFile(fileExplorerPath, "New Scene", "scene");
                        SceneManager::CreateScene(filePath);
                        if (filePath != "")
                        {
                            renamingFile = filePath;
                            strcpy_s(newFileName, sizeof(newFileName), filePath.stem().string().c_str());
                        }
                        else
                        {
                            // Todo: Handle if it wasn't created
                        }
                    }},
                    {"Create Sprite", [&]() {}},
                    {"Create Material", [&]() {
                        std::filesystem::path filePath = Utilities::CreateUniqueFile(fileExplorerPath, "New Material", "mat");
                        if (filePath != "")
                        {
                            Utilities::CreateDataFile(filePath);
                            renamingFile = filePath;
                            strcpy_s(newFileName, sizeof(newFileName), filePath.stem().string().c_str());
                        }
                        else
                        {
                            // Todo: Handle if it wasn't created
                        }
                    }},
                    {"Create Animation Graph", [&]() {
                        std::filesystem::path filePath = Utilities::CreateUniqueFile(fileExplorerPath, "Animation Graph", "animgraph");
                        if (filePath != "")
                        {
                            std::ofstream file(filePath);
                            if (file.is_open())
                            {
                                nlohmann::json jsonData = {{"version", 1}, {"path", filePath}, {"model_path", ""},
                                    {"nodes", {{{"id", -5}, {"name", "Start"}, {"x", 80}, {"y", 350}, {"loop", false}, {"speed", 0.0f}}}},
                                    {"links", nlohmann::json::array()}};
                                file << std::setw(4) << jsonData << std::endl;
                            }
                            else
                            {
                                // Todo: Properly handle if the file couldn't be opened. Maybe retry a few times, then popup with a message and delete the file.=
                                std::filesystem::remove(filePath);
                            }
                            renamingFile = filePath;
                            strcpy_s(newFileName, sizeof(newFileName), filePath.stem().string().c_str());
                        }
                        else
                        {
                            // Todo: Handle if it wasn't created
                        }
                    }},
                    {"Create Canvas", [&]() {
                        std::filesystem::path filePath = Utilities::CreateUniqueFile(fileExplorerPath, "Canvas", "canvas");
                        std::string filePathString = filePath.string();
                        for (char& c : filePathString) if (c == '\\') c = '/';
                        if (filePath != "")
                        {
                            std::ofstream file(filePath);
                            if (file.is_open())
                            {
                                nlohmann::json jsonData = {{"version", 1}, {"path", filePathString}, {"gameobjects", nlohmann::json::array()}};
                                file << std::setw(4) << jsonData << std::endl;
                            }
                            else
                            {
                                // Todo: Properly handle if the file couldn't be opened. Maybe retry a few times, then popup with a message and delete the file.
                                std::filesystem::remove(filePath);
                            }
                            renamingFile = filePath;
                            strcpy_s(newFileName, sizeof(newFileName), filePath.stem().string().c_str());
                        }
                        else
                        {
                            // Todo: Handle if it wasn't created
                        }
                    }},
                    {"Create Folder", [&]() {
                        std::filesystem::path folderPath = Utilities::CreateUniqueFile(fileExplorerPath, "New Folder", "");
                        if (folderPath != "")
                        {
                            renamingFile = folderPath;
                            strcpy_s(newFileName, sizeof(newFileName), folderPath.stem().string().c_str());
                        }
                        else
                        {
                            // Todo: Handle if it wasn't created
                        }
                    }}
                };

                //if (ImGui::MenuItem("Create Animation"))
                //{
                //    explorerContextMenuOpen = false;
                //    std::filesystem::path filePath = Utilities::CreateUniqueFile(fileExplorerPath, "New Animation", "anim");
                //    if (filePath != "")
                //    {
                //        std::ofstream file(filePath);
                //        if (file.is_open())
                //        {
                //            nlohmann::json jsonData = {
                //                {"version", 1},
                //                {"path", filePath},
                //                {"loop", true},
                //                {"speed", 1.0f},
                //                {"sprites", {}}
                //            };

                //            file << std::setw(4) << jsonData << std::endl;
                //            file.close();
                //        }
                //        else
                //        {
                //            // Todo: Properly handle if the file couldn't be opened. Maybe retry a few times, then popup with a message and delete the file.
                //            std::filesystem::remove(filePath);
                //        }


                //        renamingFile = filePath;
                //        strcpy_s(newFileName, sizeof(newFileName), filePath.stem().string().c_str());
                //    }
                //    else
                //    {
                //        // Todo: Handle if it wasn't created
                //    }1
                //}

                if (ImGui::MenuItem("Create Script"))
                {
                    createMenuItems[0].action();
                    closeContextMenu();
                }

                if (ImGui::BeginMenu("Visual Scripting"))
                {
                    if (ImGui::MenuItem("Create EventSheet"))
                    {
                        createMenuItems[1].action();
                        closeContextMenu();
                    }

                    ImGui::EndMenu();
                }
                for (int i = 2; i < createMenuItems.size(); i++)
                {
                    if (ImGui::MenuItem(createMenuItems[i].name))
                    {
                        createMenuItems[i].action();
                        closeContextMenu();
                    }
                }

                ImGui::Separator();
                ImGui::BeginDisabled();
                ImGui::MenuItem("Paste", "CTRL+V");
                ImGui::EndDisabled();
                ImGui::Separator();

                if (ImGui::MenuItem("Open In File Explorer"))
                {
                    std::filesystem::path openPath = fileExplorerPath;

                    if (!explorerContextMenuFile.empty()) // If hovering a folder (or file when using search) open that directory
                    {
                        if (std::filesystem::is_directory(explorerContextMenuFile))
                            openPath = explorerContextMenuFile;
                        else
                            openPath = explorerContextMenuFile.parent_path();
                    }

                    Utilities::OpenPathInExplorer(openPath);
                    closeContextMenu();
                }

                if (!explorerContextMenuFile.empty())
                {
                    ImGui::Separator();
                    if (ImGui::MenuItem("Rename", "F2"))
                    {
                        renamingFile = explorerContextMenuFile;
                        strcpy_s(newFileName, sizeof(newFileName), explorerContextMenuFile.stem().string().c_str());
                        closeContextMenu();
                    }
                    if (ImGui::MenuItem("Copy", "CTRL+C"))
                        closeContextMenu();
                    if (ImGui::MenuItem("Duplicate", "CTRL+D"))
                    {
                        std::string newFilename;
                        int counter = 1;
                        do {
                            newFilename = explorerContextMenuFile.stem().string() + " copy" + (counter > 1 ? std::to_string(counter) : "") + explorerContextMenuFile.extension().string();
                            counter++;
                        } while (std::filesystem::exists(explorerContextMenuFile.parent_path() / newFilename));
                        std::filesystem::copy_file(explorerContextMenuFile, explorerContextMenuFile.parent_path() / newFilename, std::filesystem::copy_options::overwrite_existing);
                        closeContextMenu();
                    }
                    if (ImGui::MenuItem("Delete", "DEL"))
                    {
                        std::filesystem::remove_all(explorerContextMenuFile);
                        closeContextMenu();
                    }
                }

                ImGui::EndPopup();
            }
            else
                closeContextMenu();
        }

        // Drag and drop files
        if (ImGui::IsWindowHovered() && RaylibWrapper::IsFileDropped())
        {
            // Todo: Consider adding a progress bar, especially if there is a lot of files and directories, so then the program doesn't freeze and confuse the user
            RaylibWrapper::FilePathList droppedFiles = RaylibWrapper::LoadDroppedFiles();
            for (int i = 0; i < (int)droppedFiles.count; i++)
                Utilities::ImportFile(droppedFiles.paths[i], fileExplorerPath / std::filesystem::path(droppedFiles.paths[i]).filename());
                // Todo: Popup a window with the error. ImportFile() returns a bool, but this won't work if a file within a folder fails. Maybe have it return a vector of strings holding the errors? Or handle this within the ImportFile()
            UnloadDroppedFiles(droppedFiles);
        }
    }
    ImGui::End();
}

void Editor::RenderContentBrowserTreeNode(std::filesystem::path path, bool openOnDefault)
{
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) // This is important as it prevents a crash if the file is deleted from an external application like File Explorer
        return;

    bool hasChildren = false;
    for (const auto& entry : std::filesystem::directory_iterator(path, ec))
    {
        if (ec)
            break;

        if (entry.is_directory())
        {
            hasChildren = true;
            break;
        }
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
    flags |= hasChildren ? ImGuiTreeNodeFlags_OpenOnArrow : ImGuiTreeNodeFlags_Leaf;
    flags |= (path == fileExplorerPath) ? ImGuiTreeNodeFlags_Selected : 0;
    flags |= openOnDefault ? ImGuiTreeNodeFlags_DefaultOpen : 0;

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);

    if (ImGui::TreeNodeEx((ICON_FA_FOLDER_OPEN + std::string(" ") + path.stem().string() + "##" + path.string()).c_str(), flags))
    {
        if (!ImGui::IsItemToggledOpen())
        {
            if (ImGui::IsItemClicked())
                fileExplorerPath = path;

            if (hasChildren)
            {
                for (const auto& entry : std::filesystem::directory_iterator(path, ec))
                {
                    if (ec)
                        break;

                    if (entry.is_directory())
                        RenderContentBrowserTreeNode(entry, false);
                }
            }
        }

        ImGui::TreePop();
    }
}

std::string RenderFileSelector(int id, std::string type, std::string selectedPath, std::vector<std::string> extensions, bool hideNoneOption, ImVec2 position)
{
    static bool open = true;
    static char searchBuffer[256];
    // Using oldId to identify if the same thing is being set as previously so it will know if it needs to reset some values
    static int oldId = -9999;
    if (id != oldId)
    {
        open = true;
        memset(searchBuffer, 0, sizeof(searchBuffer));
        ImGui::SetNextWindowScroll({0,0});
    }

    std::string selectedFile = "";
    ImGui::SetNextWindowSize(ImVec2(200, 250));
    ImGui::SetNextWindowPos(position);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.18f, 0.18f, 0.18f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.12f, 0.12f, 0.12f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.12f, 0.12f, 0.12f, 1.00f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

    ImGui::SetNextWindowClass(&EditorWindow::defaultWindowClass);
    ImGui::Begin((" Select " + type).c_str(), &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);

    ImGui::SetCursorPos({ 10, 20 });
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 23);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::InputTextWithHint("##FileSearch", "Search...", searchBuffer, sizeof(searchBuffer));
    ImGui::PopStyleVar();

    ImGui::SetCursorPos({0, 45});
    ImGui::BeginTable("##FileSelectorTable", 1);

    // Todo: It may not be best to iterate the files each frame. Maybe store them in a static and only update them when the window is opened
    // Todo: Order the files in alphabetical order
    // Todo: Make a gap between the bottom of the table and bottom of the window since the nodes make it so the bottom left of the window isn't rounded

    ImGuiTreeNodeFlags flags;

    // "None" node
    if (!hideNoneOption)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_FramePadding;
        if (!selectedPath.empty())
            flags |= ImGuiTreeNodeFlags_Selected;
        if (ImGui::TreeNodeEx("None", flags))
        {
            if (ImGui::IsItemClicked())
                selectedFile = "None";
            ImGui::TreePop();
        }
    }

    std::string search = searchBuffer;
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);

    if (type == "Sprite" || type == "Image")
    {
        // Square
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_FramePadding;
        if (selectedPath == "Square")
            flags |= ImGuiTreeNodeFlags_Selected;

        if (ImGui::TreeNodeEx("Square", flags))
        {
            if (ImGui::IsItemClicked())
            {
                oldId = -9999;
                memset(searchBuffer, 0, sizeof(searchBuffer));
                ImGui::SetScrollY(0.0f);
                selectedFile = "Square";
            }
            ImGui::TreePop();
        }

        // Circle
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_FramePadding;
        if (selectedPath == "Circle")
            flags |= ImGuiTreeNodeFlags_Selected;

        if (ImGui::TreeNodeEx("Circle", flags))
        {
            if (ImGui::IsItemClicked())
            {
                oldId = -9999;
                memset(searchBuffer, 0, sizeof(searchBuffer));
                ImGui::SetScrollY(0.0f);
                selectedFile = "Circle";
            }
            ImGui::TreePop();
        }
    }

    try
    {
        for (const auto& file : std::filesystem::recursive_directory_iterator(ProjectManager::projectData.path / "Assets"))
        {
            if (!std::filesystem::is_regular_file(file) || std::find(extensions.begin(), extensions.end(), file.path().extension().string()) == extensions.end())
                continue;

            // This converts the search and file name to lower case so its case insensitive
            std::string fileName = (extensions.size() > 1) ? file.path().filename().string() : file.path().stem().string();
            std::string lowerFileName = fileName;

            std::transform(lowerFileName.begin(), lowerFileName.end(), lowerFileName.begin(), ::tolower);

            if (searchBuffer[0] != '\0' && lowerFileName.find(search) == std::string::npos)
                continue;

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            // Using ImGuiTreeNodeFlags_Framed flag adds a frame and aligns it to the left, but then selected flag doesn't work. I would need to set the node color if I want to make it so they have a frame or aligned to the left
            flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_FramePadding;
            // Checks to see if the file path contains the selectedPath and if it does, then select it
            if (!selectedPath.empty() && file.path().string().find(selectedPath) != std::string::npos)
                flags |= ImGuiTreeNodeFlags_Selected;

            if (ImGui::TreeNodeEx((fileName).c_str(), flags))
            {
                if (ImGui::IsItemClicked())
                {
                    oldId = -9999;
                    memset(searchBuffer, 0, sizeof(searchBuffer));
                    ImGui::SetScrollY(0.0f);
                    selectedFile = std::filesystem::relative(file.path(), ProjectManager::projectData.path / "Assets").string();
                }
                ImGui::TreePop();
            }
        }
    }
    catch (const std::exception& ex)
    {
        ConsoleLogger::ErrorLog("Failed to iterate project files to find " + type + " files", true);
        memset(searchBuffer, 0, sizeof(searchBuffer));
        selectedFile = "NULL";
        oldId = -9999;
        ImGui::SetScrollY(0.0f);
    }

    ImGui::EndTable();
    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(3);

    if (!open)
    {
        oldId = -9999;
        open = true;
        selectedFile = "NULL";
        memset(searchBuffer, 0, sizeof(searchBuffer));
        // Setting the scroll here does not work.
        ImGui::SetScrollY(0.0f);
    }

    for (char& c : selectedFile) // Reformatted the path for unix
    {
        if (c == '\\')
            c = '/';
    }

    oldId = id;
    return selectedFile;
}

void Editor::RenderAnimationGraph()
{
    if (!animationGraphWinOpen) return;
    //ImGui::SetNextWindowSize(ImVec2(180, 180));
    //ImGui::SetNextWindowPos(ImVec2((RaylibWrapper::GetScreenWidth() - 180) / 2, (RaylibWrapper::GetScreenHeight() - 180) / 2));

    animationGraphHovered = true;
    static nlohmann::json* selectedNode = nullptr;

    // Todo: Check model to see if it has new animations (or if animations were moved/renamed)

    ImGui::SetNextWindowClass(&EditorWindow::defaultWindowClass);
    if (ImGui::Begin((ICON_FA_PERSON_RUNNING + std::string(" Animation Graph")).c_str(), &animationGraphWinOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        bool update = false;
        ImNodes::BeginNodeEditor();
        if (animationGraphData.is_null())
        {
            // Todo: Fade background and make it so user can't move grid
            ImGui::PushFont(FontManager::GetFont("Familiar-Pro-Bold", 25, false));
            ImVec2 textSize = ImGui::CalcTextSize("No Animation Graph selected. Select or create one in the Content Browser.");
            ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - textSize.x) * 0.5f, (ImGui::GetWindowHeight() - textSize.y) * 0.5f));
            ImGui::Text("No Animation Graph selected. Select or create one in the Content Browser.");
            ImGui::PopFont();
            ImNodes::EndNodeEditor();
            ImGui::End();
            selectedNode = nullptr;
            return;
        }

        // Todo: Add "Any State" node
        // Todo: Ability to create multiple "Any State" nodes to cleanup the graph. Make it so ALL any state nodes can be removed including the last one
        // Todo: Save user position in grid to load it. (It doesn't seem to be implemented into ImNodes)
        // Todo: Make all nodes fixed sizes
        // Todo: Add zooming
        // Todo: Support external animations (from other model files).
        // Todo: If a model is reimported, make sure to check the animations and make sure the animations index values are correct with the actual index
        // Todo: Make sure it doesn't update when dragging a node, only once its dropped
        // Todo: Use node titles to let the user know if theres any warnings/errors
        // Todo: If the user selects any default nodes like Start and switches to a new graph while its still selected, it will move the position of the node in the new graph to where it was in the old graph

        for (auto& node : animationGraphData["nodes"])
        {
            int id = node["id"];
            std::string name = node["name"];
            if (id == -5) // Start node
            {
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImNodes::IsNodeSelected(-5))
                {
                    node["x"] = ImNodes::GetNodeGridSpacePos(id).x;
                    node["y"] = ImNodes::GetNodeGridSpacePos(id).y;
                    update = true;
                }
                if (!ImNodes::IsNodeSelected(id) && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    ImNodes::SetNodeGridSpacePos(-5, ImVec2(node["x"], node["y"]));
                ImNodes::BeginNode(-5);
                ImNodes::BeginOutputAttribute(-5);
                ImGui::Text("Start");
                ImNodes::EndOutputAttribute();
                ImNodes::EndNode();
            }
            else
            {
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImNodes::IsNodeSelected(id))
                {
                    node["x"] = ImNodes::GetNodeGridSpacePos(id).x;
                    node["y"] = ImNodes::GetNodeGridSpacePos(id).y;
                    update = true;
                }
                if (!ImNodes::IsNodeSelected(id) && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    ImNodes::SetNodeGridSpacePos(id, ImVec2(node["x"], node["y"]));
                ImNodes::BeginNode(id);
                ImNodes::BeginInputAttribute(abs(node["id"].get<int>())); // Using the positive ID for input ID
                ImNodes::EndInputAttribute();
                ImNodes::BeginOutputAttribute(node["id"].get<int>() * -1); // Using the negative ID for output ID
                ImGui::Text(node["name"].get<std::string>().c_str());
                ImNodes::EndOutputAttribute();
                ImNodes::EndNode();
            }
        }

        for (int i = 0; i < animationGraphData["links"].size(); ++i)
            ImNodes::Link(i, animationGraphData["links"][i][0], animationGraphData["links"][i][1]);

        static int selectNode = 0;
        int previousSelectNode = selectNode;
        static int openAddSpriteWin = -1; // -2 = Don't open, -1 = open for a new sprite, any else opens to change the sprite at the index (openAddSpriteWin holds the index)
        if (selectedNode == nullptr)
            openAddSpriteWin = -2;

        if (!animationGraphData.is_null())
        {
            if (selectedNode == nullptr || (*selectedNode)["id"] == -5) // Checks if selectedNode is null, or if the selected node is the Start node
            {
                ImGui::SetCursorPos({ ImGui::GetWindowWidth() - 130, 10 });
                if (ImGui::Button("Create Animation", { 120, 30 }))
                {
                    // Todo: Place the node in the center of the current position rather than this hardcoded solution

                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<int> distribution(999999, 99999999);

                    int id = 0;
                    while (id == 0)
                    {
                        id = distribution(gen);
                        for (auto& node : animationGraphData["nodes"])
                        {
                            if (id == node["id"])
                            {
                                id = 0;
                                break;
                            }
                        }
                    }

                    nlohmann::json node = {
                        {"id", id},
                        {"name", "New Animation"}, // Todo: It may be best to add numbers at the end of the name if there are others with this name
                        {"index", 0}, // Todo: Not sure why node's have the index element, see if they are needed
                        {"x", 350},
                        {"y", 350},
                        {"loop", true},
                        {"speed", 1.0f},
                        {"sprites", {}}
                    };
                    animationGraphData["nodes"].push_back(node);
                    
                    selectNode = id;
                    update = true;
                }
            }
            else
            {
                ImGui::SetCursorPos({ImGui::GetWindowWidth() - 160, 10});
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.13f, 0.14f, 0.15f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

                float height = 90;
                if (selectedNode->contains("sprites"))
                {
                    height = 170 + (*selectedNode)["sprites"].size() * 100;
                    if (height > ImGui::GetWindowHeight() - 20)
                        height = ImGui::GetWindowHeight() - 20;
                }

                ImGui::BeginChild("##Animation Graph Properties", {150, height });

                char nameBuffer[256];
                strcpy_s(nameBuffer, (*selectedNode)["name"].get<std::string>().c_str());
                ImGui::SetCursorPos({10, 10});
                ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 20);
                if (ImGui::InputText("##AnimationName", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    std::string temp = nameBuffer;
                    temp.erase(std::remove_if(temp.begin(), temp.end(), ::isspace), temp.end());
                    if (!temp.empty())
                    {
                        // Todo: Consider checking if an animation with the name already exists and if it does then add a number to the end of it
                        (*selectedNode)["name"] = nameBuffer;
                        update = true;
                    }
                }

                ImGui::SetCursorPos({ 10, 40 });
                ImGui::Text("Loop");
                bool loop = (*selectedNode)["loop"].get<bool>();
                ImGui::SetCursorPos({ 60, 40 });
                if (ImGui::Checkbox("##AnimationLoop", &loop))
                {
                    (*selectedNode)["loop"] = loop;
                    update = true;
                }

                ImGui::SetCursorPos({ 10, 70 });
                ImGui::Text("Speed");
                float speed = (*selectedNode)["speed"].get<float>();
                ImGui::SetNextItemWidth(40);
                ImGui::SetCursorPos({ 60, 70 });
                if (ImGui::InputFloat("##AnimationSpeed", &speed, 0, 0, "%.05g"))
                {
                    (*selectedNode)["speed"] = speed;
                    update = true;
                }
                if (selectedNode->contains("sprites"))
                {
                    // Todo: Add support to re-order sprites.
                    // Todo: Preview button and preview at the bottom. When preview button is clicked, it will expand the window even more to fit it if needed.
                    // When hovering over sprite add X button top right to remove the sprite
                    // Todo: Add sprite drag and drop support

                    ImGui::GetWindowDrawList()->AddLine(ImVec2(ImGui::GetWindowPos().x + 10, ImGui::GetWindowPos().y + 100), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth() - 10, ImGui::GetWindowPos().y + 100), IM_COL32(25, 25, 25, 255), 2);

                    ImGui::SetCursorPos({ 50, 105 });
                    ImGui::PushFont(FontManager::GetFont("Familiar-Pro-Bold", 15, false));
                    ImGui::Text("Sprites");
                    ImGui::PopFont();

                    ImGui::SetCursorPos({ 10, 123 });
                    ImGui::BeginChild("#AnimationSprites", { ImGui::GetWindowWidth() - 10, ImGui::GetWindowHeight() - 133 });

                    static ImVec2 spriteWinPos;

                    float nextY = 10;
                    int index = 0;
                    for (auto& sprite : (*selectedNode)["sprites"])
                    {
                        // Todo: Check if path exists, if it doesn't then try to find the file by filename + extension in other folders. If it can't find it, then remove it or set it to a NULL/"File Not Found" image
                        tempTextures.push_back(new RaylibWrapper::Texture2D(RaylibWrapper::LoadTexture((ProjectManager::projectData.path / "Assets" / sprite.get<std::string>()).string().c_str())));
                        float aspectRatio = (float)tempTextures.back()->width / (float)tempTextures.back()->height;
                        ImVec2 imageSize = ImVec2(0, 0);
                        imageSize.x = aspectRatio > 1.0f ? 90 : 90 * aspectRatio;
                        imageSize.y = aspectRatio > 1.0f ? 90 / aspectRatio : 90;
                        int buttonSize = 90 + ImGui::GetStyle().FramePadding.x * 2;

                        ImGui::SetCursorPos({17, nextY});
                        ImVec2 cursorPos = ImGui::GetCursorPos();

                        ImVec2 tempPos = ImGui::GetCursorScreenPos();
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.17f, 0.17f, 0.17f, 1.0f));
                        if (ImGui::Button(("##" + std::to_string(index)).c_str(), ImVec2(buttonSize, buttonSize)))
                        {
                            openAddSpriteWin = index;
                            spriteWinPos = tempPos;
                        }
                        ImGui::PopStyleColor();

                        //if (ImGui::IsItemHovered())
                        //{
                        //    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        //    {

                        //    }
                        //}

                        ImGui::SetCursorPos(ImVec2(cursorPos.x + (buttonSize - imageSize.x) / 2, cursorPos.y + (buttonSize - imageSize.y) / 2)); // Sets the position to the button position and centers it
                        RaylibWrapper::rlImGuiImageSize(tempTextures.back(), imageSize.x, imageSize.y);
                        nextY += 100;
                        index++;
                    }

                    // Todo: Iterate the sprites, create a background of a lighter grey, then add the sprite inside of it (look at content browser for aspect ratio)

                    if (ImGui::GetCursorPosY() > ImGui::GetWindowHeight())
                        ImGui::SetCursorPos({ 15, nextY });
                    else
                        ImGui::SetCursorPos({ 15, ImGui::GetWindowHeight() - 20});
                    ImVec2 tempPos = ImGui::GetCursorScreenPos();
                    if (ImGui::Button("Add Sprite", { 100, 20 }))
                    {
                        openAddSpriteWin = -1;
                        spriteWinPos = tempPos;
                    }

                    ImGui::EndChild();

                    // Todo: Check if the position + height > window and if it is, then move it up
                    // Todo: Consider using a random number as the RenderFileSelector ID so it fixes the issue with the scroll not always being at the top.
                    // Checks if the Sprite File Selector window is open
                    if (openAddSpriteWin != -2)
                    {
                        std::string selectedFile = RenderFileSelector((*selectedNode)["id"] + openAddSpriteWin, "Sprite", "", { ".png", ".jpeg", ".jpg" }, false, { spriteWinPos.x - 235, spriteWinPos.y - 40});
                        // If NULL was returned, or if None was returned and its adding a new sprite, then close the window with no changes
                        if (selectedFile == "NULL" || (openAddSpriteWin == -1 && selectedFile == "None"))
                            openAddSpriteWin = -2;
                        else if (!selectedFile.empty())
                        {
                            // If its replacing a sprite instead of adding a new one, either remove it, or replace it
                            if (openAddSpriteWin != -1)
                            {
                                if (selectedFile == "None")
                                    (*selectedNode)["sprites"].erase((*selectedNode)["sprites"].begin() + openAddSpriteWin);
                                else
                                    (*selectedNode)["sprites"][openAddSpriteWin] = selectedFile;
                            }
                            else
                                (*selectedNode)["sprites"].push_back(selectedFile);
                            openAddSpriteWin = -2;
                        }
                    }
                }

                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor();
            }
        }

        ImNodes::EndNodeEditor();

        // This uses previousSelectNode to make it wait until the next iteration to select the node incase the node isn't created yet (Like when clicking "Create Animation")
        if (selectNode != 0 && previousSelectNode == selectNode) // Todo: It may be good to check if the node actually exists since this waits an iteration and by that time something could happen and delete the node. Maybe make SelectNode() return true or false since it checks if the node exists. if it doesnt exist, set selectNode to 0
        {
            ImNodes::SelectNode(selectNode);
            selectNode = 0;
        }

        if (ImNodes::NumSelectedNodes() > 0)
        {
            std::vector<int> selectedNodes;
            selectedNodes.resize(ImNodes::NumSelectedNodes());
            ImNodes::GetSelectedNodes(selectedNodes.data());
            if (selectedNode == nullptr || (*selectedNode)["id"] != selectedNodes.back())
            {
                openAddSpriteWin = -2;
                for (auto& node : animationGraphData["nodes"])
                {
                    if (node["id"] == selectedNodes.back())
                    {
                        selectedNode = &node;
                        break;
                    }
                }
            }
        }
        else
            selectedNode = nullptr;

        int start_attr, end_attr;
        if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
        {
            update = true;
            animationGraphData["links"].push_back({ start_attr, end_attr });
        }

        if (update)
        {
            std::ofstream file(animationGraphData["path"].get<std::filesystem::path>());
            if (file.is_open())
            {
                file << std::setw(4) << animationGraphData << std::endl;
                file.close();
                update = false;
            }
        }

        // Todo: When saving, pop up saying the graph file was deleted or moved. Asked to recreate it or delete it
    }
    ImGui::End();
}

void Editor::RenderProjectSettings()
{
    if (!projectSettingsWinOpen)
        return;

    static char nameBuffer[128], descriptionBuffer[128], versionBuffer[128], companyBuffer[128], copyrightBuffer[128];

    if (resetProjectSettings)
    {
        float xSize = RaylibWrapper::GetScreenWidth() * 0.50;
        float ySize = RaylibWrapper::GetScreenHeight() * 0.75;
        ImGui::SetNextWindowSize(ImVec2(xSize, ySize));
        ImGui::SetNextWindowPos(ImVec2((RaylibWrapper::GetScreenWidth() - xSize) / 2, (RaylibWrapper::GetScreenHeight() - ySize) / 2));

        strcpy_s(nameBuffer, ProjectManager::projectData.name.c_str());
        strcpy_s(descriptionBuffer, ProjectManager::projectData.description.c_str());
        strcpy_s(versionBuffer, ProjectManager::projectData.version.c_str());
        strcpy_s(companyBuffer, ProjectManager::projectData.company.c_str());
        strcpy_s(copyrightBuffer, ProjectManager::projectData.copyright.c_str());

        resetProjectSettings = false;
    }

    ImGui::SetNextWindowClass(&EditorWindow::defaultWindowClass);
    if (ImGui::Begin((ICON_FA_GEARS + std::string(" Project Properties")).c_str(), &projectSettingsWinOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        auto RenderSection = [](const char* title, std::function<void()> content) {
            ImGui::PushFont(FontManager::GetFont("Familiar-Pro-Bold", 25, false));
            ImGui::Text("%s", title);
            ImGui::PopFont();
            content();
            ImGui::NewLine();
        };

        auto RenderInputField = [](const char* label, char* buffer, size_t bufferSize, std::function<void(const char*)> onEdit, float width = 150.0f) {
            ImGui::Text("%s", label);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(width);
            if (ImGui::InputText(("##" + std::string(label)).c_str(), buffer, bufferSize))
            {
                onEdit(buffer);
                ProjectManager::SaveProjectData(ProjectManager::projectData);
            }
        };

        auto renderFileSelector = [](const char* label, std::string type, std::vector<std::string> extensions, bool hideNoneOption, std::string& value) {
            static std::string renderSelector = "";

            ImGui::Text("%s", label);
            ImGui::SameLine();
            ImVec2 pos = ImGui::GetCursorPos();
            if (ImGui::Button((value + "##" + std::string(label)).c_str(), { ImGui::CalcTextSize(value.c_str()).x + 20, 20 }))
            {
                renderSelector = label;

                if (ProjectManager::projectData.defaultScenePath != "None" && !std::filesystem::exists(ProjectManager::projectData.path / "Assets" / ProjectManager::projectData.defaultScenePath))
                {
                    ProjectManager::projectData.defaultScenePath = "None";
                    ProjectManager::SaveProjectData(ProjectManager::projectData);
                }
            }

            if (renderSelector != label)
                return;

            int id = 1;
            for (char c : std::string(label))
                id *= 31 + c;
            std::string selectedFile = RenderFileSelector(id, type, value, extensions, hideNoneOption, { pos.x + 235, pos.y - 40 });
            // If NULL was returned, or if None was returned, then close the window with no changes
            if (selectedFile == "NULL" || selectedFile == "None")
                renderSelector = "";
            else if (!selectedFile.empty())
            {
                value = selectedFile;
                ProjectManager::SaveProjectData(ProjectManager::projectData);
                renderSelector = "";
            }
        };

        auto RenderComboBox = [](const char* label, int& currentItem, const char* items[], int itemCount, float width = 100.0f) {
            ImGui::Text("%s", label);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(width);
            if (ImGui::Combo(("##" + std::string(label)).c_str(), &currentItem, items, itemCount))
                ProjectManager::SaveProjectData(ProjectManager::projectData);
        };

        auto RenderInputInt2 = [](const char* label, Vector2& value, float width = 100.0f) {
            ImGui::Text("%s", label);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(width);
            int intValue[2] = { static_cast<int>(value.x), static_cast<int>(value.y) };
            if (ImGui::InputInt2(("##" + std::string(label)).c_str(), intValue))
            {
                value = { static_cast<float>(intValue[0]), static_cast<float>(intValue[1]) };
                ProjectManager::SaveProjectData(ProjectManager::projectData);
            }
        };

        auto RenderInputInt = [](const char* label, int& value, float width = 25.0f) {
            ImGui::Text("%s", label);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(width);
            if (ImGui::InputInt(("##" + std::string(label)).c_str(), &value, 0, 0))
                ProjectManager::SaveProjectData(ProjectManager::projectData);
        };

        auto RenderCheckBox = [](const char* label, bool& value) {
            ImGui::Text("%s", label);
            ImGui::SameLine();
            if (ImGui::Checkbox(("##" + std::string(label)).c_str(), &value))
                ProjectManager::SaveProjectData(ProjectManager::projectData);
        };

        auto RenderInputFloat2 = [](const char* label, Vector2& value, float width = 70.0f) {
            ImGui::Text("%s", label);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(width);
            if (ImGui::InputFloat2(("##" + std::string(label)).c_str(), &value.x, "%.05g"))
                ProjectManager::SaveProjectData(ProjectManager::projectData);
        };

        RenderSection("General", [&]() {
            RenderInputField("Name", nameBuffer, sizeof(nameBuffer), [](const char* value) { ProjectManager::projectData.name = value; });
            RenderInputField("Description", descriptionBuffer, sizeof(descriptionBuffer), [](const char* value) { ProjectManager::projectData.description = value; });
            RenderInputField("Company", companyBuffer, sizeof(companyBuffer), [](const char* value) { ProjectManager::projectData.company = value; }, 100.0f);
            RenderInputField("Version", versionBuffer, sizeof(versionBuffer), [](const char* value) {
                for (char c : std::string(value))
                {
                    if (!isdigit(c) && c != '.')
                    {
                        value = ProjectManager::projectData.version.c_str();
                        break;
                    }
                }
                ProjectManager::projectData.version = value;
                }, 50.0f);
            RenderInputField("Copyright", copyrightBuffer, sizeof(copyrightBuffer), [](const char* value) { ProjectManager::projectData.copyright = value; }, 300.0f);
            renderFileSelector("Default Scene", "Scene", { ".scene" }, true, ProjectManager::projectData.defaultScenePath);
            renderFileSelector("Icon Path", "Icon", { ".ico" }, true, ProjectManager::projectData.iconPath);
            });

        RenderSection("Window Settings", [&]() {
            const char* displayModes[] = { "Borderless", "Fullscreen", "Windowed" };
            RenderComboBox("Display Mode", ProjectManager::projectData.displayMode, displayModes, IM_ARRAYSIZE(displayModes));
            RenderInputInt2("Resolution", ProjectManager::projectData.windowResolution);
            RenderInputInt2("Minimum Resolution", ProjectManager::projectData.minimumResolution);
            RenderCheckBox("Resizable", ProjectManager::projectData.resizableWindow);
            RenderInputInt("Max FPS", ProjectManager::projectData.maxFPS);
            RenderCheckBox("Run In Background", ProjectManager::projectData.runInBackground);
            RenderCheckBox("VSync", ProjectManager::projectData.vsync);
            });

        RenderSection("Graphics", [&]() {
            RenderCheckBox("Anti-aliasing", ProjectManager::projectData.antialiasing);
            RenderCheckBox("High DPI", ProjectManager::projectData.highDPI);
            });

        RenderSection("Physics", [&]() {
            RenderInputFloat2("Timestep", ProjectManager::projectData.physicsTimeStep);
            RenderInputInt("Velocity Iterations", ProjectManager::projectData.velocityIterations);
            RenderInputInt("Position Iterations", ProjectManager::projectData.positionIterations);
            });
    }
    ImGui::End();
}

void Editor::RenderScriptCreateWin()
{
    if (!scriptCreateWinOpen)
        return;

    ImGui::SetNextWindowSize(ImVec2(180, 180));
    ImGui::SetNextWindowPos(ImVec2((RaylibWrapper::GetScreenWidth() - 180) / 2, (RaylibWrapper::GetScreenHeight() - 180) / 2));

    ImGui::SetNextWindowClass(&EditorWindow::defaultWindowClass);
    if (ImGui::Begin("Create Script", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::Text("Script Name");
        static char behaviourName[36] = "";
        ImGui::SetNextItemWidth(163);
        ImGui::InputTextWithHint("##Name", "Script Name...", behaviourName, IM_ARRAYSIZE(behaviourName));
        ImGui::NewLine();
        if (ImGui::Button("Cancel", ImVec2(163, 0)))
        {
            memset(behaviourName, 0, sizeof(behaviourName));
            scriptCreateWinOpen = false;
        }

        ImGui::NewLine();

        std::string name = behaviourName;
        std::string newName = name;
        newName.erase(std::remove_if(newName.begin(), newName.end(), [](unsigned char c) {
            return std::isspace(c);
            }), newName.end());

        bool canCreate = !name.empty() && !std::filesystem::exists(fileExplorerPath / (name + ".h")) && !std::filesystem::exists(fileExplorerPath / (name + ".cpp"));

        if (newName == "" || std::filesystem::exists(fileExplorerPath / (name + ".h")) || std::filesystem::exists(fileExplorerPath / (name + ".cpp"))) canCreate = false; // Todo: Show text saying it already exists (assuming its not empty)

        if (!canCreate) ImGui::BeginDisabled();
        if (ImGui::Button("Create", ImVec2(163, 0)))
        {
            scriptCreateWinOpen = false;

            if (std::filesystem::exists(fileExplorerPath / (name + ".h")) || std::filesystem::exists(fileExplorerPath / (name + ".cpp")))
            {
                // Todo: Popup saying script with this name already exists in this location
                ImGui::End();
                return;
            }
            std::filesystem::path miscPath = std::filesystem::path(__FILE__).parent_path() / "resources" / "misc";
            std::string presetType = ProjectManager::projectData.is3D ? "3D" : "2D";
            std::filesystem::copy_file(miscPath / ("ScriptPreset " + presetType + ".h"), fileExplorerPath / (name + ".h"));
            std::filesystem::copy_file(miscPath / ("ScriptPreset " + presetType + ".cpp"), fileExplorerPath / (name + ".cpp"));

            // Replace "ScriptPreset" with the script name in the .cpp and .h
            auto ReplaceInFile = [&name](const std::filesystem::path& filePath) {
                std::ifstream input(filePath);
                if (!input.is_open())
                {
                    ConsoleLogger::WarningLog("Failed to modify " + name + filePath.extension().string() + ". You will need to manually change \"ScriptPreset\" to " + name + " within the script.", false);
                    ImGui::End();
                    return;
                }
                std::stringstream buffer;
                buffer << input.rdbuf();
                std::string content = buffer.str();
                input.close();

                size_t pos = 0;
                while ((pos = content.find("ScriptPreset", pos)) != std::string::npos)
                {
                    content.replace(pos, 12, name);
                    pos += name.length();
                }

                std::ofstream output(filePath);
                if (!output.is_open())
                {
                    ConsoleLogger::WarningLog("Failed to modify " + name + filePath.extension().string() + ". You will need to manually change \"ScriptPreset\" to " + name + " within the script.", false);
                    ImGui::End();
                    return;
                }
                output << content;
                output.close();
            };

            ReplaceInFile(fileExplorerPath / (name + ".h"));
            ReplaceInFile(fileExplorerPath / (name + ".cpp"));
        }
        if (!canCreate) ImGui::EndDisabled();
    }
    ImGui::End();
}

void Editor::RenderComponentsWin()
{
    if (!componentsWindowOpen || !std::holds_alternative<GameObject*>(objectInProperties))
        return;

    if (resetComponentsWin)
    {
        ImGui::SetNextWindowSize(ImVec2(300, 400));
        ImGui::SetNextWindowPos(ImVec2((RaylibWrapper::GetScreenWidth() - 300) / 2, (RaylibWrapper::GetScreenHeight() - 400) / 2));
        ImGui::SetNextWindowScroll({0,0});
        resetComponentsWin = false;
    }

    ImGui::SetNextWindowClass(&EditorWindow::defaultWindowClass);
    if (ImGui::Begin("Add Component", &componentsWindowOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoCollapse))
    {
        // Internal Components
        float buttonWidth = ImGui::GetWindowWidth() - 28;

        auto AddComponentInternalButton = [&](const char* componentName, auto&& AddComponentInternalFunc) {
            if (ImGui::Button(componentName, ImVec2(buttonWidth, 0))) {
                AddComponentInternalFunc();
                componentsWindowOpen = false;
                resetComponentsWin = true;
            }
        };

        // Internal components
        AddComponentInternalButton("Camera", [&]() { std::get<GameObject*>(objectInProperties)->AddComponentInternal<CameraComponent>(); });
        AddComponentInternalButton("Light", [&]() { std::get<GameObject*>(objectInProperties)->AddComponentInternal<Lighting>(); });
        AddComponentInternalButton("Collider2D", [&]() { std::get<GameObject*>(objectInProperties)->AddComponentInternal<Collider2D>(); });
        AddComponentInternalButton("Rigidbody2D", [&]()
            {
                if (std::get<GameObject*>(objectInProperties)->GetComponent<Rigidbody2D>())
                {
                    // Todo: Add popup?
                    ConsoleLogger::ErrorLog("Failed to add Rigidbody2D to " + std::get<GameObject*>(objectInProperties)->GetName() + ". This game object already has a Rigidbody2D attached to it.");
                    return;
                }
                std::get<GameObject*>(objectInProperties)->AddComponentInternal<Rigidbody2D>();
            });
        AddComponentInternalButton("Collider3D", [&]() { std::get<GameObject*>(objectInProperties)->AddComponentInternal<Collider3D>(); });
        AddComponentInternalButton("Rigidbody3D", [&]()
            {
                if (std::get<GameObject*>(objectInProperties)->GetComponent<Rigidbody3D>())
                {
                    // Todo: Add popup?
                    ConsoleLogger::ErrorLog("Failed to add Rigidbody3D to " + std::get<GameObject*>(objectInProperties)->GetName() + ". This game object already has a Rigidbody3D attached to it.");
                    return;
                }
                std::get<GameObject*>(objectInProperties)->AddComponentInternal<Rigidbody3D>();
            });
        AddComponentInternalButton("Sprite Renderer", [&]() { std::get<GameObject*>(objectInProperties)->AddComponentInternal<SpriteRenderer>(); });
        AddComponentInternalButton("Mesh Renderer", [&]() { std::get<GameObject*>(objectInProperties)->AddComponentInternal<MeshRenderer>(); });
        AddComponentInternalButton("Tilemap Renderer", [&]() { std::get<GameObject*>(objectInProperties)->AddComponentInternal<TilemapRenderer>(); });
        AddComponentInternalButton("Animation Player", [&]() { std::get<GameObject*>(objectInProperties)->AddComponentInternal<AnimationPlayer>(); });
        AddComponentInternalButton("Audio Player", [&]() { std::get<GameObject*>(objectInProperties)->AddComponentInternal<AudioPlayer>(); });
        AddComponentInternalButton("Label", [&]() { std::get<GameObject*>(objectInProperties)->AddComponentInternal<Label>(); });
        AddComponentInternalButton("Image", [&]() { std::get<GameObject*>(objectInProperties)->AddComponentInternal<Image>(); });
        AddComponentInternalButton("Button", [&]() { std::get<GameObject*>(objectInProperties)->AddComponentInternal<Button>(); });

        ImGui::Separator();

        // External coponents
        for (const auto& file : std::filesystem::recursive_directory_iterator(ProjectManager::projectData.path / "Assets"))
        {
            if (!std::filesystem::is_regular_file(file) || file.path().extension() != ".h")
                continue;

            std::filesystem::path relativePath = std::filesystem::relative(file.path(), ProjectManager::projectData.path / "Assets");
            std::filesystem::path cppPath = relativePath;
            cppPath.replace_extension(".cpp");

            AddComponentInternalButton(file.path().stem().string().c_str(), [&]() {
                ScriptComponent* scriptComponent = &std::get<GameObject*>(objectInProperties)->AddComponentInternal<ScriptComponent>();
                scriptComponent->SetHeaderPath(relativePath.string());
                scriptComponent->SetCppPath(cppPath);
                scriptComponent->SetName(relativePath.stem().string());
                });
        }
    }
    ImGui::End();
}

void Editor::RenderCameraView()
{
    if (!cameraSelected)
    {
        // Checks if the Game view window is focused
        ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_GAMEPAD + std::string(" Game")).c_str());

        if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
        {
            if (window->TabId != window->DockNode->TabBar->SelectedTabId)
                return;
        }
        else
            return;
    }

    if (resetCameraView)
    {
        cameraRenderTexture = RaylibWrapper::LoadRenderTexture(RaylibWrapper::GetScreenWidth(), RaylibWrapper::GetScreenHeight());
        ImGui::SetNextWindowSize(ImVec2(200, 112));
        ImGui::SetNextWindowPos(viewportOpened ? ImVec2(viewportPosition.z - 200, viewportPosition.w - 112) : ImVec2((RaylibWrapper::GetScreenWidth() - 200) / 2, (RaylibWrapper::GetScreenHeight() - 112) / 2));
        resetCameraView = false;
    }

    bool selectedGameObjectHasCamera = true;
    CameraComponent* camera = nullptr;

    if (selectedObject)
        camera = selectedObject->GetComponent<CameraComponent>();

    if (!camera)
    {
        selectedGameObjectHasCamera = false;
        camera = CameraComponent::main;

        if (!camera)
            return;
    }

    RaylibWrapper::BeginTextureMode(cameraRenderTexture);
    RaylibWrapper::ClearBackground(ProjectManager::projectData.is3D ? RaylibWrapper::Color{ 135, 206, 235, 255 } : RaylibWrapper::Color{128, 128, 128, 255});

    camera->raylibCamera.BeginMode3D();

    deltaTime = RaylibWrapper::GetFrameTime();

    for (GameObject* gameObject : SceneManager::GetActiveScene()->GetGameObjects()) // Todo: This is different from the main editor camera. Check if this needs to be updated.
    {
        if (!gameObject->IsActive()) continue;
        for (Component* component : gameObject->GetComponents())
        {
            if (component->IsActive() && component->runInEditor)
                component->Update();
        }
    }

    for (RenderableTexture* texture : RenderableTexture::textures) // Renders Sprites and Tilemaps
        if (texture)
            texture->Render();

    RaylibWrapper::EndMode3D();
    RaylibWrapper::EndTextureMode();

    if (selectedGameObjectHasCamera)
    {
        ImGui::SetNextWindowClass(&EditorWindow::defaultWindowClass);
        if (ImGui::Begin("Camera View", &componentsWindowOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
            rlImGuiImageRenderTextureFit(&cameraRenderTexture, true);

        ImGui::End();
    }
}

int Editor::RenderColorPicker(std::string name, ImVec2 position, ImVec4& selectedColor, ImVec4& previousColor) // Todo: Does this really need to be in the Editor.h? Also Add Undo/Redo
{
    // return 0 = do nothing, 1 = update color, 2 = update color and close
    static bool open = true;
    int action = 0;

    ImGui::SetNextWindowSize(ImVec2(250, 250));
    ImGui::SetNextWindowPos(position);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.18f, 0.18f, 0.18f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.12f, 0.12f, 0.12f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.12f, 0.12f, 0.12f, 1.00f));

    ImGui::SetNextWindowClass(&EditorWindow::defaultWindowClass);
    ImGui::Begin(name.c_str(), &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);
    if (ImGui::ColorPicker4(("##ColorPicker" + name).c_str(), (float*)&selectedColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoLabel, (float*)&previousColor))
        action = 1;

    ImGui::End();
    ImGui::PopStyleColor(3);

    if (!open)
    {
        open = true;
        action = 2;
    }

    return action;
}

void Editor::RenderProperties()
{
    static bool show = true;

    ImGui::SetNextWindowClass(&EditorWindow::defaultWindowClass);
    if (ImGui::Begin((ICON_FA_GEARS + std::string(" Properties")).c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::BeginGroup();
        //if (std::holds_alternative<GameObject*>(objectInProperties) && std::find_if(SceneManager::GetActiveScene()->GetGameObjects().begin(), SceneManager::GetActiveScene()->GetGameObjects().end(), [&](const auto& obj) { return obj == (*propertiesGameObject); }) != SceneManager::GetActiveScene()->GetGameObjects().end())
        if (auto* propertiesGameObject = std::get_if<GameObject*>(&objectInProperties))
        {
            // Active checkbox
            // Doing it this way so it calls the SetActive() function
            bool active = (*propertiesGameObject)->active;
            ImGui::Checkbox("##Active" , &active);
            if (active != (*propertiesGameObject)->active)
                (*propertiesGameObject)->SetActive(active);

            // Name
            ImGui::SameLine();
            char nameBuffer[256] = {};
            strcpy_s(nameBuffer, (*propertiesGameObject)->GetName().c_str());
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 45);
            if (ImGui::InputText("##ObjectNameText", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                (*propertiesGameObject)->SetName(std::string(nameBuffer));

            // Layers
            ImGui::NewLine();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 10);
            int width = (ImGui::GetWindowWidth() - 125) / 2;
            ImGui::Text("Layers:");
            ImGui::SameLine();
            static const char* layers[] = { "Default", "UI" };
            static int selectedLayer = 0;
            ImGui::SetNextItemWidth(width);
            if (ImGui::BeginCombo("##Layers", layers[selectedLayer]))
            {
                for (int i = 0; i < IM_ARRAYSIZE(layers); i++)
                {
                    const bool isSelected = (selectedLayer == i);
                    if (ImGui::Selectable(layers[i], isSelected))
                        selectedLayer = i;

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            // Tags
            ImGui::SameLine();
            ImGui::Text("Tag:");
            ImGui::SameLine();
            static const char* tags[] = { "None" };
            static int selectedTag = 0;
            ImGui::SetNextItemWidth(width);
            if (ImGui::BeginCombo("##Tags", tags[selectedTag]))
            {
                for (int i = 0; i < IM_ARRAYSIZE(tags); i++)
                {
                    const bool isSelected = (selectedTag == i);
                    if (ImGui::Selectable(tags[i], isSelected))
                        selectedTag = i;

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            auto RenderNumProperty = [&](const std::string& type, const std::string& label, float& value, float width, std::function<void(float)> update, bool isFloat = true)
            {
                ImGui::Text(label.c_str());
                ImGui::SameLine();
                ImGui::SetNextItemWidth(width);

                if (isFloat)
                {
                    if (ImGui::InputFloat((std::string("##") + label + type).c_str(), &value, 0, 0, "%.10g"))
                        update(value);
                }
                else
                {
                    int intValue = value;
                    if (ImGui::InputInt((std::string("##") + label + type).c_str(), &intValue, 0, 0))
                        update(intValue);
                }
            };

            // Position
            Vector3 localPosition = (*propertiesGameObject)->transform.GetLocalPosition();
            width = (ImGui::GetWindowWidth() - 165) / 3;

            ImGui::NewLine();
            ImGui::Text("Position:   ");

            ImGui::SameLine();
            RenderNumProperty("Position", "X", localPosition.x, width, [&](float x) {
                (*propertiesGameObject)->transform.SetLocalPosition({ x, localPosition.y, localPosition.z });
                });
            ImGui::SameLine();
            RenderNumProperty("Position", "Y", localPosition.y, width, [&](float y) {
                (*propertiesGameObject)->transform.SetLocalPosition({ localPosition.x, y, localPosition.z });
                });
            ImGui::SameLine();
            RenderNumProperty("Position", "Z", localPosition.z, width, [&](float z) {
                (*propertiesGameObject)->transform.SetLocalPosition({ localPosition.x, localPosition.y, z });
                });

            // Scale
            Vector3 localScale = (*propertiesGameObject)->transform.GetLocalScale();

            ImGui::NewLine();
            ImGui::Text("Scale:        ");

            ImGui::SameLine();
            RenderNumProperty("Scale", "X", localScale.x, width, [&](float x) {
                (*propertiesGameObject)->transform.SetLocalScale({ x, localScale.y, localScale.z });
                });
            ImGui::SameLine();
            RenderNumProperty("Scale", "Y", localScale.y, width, [&](float y) {
                (*propertiesGameObject)->transform.SetLocalScale({ localScale.x, y, localScale.z });
                });
            ImGui::SameLine();
            RenderNumProperty("Scale", "Z", localScale.z, width, [&](float z) {
                (*propertiesGameObject)->transform.SetLocalScale({ localScale.x, localScale.y, z });
                });

            // Rotation
            Vector3 localRotation = (*propertiesGameObject)->transform.GetLocalRotationEuler();

            ImGui::NewLine();
            ImGui::Text("Rotation:   ");

            ImGui::SameLine();
            RenderNumProperty("Rotation", "X", localRotation.x, width, [&](float x) {
                (*propertiesGameObject)->transform.SetLocalRotationEuler({ x, localRotation.y, localRotation.z });
                }, false);
            ImGui::SameLine();
            RenderNumProperty("Rotation", "Y", localRotation.y, width, [&](float y) {
                (*propertiesGameObject)->transform.SetLocalRotationEuler({ localRotation.x, y, localRotation.z });
                }, false);
            ImGui::SameLine();
            RenderNumProperty("Rotation", "Z", localRotation.z, width, [&](float z) {
                (*propertiesGameObject)->transform.SetLocalRotationEuler({ localRotation.x, localRotation.y, z });
                }, false);

            ImGui::NewLine();
             
            //Components
            static nlohmann::json* colorPopupOpened = nullptr;
            static ImVec4 selectedColor;
            static ImVec4 previousColor;
            static ImVec2 popupPosition;
            int componentsNum = 0;
            static Component* componentInContextMenu = nullptr;
            float buttonWidth = ImGui::GetWindowWidth() - 15;

            for (Component* component : (*propertiesGameObject)->GetComponents())
            {
                componentsNum++;
                //ImGui::Separator();
                int deleteYPos = ImGui::GetCursorPosY();
                std::string name = (typeid(*component) == typeid(ScriptComponent)) ?
                    ICON_FA_FILE_CODE + std::string(" ") + dynamic_cast<ScriptComponent*>(component)->GetName() :
                    component->iconUnicode + " " + component->name;

                ImGui::SetNextItemWidth(buttonWidth);
                if (ImGui::CollapsingHeader((name + "##" + std::to_string(component->id)).c_str())) // Todo: Figure out why the width isn't being set on this
                {
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    {
                        componentInContextMenu = component;
                        ImGui::OpenPopup("ComponentsContextMenu");
                    }

                    int windowHeight = (component->exposedVariables != nullptr && !component->exposedVariables[1].empty()) ?component->exposedVariables[1].size() * 33 : 1;

                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.22f, 0.22f, 0.22f, 1.00f));
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
                    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 5));

                    // Todo: Is creating a child window the best thing to do here?

                    ImGui::BeginChild((std::to_string(ImGui::GetCursorPosY())).c_str(), ImVec2(buttonWidth, windowHeight), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding);
                    
                    // Check to see if exposed variables updated
                    if (typeid(*component) == typeid(ScriptComponent) && (component->exposedVariables == nullptr || oneSecondDelay <= 0)) // Todo: Do not check them for updates. Also check if its empty // Todo: Threading
                    {
                        if (component->exposedVariables == nullptr || component->exposedVariables[0].empty()) // Todo: Check every ~1 second if the file is modified and if should get exposed variables again. DONOT change values, only remove removed variables
                            component->exposedVariables = Utilities::GetExposedVariables(ProjectManager::projectData.path / "Assets" / dynamic_cast<ScriptComponent*>(component)->GetHeaderPath()); // Todo: Add Threading
                        else
                        {
                            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(std::filesystem::last_write_time(ProjectManager::projectData.path / "Assets" / dynamic_cast<ScriptComponent*>(component)->GetHeaderPath()) - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
                            std::time_t tt = std::chrono::system_clock::to_time_t(sctp);

                            if (component->exposedVariables[0].get<int>() != tt)
                            {
                                nlohmann::json updatedJson = Utilities::GetExposedVariables(ProjectManager::projectData.path / "Assets" / dynamic_cast<ScriptComponent*>(component)->GetHeaderPath()); // Todo: Add Threading

                                //for (auto updatedElement = updatedJson[1].begin(); updatedElement != updatedJson[1].end(); ++updatedElement)
                                //{
                                //    for (auto oldElement = component->exposedVariables[1].begin(); oldElement != component->exposedVariables[1].end(); ++oldElement)
                                //    {
                                //        if ((*updatedElement)[0] == (*oldElement)[0] && (*updatedElement)[1] == (*oldElement)[1])
                                //        {
                                //            (*updatedElement)[2] = (*oldElement)[2];
                                //            break;
                                //        }
                                //    }
                                //}

                                for (auto& updatedElement : updatedJson[1])
                                {
                                    auto it = std::find_if(component->exposedVariables[1].begin(), component->exposedVariables[1].end(),
                                        [&](const auto& oldElement) { return updatedElement[0] == oldElement[0] && updatedElement[1] == oldElement[1]; });
                                    if (it != component->exposedVariables[1].end())
                                        updatedElement[2] = (*it)[2];
                                }

                                component->exposedVariables = updatedJson;
                            }
                        }
                    }
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.16f, 0.17f, 0.18f, 1.00f));
                    if (component->exposedVariables != nullptr) // Todo: Add Threading
                    {
                        // Todo: Add vectors, list, and array
                        //ConsoleLogger::InfoLog(component->exposedVariables.dump(4));
                        for (auto it = component->exposedVariables[1].begin(); it != component->exposedVariables[1].end(); ++it)
                        {
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
                            std::string name = (*it)[3];
                            //ImGui::NewLine();
                            ImGui::Text(name.c_str());
                            if ((*it)[0] == "bool")
                            {
                                ImGui::SameLine();
                                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
                                bool value = (*it)[2].get<bool>();
                                if (ImGui::Checkbox(("##" + name).c_str(), &value))
                                    (*it)[2] = value;
                            }
                            else if ((*it)[0] == "string" || (*it)[0] == "char")
                            {
                                ImGui::SameLine();
                                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
                                ImGui::SetNextItemWidth(120);
                                std::string inputText = (*it)[2].get<std::string>();
                                char buffer[256];
                                strcpy_s(buffer, sizeof(buffer), inputText.c_str());
                                ImGui::InputText(("##" + name).c_str(), buffer, sizeof(buffer));
                                (*it)[2] = buffer;
                            }
                            else if ((*it)[0] == "float")
                            {
                                ImGui::SameLine();
                                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
                                ImGui::SetNextItemWidth(60); 
                                float value = (*it)[2].get<float>();
                                ImGui::InputFloat(("##" + name).c_str(), &value, 0, 0, "%.10g");
                                (*it)[2] = value;
                            }
                            else if ((*it)[0] == "int")
                            {
                                ImGui::SameLine();
                                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
                                ImGui::SetNextItemWidth(60);
                                int value = (*it)[2].get<int>();
                                ImGui::InputInt(("##" + name).c_str(), &value, 0, 0);
                                (*it)[2] = value;
                            }
                            else if ((*it)[0] == "Color")
                            {
                                ImGui::SameLine();
                                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
                                // Todo: Add black outline
                                if (ImGui::ColorButton(("##" + name).c_str(), ImVec4((*it)[2][0].get<float>() / 255, (*it)[2][1].get<float>() / 255, (*it)[2][2].get<float>() / 255, (*it)[2][3].get<float>() / 255), 0, ImVec2(20, 20)))
                                {
                                    if (colorPopupOpened != nullptr && colorPopupOpened[3] == name)
                                        colorPopupOpened = nullptr;
                                    else
                                    {
                                        colorPopupOpened = &(*it);
                                        previousColor = ImVec4((*it)[2][0].get<float>() / 255, (*it)[2][1].get<float>() / 255, (*it)[2][2].get<float>() / 255, (*it)[2][3].get<float>() / 255);
                                        selectedColor = previousColor;
                                    }
                                }
                                if (colorPopupOpened != nullptr && (*colorPopupOpened)[3].get<std::string>() == name)
                                    popupPosition = ImVec2(ImGui::GetWindowPos().x - 250, ImGui::GetCursorPosY() + ImGui::GetWindowPos().y - 20);
                            }
                            else if ((*it)[0] == "Vector2")
                            {
                                width = (ImGui::GetWindowWidth() - (65 + ImGui::CalcTextSize(name.c_str()).x)) / 2;
                                //ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
                                ImGui::SameLine();
                                ImGui::Text("X");
                                ImGui::SameLine();
                                float x = (*it)[2][0].get<float>();
                                ImGui::SetNextItemWidth(width);
                                ImGui::InputFloat(("##X" + name).c_str(), &x, 0, 0, "%.10g");
                                (*it)[2][0] = x;

                                ImGui::SameLine();
                                ImGui::Text("Y");
                                ImGui::SameLine();
                                float y = (*it)[2][1].get<float>();
                                ImGui::SetNextItemWidth(width);
                                ImGui::InputFloat(("##Y" + name).c_str(), &y, 0, 0, "%.10g");
                                (*it)[2][1] = y;
                            }
                            else if ((*it)[0] == "Vector3")
                            {
                                width = (ImGui::GetWindowWidth() - (98 + ImGui::CalcTextSize(name.c_str()).x)) / 3;
                                //ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
                                ImGui::SameLine();
                                ImGui::Text("X");
                                ImGui::SameLine();
                                float x = (*it)[2][0].get<float>();
                                ImGui::SetNextItemWidth(width);
                                ImGui::InputFloat(("##X" + name).c_str(), &x, 0, 0, "%.10g");
                                (*it)[2][0] = x;

                                ImGui::SameLine();
                                ImGui::Text("Y");
                                ImGui::SameLine();
                                float y = (*it)[2][1].get<float>();
                                ImGui::SetNextItemWidth(width);
                                ImGui::InputFloat(("##Y" + name).c_str(), &y, 0, 0, "%.10g");
                                (*it)[2][1] = y;

                                ImGui::SameLine();
                                ImGui::Text("Z");
                                ImGui::SameLine();
                                float z = (*it)[2][2].get<float>();
                                ImGui::SetNextItemWidth(width);
                                ImGui::InputFloat(("##Z" + name).c_str(), &z, 0, 0, "%.10g");
                                (*it)[2][2] = z;
                            }
                            else if ((*it).size() > 4 && (*it)[4].contains("Extensions"))
                            {
                                static bool openSelector = false;
                                // Todo: If the user deselects or closes the component, it will hide the menu, but once it is reopened, or another one with Extensions, it will open the menu. Maybe right here right after the bool, run RenderFIleSelector() with a special parameter to check if the old ID != new ID and if it doesnt equal, then return "NULL" and it will know it shouldn't be open
                                ImGui::SameLine();
                                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.17f, 0.18f, 1.00f));
                                std::string selectedFile = "None Selected";
                                if (!(*it)[2].is_null() && (*it)[2].get<std::string>() != "nullptr")
                                {
                                    if ((*it)[4]["Extensions"].size() > 1)
                                        selectedFile = std::filesystem::path((*it)[2].get<std::string>()).filename().string();
                                    else
                                        selectedFile = std::filesystem::path((*it)[2].get<std::string>()).stem().string();
                                }
                                ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
                                if (ImGui::Button(selectedFile.c_str(), { ImGui::GetWindowWidth() - 130, 20 }))
                                    openSelector = true;

                                // If right clicked, then go to it in the content browser
                                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
                                    std::filesystem::exists(ProjectManager::projectData.path / "Assets" / std::filesystem::path((*it)[2].get<std::string>())))
                                {
                                    // Set content browser path to the file
                                    fileExplorerPath = ProjectManager::projectData.path / "Assets" / std::filesystem::path((*it)[2].get<std::string>()).parent_path();

                                    // Focus Content Browser
                                    ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_FOLDER_OPEN + std::string(" Content Browser")).c_str());
                                    if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                                        window->DockNode->TabBar->NextSelectedTabId = window->TabId;

                                    focusContentBrowserFile = (ProjectManager::projectData.path / "Assets" / std::filesystem::path((*it)[2].get<std::string>())).string();
                                }

                                // Checks to see if the user is hovering over the button while dragging a file with a file type that can be used for the variable
                                else if (ImGui::IsMouseHoveringRect(cursorScreenPos, { cursorScreenPos.x + ImGui::GetWindowWidth() - 130, cursorScreenPos.y + 20 }) && dragData.first != None && dragData.second.find("Path") != dragData.second.end()) // Using this since ImGui::IsItemHovered() won't work while dragging.
                                {
                                    std::vector<std::string> extensions;
                                    for (const auto& extension : (*it)[4]["Extensions"])
                                        extensions.push_back(extension);
                                    if (std::find(extensions.begin(), extensions.end(), std::any_cast<std::filesystem::path>(dragData.second["Path"]).extension().string()) != extensions.end())
                                    {
                                        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                                            (*it)[2] = std::filesystem::relative(std::any_cast<std::filesystem::path>(dragData.second["Path"]), ProjectManager::projectData.path / "Assets").string();
                                        dragData.second["HoveringValidElement"] = true; // This lets the code in the Content Browser know its hovering something it can be placed in, so it can change the cursor icon
                                    }
                                }

                                ImGui::PopStyleColor();

                                if (openSelector)
                                {
                                    // Adds the extensions to the extensions vector so it can be passed into RenderFileSelector()
                                    std::vector<std::string> extensions;
                                    for (const auto& extension : (*it)[4]["Extensions"])
                                        extensions.push_back(extension);

                                    std::string currentlySelected = ((*it)[2] == "nullptr") ? "" : (*it)[2];
                                    std::string selectedFile = RenderFileSelector(componentsNum, name, currentlySelected, extensions, false, {ImGui::GetCursorScreenPos().x - 100, ImGui::GetCursorScreenPos().y - 40});
                                    if (selectedFile == "NULL")
                                        openSelector = false;
                                    else if (!selectedFile.empty())
                                    {
                                        if (selectedFile == "None")
                                            (*it)[2] = "nullptr";
                                        else
                                            (*it)[2] = selectedFile;
                                        openSelector = false;
                                    }
                                }
                            }
                            else if ((*it).size() > 4) // This is probably not a good solution for checking if its an enum or not. Maybe give the array a key
                            {
                                // Todo: The enum values won't be capitalized or have spaces, only the selected enum value will be capitalized with spaces
                                ImGui::SameLine();
                                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
                                std::string value = (*it)[2].get<std::string>();
                                ImGui::SetNextItemWidth(ImGui::CalcTextSize(value.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f + 20);
                                if (ImGui::BeginCombo(("##" + name).c_str(), value.c_str())) // Todo: Resize Combo Box to fit selected value.
                                {
                                    for (const std::string& enumValue : (*it)[4])
                                    {
                                        if (ImGui::Selectable(enumValue.c_str(), value == enumValue))
                                            (*it)[2] = enumValue;
                                    }
                                    ImGui::EndCombo();
                                }
                            }
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
                            //ConsoleLogger::ErrorLog("Found Exposed Variable: " + it->dump());
                        }
                    }
                    ImGui::PopStyleColor();
                    ImGui::EndChild();
                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor();
                }
                else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                {
                    componentInContextMenu = component;
                    ImGui::OpenPopup("ComponentsContextMenu");
                }
                int oldYPos = ImGui::GetCursorPosY();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                //ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
                ImGui::SameLine();
                ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 50, deleteYPos));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.16f, 0.17f, 0.18f, 1.00f));
                ImGui::Checkbox("##ComponentActive", &component->active); // Todo: This isn't detecting clicks
                ImGui::PopStyleColor();

                ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 25, deleteYPos));
                if (ImGui::Button(("X##" + std::to_string(componentsNum)).c_str())) // Todo: This isn't detecting clicks
                    (*propertiesGameObject)->RemoveComponent(component);

                ImGui::PopStyleColor(2);
                ImGui::SetCursorPosY(oldYPos);
            }

            ImGui::NewLine();
            ImGui::SetCursorPos(ImVec2(25, ImGui::GetCursorPosY() - 15));
            if (ImGui::Button("Add Component", ImVec2(ImGui::GetWindowWidth() - 50, 0)))
                componentsWindowOpen = true;

            if (colorPopupOpened != nullptr)
            {
                int action = RenderColorPicker("Color", popupPosition, selectedColor, previousColor);
                if (action != 0)
                {
                    (*colorPopupOpened)[2][0] = selectedColor.x * 255;
                    (*colorPopupOpened)[2][1] = selectedColor.y * 255;
                    (*colorPopupOpened)[2][2] = selectedColor.z * 255;
                    (*colorPopupOpened)[2][3] = selectedColor.w * 255;
                    if (action == 2)
                        colorPopupOpened = nullptr;
                }
            }

            if (ImGui::BeginPopupContextItem("ComponentsContextMenu"))
            {
                if (ImGui::MenuItem("Delete"))
                {
                    (*propertiesGameObject)->RemoveComponent(componentInContextMenu);
                    componentInContextMenu = nullptr;
                }

                ImGui::EndPopup();
            }

            // Dropped script files
            // I need to use this solution since IsWindoweHovered() doesn't work well when the mouse is dragging. It's not a perfect solution as it detects it being hovered when there is a window above it, but it works
            ImVec2 mousePos = ImGui::GetMousePos();
            if ((mousePos.x >= ImGui::GetWindowPos().x && mousePos.x <= ImGui::GetWindowPos().x + ImGui::GetWindowWidth() && mousePos.y >= ImGui::GetWindowPos().y && mousePos.y <= ImGui::GetWindowPos().y + ImGui::GetWindowHeight()) && dragData.first != None && (std::any_cast<std::filesystem::path>(dragData.second["Path"]).extension().string() == ".h" || std::any_cast<std::filesystem::path>(dragData.second["Path"]).extension().string() == ".cpp"))
            {
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                {
                    std::filesystem::path path = std::filesystem::relative(std::any_cast<std::filesystem::path>(dragData.second["Path"]), ProjectManager::projectData.path / "Assets").string();
                    int createComponent = 2; // 0 = can't find header, 1 = can't find cpp, 2 = create
                    std::filesystem::path cppPath = path;

                    if (path.extension() == ".h")
                    {
                        cppPath.replace_extension(".cpp");
                        if (!std::filesystem::exists(ProjectManager::projectData.path / "Assets" / cppPath))
                            createComponent = 1;
                    }
                    else
                    {
                        path.replace_extension(".h");
                        if (!std::filesystem::exists(ProjectManager::projectData.path / "Assets" / path))
                            createComponent = 0;
                    }

                    if (createComponent == 2)
                    {
                        ScriptComponent* scriptComponent = &(*propertiesGameObject)->AddComponentInternal<ScriptComponent>();
                        scriptComponent->SetHeaderPath(path.string());
                        scriptComponent->SetCppPath(cppPath);
                        scriptComponent->SetName(path.stem().string());
                    }
                    else if (createComponent == 1) // Todo: send popup warning
                        ConsoleLogger::WarningLog("Failed to add the component " + path.stem().string() + " to the game object. " + cppPath.filename().string() + " is missing. Make sure it is in the same folder as " + path.filename().string());
                    else if (createComponent == 0) // Todo: send popup warning
                        ConsoleLogger::WarningLog("Failed to add the component " + path.stem().string() + " to the game object. " + path.filename().string() + " is missing. Make sure it is in the same folder as " + cppPath.filename().string());
                }
                dragData.second["HoveringValidElement"] = true; // This lets the code in the Content Browser know its hovering something it can be placed in, so it can change the cursor icon
            }
        }
        // Data File
        else if (std::holds_alternative<DataFile>(objectInProperties))
        {
            DataFile& dataFile = std::get<DataFile>(objectInProperties);
            if (!std::filesystem::exists(dataFile.path))
            {
                objectInProperties = std::monostate{};
                ImGui::EndGroup();
                ImGui::End();
                return;
            }

            int offset = 0;
            switch (dataFile.type)
            {
            case DataFileTypes::Sound:
                ImGui::SetCursorPos({ 10, 30 });
                RaylibWrapper::rlImGuiImageSizeV(IconManager::imageTextures["SoundIcon"], { 25, 25 });
                offset = 35;
                break;
            case DataFileTypes::Tilemap:
                ImGui::SetCursorPos({ 10, 30 });
                RaylibWrapper::rlImGuiImageSizeV(IconManager::imageTextures["LDtkIcon"], { 25, 25 });
                offset = 35;
                break;
            }

            std::string fileName = std::filesystem::path(std::get<DataFile>(objectInProperties).path).stem().string();
            ImGui::PushFont(FontManager::GetFont("Familiar-Pro-Bold", 20, false));
            ImGui::SetCursorPos({ (ImGui::GetWindowWidth() - offset - ImGui::CalcTextSize(fileName.c_str()).x) / 2 + offset, 30});
            ImGui::Text(fileName.c_str());
            ImGui::PopFont();

            ImGui::SetCursorPosY(60);
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 2.5f);

            ImGui::SetCursorPos({15, 70});
            bool updateJson = false;
            for (auto& [key, value] : std::get<DataFile>(objectInProperties).json["public"].items())
            {
                // Adds spaces before all capital letters, and makes the first letter a capital
                std::string newKey;
                for (char c : key) {
                    if (std::isupper(c)) {
                        newKey += ' ';
                    }
                    newKey += c;
                }
                newKey[0] = std::toupper(newKey[0]);

                if (value.is_boolean())
                {
                    ImGui::Text(newKey.c_str());
                    ImGui::SameLine();
                    bool tempValue = value.get<bool>();
                    if (ImGui::Checkbox(("##" + key).c_str(), &tempValue))
                    {
                        value = tempValue;
                        updateJson = true;
                    }
                }
                else if (value.is_number_integer())
                {
                    ImGui::Text(newKey.c_str());
                    ImGui::SameLine();

                    int inputWidth = ImGui::CalcTextSize(std::to_string(value.get<int>()).c_str()).x + ImGui::CalcTextSize("1").x + 10;
                    if (inputWidth < 60)
                        inputWidth = 60;
                    else if (inputWidth > (ImGui::GetWindowWidth() - ImGui::CalcTextSize(newKey.c_str()).x - 45))
                        inputWidth = ImGui::GetWindowWidth() - ImGui::CalcTextSize(newKey.c_str()).x - 45;

                    ImGui::SetNextItemWidth(inputWidth);
                    ImGui::SetCursorPos({ ImGui::CalcTextSize(newKey.c_str()).x + 30, ImGui::GetCursorPosY() - 3 });
                    int input = value.get<int>();
                    if (ImGui::InputInt(("##" + key).c_str(), &input, 0, 0))
                    {
                        if (input > INT_MAX)
                            input = INT_MAX;
                        else if (input < -INT_MIN)
                            input = INT_MIN;
                        value = input;
                        updateJson = true;
                    }

                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
                }
                else if (value.is_number_float())
                {
                    ImGui::Text(newKey.c_str());
                    ImGui::SameLine();

                    // Todo: Doesn't work the best when there are a lot of decimals
                    std::string stringValue = std::to_string(value.get<float>());
                    stringValue.erase(stringValue.find_last_not_of('0') + 1, std::string::npos); // Removes trailing 0's

                    int inputWidth = ImGui::CalcTextSize(stringValue.c_str()).x + ImGui::CalcTextSize("1").x + 10;
                    if (inputWidth < 60)
                        inputWidth = 60;
                    else if (inputWidth > (ImGui::GetWindowWidth() - ImGui::CalcTextSize(newKey.c_str()).x - 45))
                        inputWidth = ImGui::GetWindowWidth() - ImGui::CalcTextSize(newKey.c_str()).x - 45;

                    ImGui::SetNextItemWidth(inputWidth);
                    ImGui::SetCursorPos({ ImGui::CalcTextSize(newKey.c_str()).x + 30, ImGui::GetCursorPosY() - 3 });
                    float input = value.get<float>();
                    if (ImGui::InputFloat(("##" + key).c_str(), &input, 0, 0, "%.10g"))
                    {
                        value = input;
                        updateJson = true;
                    }

                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
                }
                else if (value.is_string())
                {
                    ImGui::Text(newKey.c_str());
                    ImGui::SameLine();

                    int inputWidth = ImGui::CalcTextSize(value.get<std::string>().c_str()).x + ImGui::CalcTextSize("a").x + 10;
                    if (inputWidth < 130)
                        inputWidth = 130;
                    else if (inputWidth > (ImGui::GetWindowWidth() - ImGui::CalcTextSize(newKey.c_str()).x - 45))
                        inputWidth = ImGui::GetWindowWidth() - ImGui::CalcTextSize(newKey.c_str()).x - 45;

                    ImGui::SetNextItemWidth(inputWidth);
                    ImGui::SetCursorPos({ ImGui::CalcTextSize(newKey.c_str()).x + 30, ImGui::GetCursorPosY() - 3 });
                    std::string inputText = value.get<std::string>();
                    char buffer[256];
                    strcpy_s(buffer, sizeof(buffer), inputText.c_str());
                    if (ImGui::InputText(("##" + key).c_str(), buffer, sizeof(buffer)))
                    {
                        value = buffer;
                        updateJson = true;
                    }

                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
                }
                else if (value.is_array())
                {
                    // Todo: Add support for drag and drop
                    if (value[0] == "file") // File Selector
                    {
                        // Value[0] = "file", Value[1] = extensions array, Value[2] = value
                        ImGui::Text(newKey.c_str());
                        ImGui::SameLine();

                        static std::string fileSelectorOpenVariable = "";
                        ImGui::SetCursorPos({ ImGui::CalcTextSize(newKey.c_str()).x + 30, ImGui::GetCursorPosY() - 3 });
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.17f, 0.18f, 1.00f));
                        std::string selectedFile = "None Selected";
                        if (!value[2].is_null() && value[2].get<std::string>() != "nullptr" && !value[2].get<std::string>().empty())
                        {
                            if (value[1].size() > 1) // Checking if there is more than one extension that can be selected
                                selectedFile = std::filesystem::path(value[2].get<std::string>()).filename().string();
                            else
                                selectedFile = std::filesystem::path(value[2].get<std::string>()).stem().string();
                        }
                        ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
                        if (ImGui::Button((selectedFile + "##" + fileName + key).c_str(), {ImGui::GetWindowWidth() - 130, 20}))
                            fileSelectorOpenVariable = key;

                        ImGui::PopStyleColor();

                        if (!fileSelectorOpenVariable.empty() && fileSelectorOpenVariable == key)
                        {
                            std::string currentlySelected = (value[2] == "nullptr") ? "" : value[2];
                            selectedFile = RenderFileSelector(85623, key, currentlySelected, value[1], false, {ImGui::GetCursorScreenPos().x - 100, ImGui::GetCursorScreenPos().y - 40});
                            if (selectedFile == "NULL")
                                fileSelectorOpenVariable = "";
                            else if (!selectedFile.empty())
                            {
                                if (selectedFile == "None")
                                    value[2] = "nullptr";
                                else
                                    value[2] = selectedFile;
                                fileSelectorOpenVariable = "";
                                updateJson = true;
                            }
                        }
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
                    }
                    // TODO: Add vector3

                }
                else
                    continue;

                ImGui::SetCursorPos({ 15, ImGui::GetCursorPosY() + 15 });
            }

            if (updateJson)
            {
                try
               {
                    std::ofstream file(dataFile.path);
                    file << dataFile.json.dump(4);
                    file.close();
                }
                catch (const std::exception& e)
                {
                    if (ConsoleLogger::showDebugMessages)
                        ConsoleLogger::WarningLog("There was an error saving your changes to " + dataFile.path.substr(0, dataFile.path.size() - 5) + ".", false);
                    else
                        ConsoleLogger::WarningLog("There was an error saving your changes to " + dataFile.path + ". Error: " + e.what(), true);
                }
            }
        }
        ImGui::EndGroup();
    }
    ImGui::End();
}

void Editor::RenderConsole()
{
    static int numberOfLogs = 0;
    ImGui::SetNextWindowClass(&EditorWindow::defaultWindowClass);
    if (ImGui::Begin((ICON_FA_CODE + std::string(" Console")).c_str(), nullptr, ImGuiWindowFlags_NoCollapse))
    {
        for (int i = 0; i < ConsoleLogger::GetLogCount(); i++)
        {
            const ConsoleLogger::LogEntry& log = ConsoleLogger::GetLog(i);

            switch (log.type)
            {
                case ConsoleLogger::LogType::INFO:
                    ImGui::TextColored({ 0.8f, 0.8f, 0.8f, 1.0f }, "[INFO]");
                    break;
                case ConsoleLogger::LogType::WARNING:
                    ImGui::TextColored({ 1.0f, 1.0f, 0.60f, 1.0f }, "[WARNING]");
                    break;
                case ConsoleLogger::LogType::ERROR_:
                    ImGui::TextColored({ 1.0f, 0.4f, 0.4f, 1.0f }, "[ERROR]");
                    break;
            }
            ImGui::SameLine();
            ImGui::Text(log.message.c_str());
        }
        
        if (ConsoleLogger::GetLogCount() != numberOfLogs && ImGui::GetScrollMaxY() - ImGui::GetScrollY() < 50)
            ImGui::SetScrollHereY(1);

        ImGui::NewLine();
    }
    ImGui::End();

    numberOfLogs = ConsoleLogger::GetLogCount();
}

bool Editor::RenderHierarchyNode(GameObject* gameObject, bool normalColor, bool& childDoubleClicked)
{
    childDoubleClicked = false;

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, normalColor ? IM_COL32(36, 40, 43, 255) : IM_COL32(45, 48, 51, 255));

    //ImGui::PushStyleColor(ImGuiCol_Header, {64, 64, 64, 255});

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;

    if (selectedObject != nullptr && selectedObject->GetId() == gameObject->GetId())
        flags |= ImGuiTreeNodeFlags_Selected;

    flags |= gameObject->GetChildren().empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnArrow;

    //std::string icon = ICON_FA_CUBE;
    //if (gameObject->GetComponent<CameraComponent>()) // Todo: Not sure if this is a good idea to check if it has teh Camera component every frame.
    //    icon = ICON_FA_CAMERA;

    // This is coded a bit different than I normally would since TreeNodeEx() doesn't return true if the game object has children but its not expanded
    if (!gameObject->IsActive() || !gameObject->IsGlobalActive())
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.65f, 0.65f, 1));
    bool nodeOpen = ImGui::TreeNodeEx((ICON_FA_CUBE + std::string(" ") + gameObject->GetName() + "##" + std::to_string(gameObject->GetId())).c_str(), flags);
    if (!gameObject->IsActive() || !gameObject->IsGlobalActive())
        ImGui::PopStyleColor();

    bool currentNodeDoubleClicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

    if (ImGui::IsItemClicked())
    {
        hierarchyObjectClicked = true;
        //if (selectedObject != nullptr)
        //EventSystem::Invoke("ObjectDeselected", selectedObject);
        objectInProperties = selectedObject = gameObject;
        //EventSystem::Invoke("ObjectSelected", selectedObject);
        cameraSelected = selectedObject->GetComponent<CameraComponent>() != nullptr;
        if (!cameraSelected && cameraSelected)
        {
            cameraSelected = false;
            resetCameraView = true;
        }
    }
    else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        objectInHierarchyContextMenu = gameObject;

    if (nodeOpen)
    {
        for (GameObject* child : gameObject->GetChildren())
        {
            bool childClicked = false;
            normalColor = RenderHierarchyNode(child, !normalColor, childClicked);
            childDoubleClicked |= childClicked;
        }
        ImGui::TreePop();
    }

    if (currentNodeDoubleClicked && !childDoubleClicked)
    {
        // Todo: If 3D, rotate the camera and set proper distance depending on the mesh's size
        // Todo: For 2D and if its a sprite, set proper distance so entire sprite fits on screen
        // Todo: Lerp camera position to target
        const auto& pos = gameObject->transform.GetPosition();

        camera.position = { pos.x, pos.y, camera.position.z };
        camera.target = { pos.x, pos.y, camera.target.z };
    }

    childDoubleClicked |= currentNodeDoubleClicked;

    return normalColor; // Todo: If has child and child is open, return the lowest color rather than this node's color
}

void Editor::RenderHierarchy()
{
    hierarchyObjectClicked = false;
    ImGui::SetNextWindowClass(&EditorWindow::defaultWindowClass);
    ImGui::Begin((ICON_FA_SITEMAP + std::string(" Hierarchy")).c_str(), nullptr, ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginTable("HierarchyTable", 1))
    {
        bool hierarchyRowColor = true;

        for (GameObject* gameObject : SceneManager::GetActiveScene()->GetGameObjects())
            if (gameObject->GetParent() == nullptr)
            {
                bool childDoubleClicked = false;
                hierarchyRowColor = !RenderHierarchyNode(gameObject, hierarchyRowColor, childDoubleClicked);
            }

        hierarchyRowColor = false;
        ImGui::EndTable();
    }

    if (hierarchyContextMenuOpen || (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)))
    {
        hierarchyContextMenuOpen = true;
        if (ImGui::BeginPopupContextWindow("HierarchyContextMenu"))
        {
            ImGui::Separator();

            struct ObjectItem
            {
                std::string menuName;
                std::string name;
                int dimension; // 1 = both, 2 = 2D, 3 = 3D
                ModelType model;
            };

            struct GuiObjectItem
            {
                std::string menuName;
                std::string name;
            };

            static const std::vector<ObjectItem> menuObjects = {
                {"Create Empty", "GameObject", 1, Custom},
                {"Create Cube", "Cube", 3, Cube},
                {"Create Cylinder", "Cylinder", 3, Cylinder},
                {"Create Sphere", "Sphere", 3, Sphere},
                {"Create Plane", "Plane", 3, Plane},
                {"Create Cone", "Cone", 3, Cone},
                {"Create Light", "Light", 3, Custom},
                {"Create Square", "Square", 2, Custom},
                {"Create Circle", "Circle", 2, Custom},
                {"Create Tilemap", "Tilemap", 2, Custom},
                {"Create Camera", "Camera", 1, Custom}
            };

            static const std::vector<GuiObjectItem> menuGUIObjects = {
                {"Create Label", "Label"},
                {"Create Image", "Image"},
                {"Create Button", "Button"}
            };

            ObjectItem objectToCreate = {"", "", 0, Custom};
            
            for (const ObjectItem& item : menuObjects)
            {
                if ((!ProjectManager::projectData.is3D && item.dimension == 3) || (ProjectManager::projectData.is3D && item.dimension == 2))
                    continue;

                if (ImGui::MenuItem(item.menuName.c_str()))
                {
                    hierarchyContextMenuOpen = false;
                    objectToCreate = item;
                }
            }

            GuiObjectItem guiObjectToCreate = { "", "" };

            if (ImGui::BeginMenu("GUI"))
            {
                for (const GuiObjectItem& item : menuGUIObjects)
                {
                    if (ImGui::MenuItem(item.menuName.c_str()))
                    {
                        hierarchyContextMenuOpen = false;
                        guiObjectToCreate = item;
                    }
                }
                ImGui::EndMenu();
            }

            if (objectInHierarchyContextMenu != nullptr && ImGui::MenuItem("Delete"))
            {
                hierarchyObjectClicked = true;
                hierarchyContextMenuOpen = false;

                if (std::holds_alternative<GameObject*>(objectInProperties) && std::get<GameObject*>(objectInProperties)->GetId() == objectInHierarchyContextMenu->GetId())
                    objectInProperties = std::monostate{};

                if (selectedObject != nullptr && objectInHierarchyContextMenu->GetId() == selectedObject->GetId())
                {
                    //EventSystem::Invoke("ObjectDeselected", nullptr);
                    selectedObject = nullptr;
                }

                SceneManager::GetActiveScene()->RemoveGameObject(objectInHierarchyContextMenu);
                objectInHierarchyContextMenu = nullptr;
            }

            if (!objectToCreate.name.empty() || !guiObjectToCreate.name.empty())
            {
                hierarchyObjectClicked = true;

                GameObject* gameObject = SceneManager::GetActiveScene()->AddGameObject();
                gameObject->transform.SetPosition({ 0,0,0 });
                gameObject->transform.SetScale({ 1,1,1 });
                gameObject->transform.SetRotation(Quaternion::Identity());
                gameObject->SetName(objectToCreate.name + guiObjectToCreate.name);
                if (objectToCreate.name == "Camera")
                {
                    gameObject->AddComponentInternal<CameraComponent>();
                    if (!ProjectManager::projectData.is3D)
                    {
                        gameObject->transform.SetPosition({ 0,0,0 });
                        gameObject->transform.SetRotationEuler({ 180, 0, 0 });
                    }
                }
                else if (objectToCreate.name == "Light")
                    gameObject->AddComponentInternal<Lighting>();
                else if (objectToCreate.name == "Tilemap")
                {
                    gameObject->AddComponentInternal<TilemapRenderer>();
                }
                else if (objectToCreate.name != "GameObject" && guiObjectToCreate.name == "")
                {
                    if (ProjectManager::projectData.is3D)
                    {
                        MeshRenderer& meshRenderer = gameObject->AddComponentInternal<MeshRenderer>();
                        meshRenderer.SetModelPath(objectToCreate.name);
                        meshRenderer.SetModel(objectToCreate.model, objectToCreate.name, ShaderManager::LitStandard);

                        Collider3D& collider = gameObject->AddComponentInternal<Collider3D>();
                        if (objectToCreate.model == Cube)
                            collider.SetShapeInternal("Box");
                        else if (objectToCreate.model == Cylinder)
                        {
                            collider.SetShapeInternal(objectToCreate.name);
                            collider.SetOffset({0, 1, 0});
                        }
                        else if (objectToCreate.model == Cone)
                        {
                            collider.SetShapeInternal(objectToCreate.name);
                            collider.SetOffset({ 0, 0.5f , 0 });
                        }
                        else
                            collider.SetShapeInternal(objectToCreate.name);
                    }
                    else
                    {
                        SpriteRenderer& spriteRenderer = gameObject->AddComponentInternal<SpriteRenderer>();
                        spriteRenderer.SetSprite(new Sprite(objectToCreate.name));

                        //gameObject->AddComponentInternal<Collider2D>(); // Todo: Set size and type
                    }
                }
                else if (guiObjectToCreate.name == "Label")
                    gameObject->AddComponentInternal<Label>();
                else if (guiObjectToCreate.name == "Image")
                    gameObject->AddComponentInternal<Image>();
                else if (guiObjectToCreate.name == "Button")
                    gameObject->AddComponentInternal<Button>();

                //SceneManager::GetActiveScene()->AddGameObject(gameObject);

                for (Component* component : gameObject->GetComponents())
                    component->gameObject = SceneManager::GetActiveScene()->GetGameObjects().back();

                if (objectInHierarchyContextMenu != nullptr)
                {
                    gameObject->SetParent(objectInHierarchyContextMenu);
                    gameObject->transform.SetLocalPosition({ 0,0,0 });
                    gameObject->transform.SetLocalRotationEuler({ 0,0,0 });
                    gameObject->transform.SetLocalScale({ 1,1,1 });
                }

                //if (selectedObject != nullptr)
                    //EventSystem::Invoke("ObjectDeselected", selectedObject);
                selectedObject = gameObject;
                objectInProperties = gameObject;
                //EventSystem::Invoke("ObjectSelected", selectedObject);

                if (selectedObject->GetComponent<CameraComponent>() != nullptr)
                    cameraSelected = true;
                else if (cameraSelected)
                {
                    cameraSelected = false;
                    resetCameraView = true;
                }
            }

            ImGui::EndPopup();
        }
        if (hierarchyContextMenuOpen && !ImGui::IsPopupOpen("HierarchyContextMenu"))
        {
            objectInHierarchyContextMenu = nullptr;
            hierarchyContextMenuOpen = false;
        }
    }
    else if (!hierarchyObjectClicked && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
    {
        if (selectedObject)
        {
            //EventSystem::Invoke("ObjectDeselected", selectedObject);
            selectedObject = nullptr;
            cameraSelected = false;
            resetCameraView = true;
        }
        if (std::holds_alternative<GameObject*>(objectInProperties))
            objectInProperties = std::monostate{};
    }
    ImGui::End();
}

void Editor::RenderTopbar()
{
    int currentWidth = RaylibWrapper::GetScreenWidth();
    int currentHeight = RaylibWrapper::GetScreenHeight();
    ImGui::SetNextWindowSize(ImVec2(currentWidth, 15));
    ImGui::SetNextWindowPos(ImVec2(0, 30));
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiTableFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
    if (ImGui::Begin("##TopBar", nullptr, windowFlags))
    {
        // Play Button
        ImTextureID playIconTexture;
        //if (playing) playIconTexture = activePlayIcon;
        //else playIconTexture = playIcon;

        ImGui::SetCursorPos(ImVec2(currentWidth / 2 - 20, 2));

        if (RaylibWrapper::rlImGuiImageButtonSize("##PlayButton", IconManager::imageTextures["PlayIcon"], ImVec2(20, 20)))
        {
            //startStopPlaying = true;
            //paused = false;
        }

        // Pause Button
        ImTextureID pauseIconTexture;
        //if (paused) pauseIconTexture = activePauseIcon;
        //else if (playing) pauseIconTexture = pauseIcon;
        //else pauseIconTexture = grayedPauseIcon;

        ImGui::SetCursorPos(ImVec2(currentWidth / 2 + 20, 2));

        if (RaylibWrapper::rlImGuiImageButtonSize("##PauseButton", IconManager::imageTextures["GrayedPauseIcon"], ImVec2(20, 20)))
        {
            //if (playing) paused = !paused;
        }

    }
    ImGui::End();
}

void Editor::OnBuildFinish(int success, bool debug) // 0 = failed, 1 = success, 2 = cancelled
{
    if (debug)
    {
        // Moved this to EnterPlayMode()
        //if (success == 1)
        //{
        //    //Utilities::OpenPathInExplorer(ProjectManager::projectData.path / "Internal" / "Builds" / "Windows" / "Debug" / (ProjectManager::projectData.name + ".exe"));

        //    std::filesystem::path path = ProjectManager::projectData.path / "Internal" / "Builds" / "Windows" / "Debug" / (ProjectManager::projectData.name + ".exe");

        //    // Todo: Once I implment debugging, I would use the following function instead:
        //    //Utilities::LaunchProcess(path.string(), "lldb -o run -- \"" + path.string() + "\""); // After this command, "r playmode" still needs to be ran after lldb initializes
        //    std::string error = Utilities::LaunchProcess("\"" + path.string() + "\" playmode");
        //    if (error.empty())
        //    {
        //        asyncPBODisplay.ConnectToSharedMemory();
        //        if (gameViewOpen)
        //        {
        //            ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_GAMEPAD + std::string(" Game")).c_str());
        //            if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
        //                window->DockNode->TabBar->NextSelectedTabId = window->TabId;
        //        }
        //        else
        //            gameViewOpen = true; // Todo: Make sure this focuses the game view
        //    }
        //    else
        //        ConsoleLogger::ErrorLog("Failed to launch play mode: failed to run game. Error: " + error);

        //}
        //else if (success == 0)
        //    ConsoleLogger::ErrorLog("Failed to enter play mode: failed to compile.");
    }
    else
    {
        ImGuiToast toast(success == 1 ? ImGuiToastType::Success : (success == 2 ? ImGuiToastType::Info : ImGuiToastType::Error), 2500, true);
        toast.setTitle(success == 1 ? "Successfully built" : (success == 2 ? "Successfully cancelled" : "Build failed"), "");
        toast.setContent(success == 1 ? "The project has successfully built." : (success == 2 ? "The project has successfully cancelled." : "The project failed to build."));
        ImGui::InsertNotification(toast);

        if (success == 2)
            ConsoleLogger::InfoLog("Build - Build cancelled");
    }

    if (success == 0)
    {
        ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
        if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
            window->DockNode->TabBar->NextSelectedTabId = window->TabId;

        ImGui::SetScrollY(window, window->ContentSize.y);
    }
}

void Editor::EnterPlayMode()
{
    if (playModeActive)
        return;

    std::thread(ProjectManager::BuildToWindows, ProjectManager::projectData, true,
        [this](int success, bool debug) {
            if (success == 1) // 0 = fail, 1 = success, 2 = cancelled
            {
                std::filesystem::path path = ProjectManager::projectData.path / "Internal" / "Builds" / "Windows" / "Debug" / (ProjectManager::projectData.name + ".exe");

                // Todo: Once I implment debugging, I would use the following function instead:
                //Utilities::LaunchProcess(path.string(), "lldb -o run -- \"" + path.string() + "\""); // After this command, "r playmode" still needs to be ran after lldb initializes
                std::pair<std::string, Utilities::JobHandle> result = Utilities::LaunchProcess("\"" + path.string() + "\" playmode"); // Result string = Error
                if (result.first.empty())
                {
                    asyncPBODisplay.ConnectToSharedMemory();
                    playModeActive = true;
                    playModeJobHandle = result.second;

                    // Focus Game window
                    if (gameViewOpen)
                    {
                        ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_GAMEPAD + std::string(" Game")).c_str());
                        if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                            window->DockNode->TabBar->NextSelectedTabId = window->TabId;
                    }
                    else
                        gameViewOpen = true; // Todo: Make sure this focuses the game view
                }
                else
                {
                    ConsoleLogger::ErrorLog("Failed to launch play mode: failed to run game. Error: " + result.first);
                    success = 0;
                }
            }
            else if (success == 0)
                ConsoleLogger::ErrorLog("Failed to enter play mode: failed to compile.");

            // Focus the console to show the errors
            if (success == 0)
            {
                ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
                if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                    window->DockNode->TabBar->NextSelectedTabId = window->TabId;

                ImGui::SetScrollY(window, window->ContentSize.y);
            }
        }).detach();
}

void Editor::ExitPlayMode()
{
    if (!playModeActive)
        return;

    playModeActive = false;
    Utilities::TerminateJob(playModeJobHandle);
    playModeJobHandle = nullptr;
    asyncPBODisplay.Disconnect();
}

void Editor::Render()
{
    // Todo: Put this in RenderDockSpace()
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowPos(ImVec2(0, 31));
    ImGui::SetNextWindowSize(ImVec2(RaylibWrapper::GetScreenWidth(), RaylibWrapper::GetScreenHeight() - 31));
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("##DockSpace", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);
    dockspaceID = ImGui::GetID("DockSpace"); // Todo: Do this in the init so it only sets the ID once
    if (ImGui::DockBuilderGetNode(dockspaceID) == NULL)
    {
        //ImGui::DockBuilderRemoveNode(dockspaceID);
        ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton);
        ImGui::DockBuilderSetNodeSize(dockspaceID, ImGui::GetWindowSize());

        ImGuiID dock_main_id = dockspaceID;
        ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);
        ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.15f, NULL, &dock_main_id);
        ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.20f, NULL, &dock_main_id);

        ImGui::DockBuilderDockWindow((ICON_FA_SITEMAP + std::string(" Hierarchy")).c_str(), dock_id_left);
        ImGui::DockBuilderDockWindow((ICON_FA_CUBES + std::string(" Viewport")).c_str(), dock_main_id);
        ImGui::DockBuilderDockWindow((ICON_FA_GAMEPAD + std::string(" Game")).c_str(), dock_main_id);
        ImGui::DockBuilderDockWindow((ICON_FA_PERSON_RUNNING + std::string(" Animation Graph")).c_str(), dock_main_id);
        ImGui::DockBuilderDockWindow((ICON_FA_BOX_ARCHIVE + std::string(" Asset Manager")).c_str(), dock_main_id);
        ImGui::DockBuilderDockWindow((ICON_FA_BRUSH + std::string(" Canvas Editor")).c_str(), dock_main_id);
        ImGui::DockBuilderDockWindow((ICON_FA_GEARS + std::string(" Properties")).c_str(), dock_id_right);
        ImGui::DockBuilderDockWindow((ICON_FA_FOLDER_OPEN + std::string(" Content Browser")).c_str(), dock_id_bottom);
        ImGui::DockBuilderDockWindow((ICON_FA_CODE + std::string(" Console")).c_str(), dock_id_bottom);
        ImGui::DockBuilderFinish(dockspaceID);
    }
    ImGui::DockSpace(dockspaceID, {0,0}, ImGuiDockNodeFlags_NoWindowMenuButton);
    ImGui::End();
    ImGui::PopStyleVar();

    // Todo: Docksapce and EditorWindow should  be the same

    ImGui::SetNextWindowPos(ImVec2(0, 30));
    ImGui::SetNextWindowSize(ImVec2(RaylibWrapper::GetScreenWidth(), RaylibWrapper::GetScreenHeight() - 30));
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.14f, 0.14f, 0.14f, 1.00f));
    ImGui::Begin("##EditorWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::End();
    ImGui::PopStyleColor();

    // Todo: All the windows below should be changed to a EditorWindow
    RenderViewport();
    RenderGameView();
    RenderHierarchy();
    RenderProperties();
    RenderContentBrowser();
    RenderConsole();
    RenderCameraView();
    RenderComponentsWin();
    RenderScriptCreateWin();
    RenderAnimationGraph();
    for (auto it = windows.begin(); it != windows.end(); ) // Not using a range-based loop so we can remove a window from the vector
    {
        EditorWindow* window = *it;

        ImGui::SetNextWindowClass(&EditorWindow::defaultWindowClass);
        ImGui::SetNextWindowDockID(dockspaceID, ImGuiCond_Once);

        if (window->windowColor.z != -1)
            ImGui::PushStyleColor(ImGuiCol_WindowBg, window->windowColor);

        ImGui::Begin((window->fullWindowName).c_str(), &window->windowOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        window->Render();
        ImGui::End();

        if (window->windowColor.z != -1)
            ImGui::PopStyleColor();

        if (!window->windowOpen)
            window->OnClose();

        if (!window->windowOpen) // Checking again in case it got changed in OnClose()
        {
            delete window;
            it = windows.erase(it);
        }
        else
            ++it;
    }

    //CanvasEditor::Render();
    RenderProjectSettings();
    AssetManager::RenderWindow();
    //RenderTopbar();

    // Todo: Move this to RenderTitleBar()
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(310, 30));
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##MenuBar", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus);

    if (ImGui::BeginMenuBar())
    {
        ImGui::SetCursorPos(ImVec2(5, 0));
        if (ImGui::BeginMenu("File", !ImGuiPopup::IsActive())) {
            if (ImGui::MenuItem("Save Project", "Ctrl+S"))
            {
                SceneManager::SaveScene(SceneManager::GetActiveScene());
                ImGuiToast toast(ImGuiToastType::Success, 1500, true);
                toast.setTitle("Project Saved", "");
                toast.setContent("The project has successfully saved.");
                ImGui::InsertNotification(toast);
            }

            if (playModeActive)
                ImGui::BeginDisabled();
            if (ImGui::MenuItem("Play", "Ctrl+P"))
            {
                // Todo: Check if changes have been made and it needs to be recompiled
                //std::thread(ProjectManager::BuildToWindows, ProjectManager::projectData, true, &Editor::OnBuildFinish).detach();
                //std::thread(ProjectManager::BuildToWindows, ProjectManager::projectData, true,
                //    [this](int success, bool debug) {
                //        this->OnBuildFinish(success, debug);
                //    }).detach();
                EnterPlayMode();
            }
            if (playModeActive)
                ImGui::EndDisabled();

            if (ImGui::BeginMenu("Build Project"))
            {
                if (ImGui::MenuItem("Windows", ""))
                    //std::thread(ProjectManager::BuildToWindows, ProjectManager::projectData, false, &Editor::OnBuildFinish).detach();
                    std::thread(ProjectManager::BuildToWindows, ProjectManager::projectData, false,
                        [this](int success, bool debug) {
                            this->OnBuildFinish(success, debug);
                        }).detach();

                if (ImGui::MenuItem("Web", ""))
                    //std::thread(ProjectManager::BuildToWeb, ProjectManager::projectData, &Editor::OnBuildFinish).detach();
                    std::thread(ProjectManager::BuildToWeb, ProjectManager::projectData,
                        [this](int success, bool debug) {
                            this->OnBuildFinish(success, debug);
                        }).detach();

                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::SetCursorPos(ImVec2(43, 0));
        if (ImGui::BeginMenu("Edit", !ImGuiPopup::IsActive())) {
            if (ImGui::MenuItem("Project Settings", ""))
            {
                if (projectSettingsWinOpen)
                {
                    ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_GEARS + std::string(" Project Properties")).c_str());
                    if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                        window->DockNode->TabBar->NextSelectedTabId = window->TabId;
                }
                else
                {
                    projectSettingsWinOpen = true;
                    resetProjectSettings = true;
                }
            }
            //if (ImGui::MenuItem("Reload API Files", "")) // Todo: Move this into settings or help
            //{
            //    ConsoleLogger::InfoLog("Reloading API Files");
            //    if (!std::filesystem::exists(ProjectManager::projectData.path / "api"))
            //    {
            //        std::filesystem::create_directories(ProjectManager::projectData.path / "api");
            //        Utilities::HideFile(ProjectManager::projectData.path / "api");
            //    }
            //    std::filesystem::remove_all(ProjectManager::projectData.path / "api");
            //    ProjectManager::CopyApiFiles(std::filesystem::path(__FILE__).parent_path(), ProjectManager::projectData.path / "api");
            //}
            ImGui::EndMenu();
        }
        ImGui::SetCursorPos(ImVec2(85, 0));
        if (ImGui::BeginMenu("Window", !ImGuiPopup::IsActive())) {
            if (ImGui::MenuItem("Hierarchy", "")) {}
            if (ImGui::MenuItem("Content Browser", "")) {}
            if (ImGui::MenuItem("Console", "")) {}
            if (ImGui::MenuItem("Properties", "")) {}
            if (ImGui::MenuItem("Sprite Editor", "")) {}
            if (ImGui::MenuItem("Animation Graph", ""))
            {
                animationGraphWinOpen = true;
                ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_PERSON_RUNNING + std::string(" Animation Graph")).c_str());
                if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                    window->DockNode->TabBar->NextSelectedTabId = window->TabId;
            }
            if (ImGui::MenuItem("Asset Manager", ""))
            {
                AssetManager::open = true;
                ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_BOX_ARCHIVE + std::string(" Asset Manager")).c_str());
                if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                    window->DockNode->TabBar->NextSelectedTabId = window->TabId;
            }
            //if (ImGui::MenuItem("Canvas Editor", "")) { CanvasEditor::windowOpen = true; }
            ImGui::EndMenu();
        }
        ImGui::SetCursorPos(ImVec2(152, 0));
        if (ImGui::BeginMenu("Asset Manager", !ImGuiPopup::IsActive()))
        {
            AssetManager::open = true;
            ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_BOX_ARCHIVE + std::string(" Asset Manager")).c_str());
            if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                window->DockNode->TabBar->NextSelectedTabId = window->TabId;
            ImGui::EndMenu();
        }
        ImGui::SetCursorPos(ImVec2(263, 0));
        if (ImGui::BeginMenu("Help", !ImGuiPopup::IsActive())) {
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End();

    // Popup Overlay to block input for like building
    if (ImGuiPopup::IsActive())
    {
        ImGui::SetNextWindowBgAlpha(0.2f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1, 1, 1, 1));
        ImGui::SetNextWindowPos(ImVec2(0, 30));
        ImGui::SetNextWindowSize(ImVec2(RaylibWrapper::GetScreenWidth(), RaylibWrapper::GetScreenHeight() - 30));
        ImGui::Begin("##PopupOverlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
    else // This makes sure stuff can't be changed while a popup or building is happening
    {
        if (ImGui::IsKeyPressed(ImGuiKey_S) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
        {
            SceneManager::SaveScene(SceneManager::GetActiveScene());
            ImGuiToast toast(ImGuiToastType::Success, 1500, true);
            toast.setTitle("Project Saved", "");
            toast.setContent("The project has successfully saved.");
            ImGui::InsertNotification(toast);
        }
    }

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(RaylibWrapper::GetScreenWidth(), 30));
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.14f, 0.14f, 0.14f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.14f, 0.14f, 1.00f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("##MenuBar2", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Todo: It may be best to calculate the position only in Init and window resize.
    // Todo: Store the version in a variable and use that both in this titlebar, and the Raylib window titlebar 
    std::string text = "Cryonic Engine v0.1 - " + ProjectManager::projectData.name;
    ImGui::SetCursorPos(ImVec2((ImGui::GetWindowSize().x - ImGui::CalcTextSize(text.c_str()).x) * 0.5f, 7.5f));
    ImGui::Text((text).c_str());

    ImGui::PushFont(FontManager::GetFont("Familiar-Pro-Bold", 25, false));
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 60, 0));
    if (ImGui::Button("-", ImVec2(30, 30)))
    {
        RaylibWrapper::MinimizeWindow();
    }
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 30, 0));
    if (ImGui::Button("x", ImVec2(30, 30)))
    {
        // Todo: Popup save warning if the scene has not been saved. Popup options "Save & Quit", "Quit Without Saving", and "Cancel"
        closeEditor = true;
    }
    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();


    // Notifications
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f));

    ImGui::RenderNotifications();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(1);


    // Drag text
    if (dragData.first != None && !viewportHovered || dragData.first != None && viewportHovered && !(dragData.first == ImageFile || dragData.first == ModelFile))
    {
        std::filesystem::path dragPath = std::any_cast<std::filesystem::path>(dragData.second["Path"]);
        ImVec2 textSize = ImGui::CalcTextSize(dragPath.filename().string().c_str());
        ImGui::SetNextWindowPos(ImGui::GetMousePos()); // Places it at the bottom right of the mouse
        //ImGui::SetNextWindowPos({ ImGui::GetMousePos().x - (textSize.x/2), ImGui::GetMousePos().y - textSize.y - 20}); // Places it at the top of the mouse
        ImGui::SetNextWindowSize({ textSize.x + 10, textSize.y });
        if (ImGui::Begin("##DragText", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::Text((dragPath.filename().string() + "##DragText").c_str());
            ImGui::End();
        }
    }


    ImGuiPopup::Render();
}

void Editor::SetupViewport()
{
    ViewTexture = RaylibWrapper::LoadRenderTexture(RaylibWrapper::GetScreenWidth(), RaylibWrapper::GetScreenHeight());

    if (ProjectManager::projectData.is3D)
    {
        camera.fovy = 45;
        camera.up.y = 1;
        camera.position.y = 3;
        camera.position.z = -25;
        camera.projection = RaylibWrapper::CAMERA_PERSPECTIVE;
        camera.target = { 0.0f, 0.0f, 0.0f };
    }
    else
    {
        camera.projection = RaylibWrapper::CAMERA_ORTHOGRAPHIC;
        camera.up.y = 1;
        camera.fovy = 45;
        camera.target = { 0.0f, 0.0f, 0.0f };
        camera.position = {0, 3, 500};
    }
}

void CloseViewport()
{
    RaylibWrapper::UnloadRenderTexture(ViewTexture);
    RaylibWrapper::UnloadTexture(GridTexture);
}

void Editor::InitFonts()
{
    // First font created is the default folt. This should be Familiar-Pro-Bold 15
    FontManager::LoadFonts("Familiar-Pro-Bold", { 15, 10, 18, 20, 25, 30 });
    FontManager::LoadFont("BoldMarker", 90);
    FontManager::LoadFont("fa-solid-900", 15, true);
    FontManager::LoadFont("Roboto-Bold", 15);
    FontManager::LoadFont("Roboto-Medium", 12);
}

void Editor::InitStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 1.0f;
    style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.50f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1, 1, 1, 1.00f); // ImVec4(0.11f, 0.64f, 0.92f, 1.00f)
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    style.GrabRounding = style.FrameRounding = 5;
}

void Editor::InitMisc()
{
    //if (projectData.path.stem().string() != projectData.name)
    //    projectData.path = projectData.path / projectData.name;

    char* userProfile;
    size_t len;
    _dupenv_s(&userProfile, &len, "USERPROFILE");

    std::filesystem::path projectsPath = std::filesystem::path(userProfile) / "Documents" / "Cryonic Engine Projects";
    if (!std::filesystem::exists(projectsPath)) std::filesystem::create_directory(projectsPath);
}

void Editor::InitScenes()
{
    std::filesystem::path scenesPath = ProjectManager::projectData.path / "Assets" / "Scenes";
    std::filesystem::path selectedScenePath;
    if (std::filesystem::exists(scenesPath))
    {
        for (const auto& file : std::filesystem::directory_iterator(scenesPath))
        {
            if (file.is_regular_file() && file.path().extension() == ".scene" && SceneManager::LoadScene(file.path()))
            {
                selectedScenePath = file.path();
                break;
            }
        }
    }

    // No scenes found, create a new one
    if (selectedScenePath.empty())
    {
        if (std::filesystem::exists(ProjectManager::projectData.path / "Assets" / "Scenes" / "Default.scene"))
            SceneManager::LoadScene(ProjectManager::projectData.path / "Assets" / "Scenes" / "Default.scene");
        else
        {
            std::filesystem::create_directories(scenesPath);
            SceneManager::CreateScene(scenesPath / "Default.scene");
            SceneManager::LoadScene(scenesPath / "Default.scene");
            //SceneManager::SetActiveScene(); // LoadScene sets the scene to active
            //SceneManager::SaveScene(SceneManager::GetActiveScene()); // Creates Default.scene. It should not create a scene file automatically

        }
    }
    //else SceneManager::SetActiveScene(&SceneManager::GetScenes()->back()); // The scene is already getting set to active in the LoadScene() above
}

void Editor::InitMaterialPreview()
{
    materialPreviewMesh.Create(ModelType::Sphere, "MaterialPreview", ShaderManager::LitStandard, "MaterialPreview");
    Material::LoadPreviewMaterial();
    
    materialPreviewMesh.SetMaterials({ &Material::previewMaterial });

    // Setup lighting shader

    float lightPositions[3] = { 0.0f, 0.0f, 0.0f };
    float lightColors[3] = { 1.0f, 1.0f, 1.0f };
    int lightType[1] = { 0 }; // 0 = Point, 1 = Spot, 2 = Directional
    float ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    int enableShadows = 0;

    materialPreviewMesh.SetShaderValue(0, materialPreviewMesh.GetShaderLocation(0, "lightPos"), lightPositions, RaylibWrapper::SHADER_UNIFORM_VEC3);
    materialPreviewMesh.SetShaderValue(0, materialPreviewMesh.GetShaderLocation(0, "lightColor"), lightColors, RaylibWrapper::SHADER_UNIFORM_VEC4);
    materialPreviewMesh.SetShaderValue(0, materialPreviewMesh.GetShaderLocation(0, "lightType"), &lightType, RaylibWrapper::SHADER_UNIFORM_INT);
    materialPreviewMesh.SetShaderValue(0, materialPreviewMesh.GetShaderLocation(0, "ambient"),ambient, RaylibWrapper::SHADER_UNIFORM_VEC4);
    materialPreviewMesh.SetShaderValue(0, materialPreviewMesh.GetShaderLocation(0, "enableShadows"), &enableShadows, RaylibWrapper::SHADER_UNIFORM_INT);
}

void Editor::CleanupCache()
{
    // Cleanup every second
    static float timeSinceLastCleanup = 0.0f;
    timeSinceLastCleanup += RaylibWrapper::GetFrameTime();

    if (timeSinceLastCleanup < 1.0f)
        return;

    timeSinceLastCleanup = 0;

    // Textures
    for (auto it = cachedTextures.begin(); it != cachedTextures.end(); )
    {
        if (RaylibWrapper::GetTime() - it->second.second >= 2) // Checks if the texture was used in the last 2 seconnds
        {
            RaylibWrapper::UnloadTexture(it->second.first);
            it = cachedTextures.erase(it);
        }
        else
            ++it;
    }
}

void Editor::Cleanup()
{
    IconManager::Cleanup();
    ShaderManager::Cleanup();
    AssetManager::Cleanup();
    Material::UnloadDefaultMaterial();
    ShadowManager::UnloadShaders();
    materialPreviewMesh.Unload();
    Material::UnloadPreviewMaterial();
    Material::UnloadWhiteTexture();

    for (auto& image : tempTextures)
    {
        UnloadTexture(*image);
        //delete image;
    }
    tempTextures.clear();

    for (auto& image : tempRenderTextures)
    {
        UnloadRenderTexture(*image);
        //delete image;
    }
    tempRenderTextures.clear();

    for (EditorWindow* window : windows)
        delete window;
    windows.clear();

    for (GameObject* gameObject : SceneManager::GetActiveScene()->GetGameObjects())
    {
        for (Component* component : gameObject->GetComponents())
        {
            component->Destroy();
        }
    }

    if (cameraSelected) // Todo: This needs to be changed as its possible there's a render texture from just viewing the Game view
        UnloadRenderTexture(cameraRenderTexture);

    RaylibWrapper::CloseAudioDevice();

    RaylibWrapper::ImGui_ImplRaylib_Shutdown();
    ImNodes::DestroyContext();
    ImGui::DestroyContext();
}

void Editor::Init()
{
    RaylibWrapper::SetConfigFlags(RaylibWrapper::FLAG_WINDOW_UNDECORATED | RaylibWrapper::FLAG_WINDOW_RESIZABLE | RaylibWrapper::FLAG_WINDOW_HIGHDPI);// FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT
    RaylibWrapper::InitWindow(RaylibWrapper::GetScreenWidth(), RaylibWrapper::GetScreenHeight(), ("Cryonic Engine v0.1 - " + ProjectManager::projectData.name).c_str());
    RaylibWrapper::MaximizeWindow();
    RaylibWrapper::SetWindowMinSize(100, 100);
    RaylibWrapper::SetTargetFPS(144); // Todo: Set target FPS to monitor refresh rate and handle editor being moved across monitors or just take the higher refresh rate

    RaylibWrapper::InitAudioDevice();

    // Setup Dear ImGui context
    ImGui::CreateContext();
    ImNodes::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enabled Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enabled Multi-Viewports
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.NavActive = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    InitStyle();

    EditorWindow::defaultWindowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoWindowMenuButton;

    // Setup Platform/Renderer backends
    RaylibWrapper::ImGui_ImplRaylib_Init();
    FontManager::InitFontManager();
    InitFonts();
    //InitImages();
    IconManager::Init();
    ShaderManager::Init();
    ShadowManager::LoadShaders();
    Material::LoadWhiteTexture();
    Material::LoadDefaultMaterial();
    InitMaterialPreview();
    InitMisc();
    InitScenes(); // Must go after InitMisc() and ShaderManager::Init()

    SetupViewport();
    AssetManager::Init(&EditorWindow::defaultWindowClass);

    FileWatcher::Init();
    ScriptHeaderGenerator::Init();

    while (!closeEditor)
    {
        MainThreadQueue::Process();

        oneSecondDelay -= RaylibWrapper::GetFrameTime();
        FontManager::UpdateFonts();
        if (CameraComponent::main != nullptr)
            ShaderManager::UpdateShaders(CameraComponent::main->gameObject->transform.GetPosition().x, CameraComponent::main->gameObject->transform.GetPosition().y, CameraComponent::main->gameObject->transform.GetPosition().z);

        RaylibWrapper::ImGui_ImplRaylib_ProcessEvents();

        RaylibWrapper::ImGui_ImplRaylib_NewFrame();
        ImGui::NewFrame();

        UpdateViewport();
        Render();

        // Rendering
        ImGui::Render();

        RaylibWrapper::BeginDrawing();
        RaylibWrapper::ClearBackground({64,64,64,255});
        RaylibWrapper::ImGui_ImplRaylib_RenderDrawData(ImGui::GetDrawData());
        RaylibWrapper::EndDrawing();

        for (auto& image : tempTextures)
        {
            RaylibWrapper::UnloadTexture(*image);
            delete image;
        }
        tempTextures.clear();

        for (auto& image : tempRenderTextures)
        {
            RaylibWrapper::UnloadRenderTexture(*image);
        }
        tempRenderTextures.clear();

        CleanupCache();

        //if (!cameraSelected) // Todo: This needs to be changed as its possible that the camera view is selected
        //    RaylibWrapper::UnloadRenderTexture(cameraRenderTexture);

        if (oneSecondDelay <= 0)
            oneSecondDelay = 1;
    }

    for (EditorWindow* window : windows)
        window->OnClose();

    Cleanup();
    RaylibWrapper::CloseWindow();
}
