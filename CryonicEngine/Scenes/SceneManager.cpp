#include "SceneManager.h"
#include <fstream>
#include <iomanip>
#include "../ConsoleLogger.h"
#include <iostream>
#include <sstream>
//#include "../Editor.h"
#include <algorithm>
#include <cctype>
#include "../json.hpp"

#include "../Components/ScriptLoader.h"
#include "../Components/Component.h"
#include "../Components/MeshRenderer.h"
#include "../Components/Terrain.h"
#include "../Components/ScriptComponent.h"
#include "../Components/CameraComponent.h"
#include "../Components/Lighting.h"
#include "../Components/SpriteRenderer.h"
#include "../Components/Collider2D.h"
#include "../Components/Rigidbody2D.h"
#if defined(IS3D) || defined(EDITOR)
#include "../Components/Collider3D.h"
#include "../Components/Rigidbody3D.h"
#endif
#include "../Components/AnimationPlayer.h"
#include "../Components/AudioPlayer.h"
#include "../Components/TilemapRenderer.h"
#include "../Components/Label.h"
#include "../Components/Image.h"
#include "../Components/Button.h"
#include "../Components/CanvasRenderer.h"

#if defined(EDITOR)
#include "../ProjectManager.h"
#else
#include "../Game.h"
#include "../EventSystem.h"
#endif
#include "../RaylibWrapper.h"

using json = nlohmann::json;

std::deque<Scene> SceneManager::m_scenes;
Scene* SceneManager::m_activeScene;

SceneManager::SceneManager() {
    m_activeScene = nullptr;
}

Scene* SceneManager::GetActiveScene() {
    return m_activeScene;
}

void SceneManager::SetActiveScene(Scene* scene) {
    if (m_activeScene != nullptr)
    {
        if (scene->GetPath() == m_activeScene->GetPath())
            return;

#if !defined(EDITOR)
        EventSystem::Invoke("ActiveSceneChanged");
#endif

        UnloadScene(m_activeScene); // Todo: Maybe this should be moved to LoadScene()
    }
    m_activeScene = scene;
}

std::deque<Scene>* SceneManager::GetScenes() {
    return &m_scenes; 
}

bool SceneManager::SaveScene(Scene* scene)
{
    json sceneData;

    // Save game objects
    for (GameObject* object : scene->GetGameObjects())
    {
        json gameObjectData;

        // Save texture
        //if (object->GetTexture() != nullptr) {
        //    gameObjectData["texture_path"] = object->GetPath();
        //}

        //gameObjectData["model_path"] = object->GetModelPath().string();
        gameObjectData["name"] = object->GetName();
        gameObjectData["position"] = { object->transform.GetPosition().x, object->transform.GetPosition().y, object->transform.GetPosition().z };
        //gameObjectData["real_size"] = { object->GetRealSize().x, object->GetRealSize().y, object->GetRealSize().z };
        gameObjectData["size"] = { object->transform.GetScale().x, object->transform.GetScale().y, object->transform.GetScale().z};
        gameObjectData["rotation"] = { object->transform.GetRotation().x, object->transform.GetRotation().y, object->transform.GetRotation().z, object->transform.GetRotation().w };
        gameObjectData["id"] = object->GetId();
        gameObjectData["active"] = object->IsActive();
        gameObjectData["globalActive"] = object->IsGlobalActive();

        //gameObjectData["tint"] = { object->GetTint().Value.x, object->GetTint().Value.y, object->GetTint().Value.z, object->GetTint().Value.w };
        //gameObjectData["z_order"] = object->GetZOrder();

        if (object->GetParent() != nullptr)
            gameObjectData["parent_id"] = object->GetParent()->GetId();
        else
            gameObjectData["parent_id"] = -1;

        // Save components
        json componentsData;
        for (Component* component : object->GetComponents())
        {
            json componentData;
            componentData["name"] = component->name;
            componentData["active"] = component->IsActive();
            componentData["id"] = component->id;
            #if defined(EDITOR)
            componentData["exposed_variables"] = component->exposedVariables;
            #endif
            //componentData["runInEditor"] = component->runInEditor;

            // Temporary solution
            // Todo: maybe put these in Exposed Variables (but not visible in Properties), or something similar to ExposedVariables like SerializedVariables
            if (dynamic_cast<MeshRenderer*>(component))
            {
                componentData["model_path"] = dynamic_cast<MeshRenderer*>(component)->GetModelPath();
            }
			else if (dynamic_cast<Terrain*>(component))
			{
				componentData["height_data"] = dynamic_cast<Terrain*>(component)->SerializeHeightData();
			}
            else if (dynamic_cast<ScriptComponent*>(component))
            {
                auto formatPathForUnix = [](std::string path) {
                    std::replace(path.begin(), path.end(), '\\', '/');
                    return path;
                };

                componentData["cpp_path"] = formatPathForUnix(dynamic_cast<ScriptComponent*>(component)->GetCppPath().string());
                componentData["header_path"] = formatPathForUnix(dynamic_cast<ScriptComponent*>(component)->GetHeaderPath().string());
            }

            componentsData.push_back(componentData);
        }
        gameObjectData["components"] = componentsData;

        // Add game object data to scene data
        sceneData["game_objects"].push_back(gameObjectData);
    }

    //std::cout << sceneData.dump(4) << std::endl;
    // 
    // Write JSON data to file
    //ConsoleLogger::InfoLog("Saved scene at " + scene->GetPath().string());

    std::string formattedPath = scene->GetPath().string();
    std::size_t found = formattedPath.find('\\');
    while (found != std::string::npos) {
        formattedPath.replace(found, 1, "\\\\");
        found = formattedPath.find('\\', found + 2);
    }
    formattedPath.erase(std::remove_if(formattedPath.begin(), formattedPath.end(), [](char c) {
        return !std::isprint(static_cast<unsigned char>(c));
        }), formattedPath.end());

    if (!std::filesystem::exists(scene->GetPath().parent_path()))
        std::filesystem::create_directories(scene->GetPath().parent_path());

    std::ofstream file(formattedPath);
    file << std::setw(4) << sceneData << std::endl;
    file.close();

    if (file.fail() && file.bad())
    {
        if (file.eof())
        {
            ConsoleLogger::ErrorLog("End of file reached while saving the scene '" + scene->GetPath().stem().string() + "' at the path '" + scene->GetPath().string() + "'");
        }
        else if (file.fail())
        {
            ConsoleLogger::ErrorLog("The scene \"" + scene->GetPath().stem().string() + "\" at the path \"" + scene->GetPath().string() + "\" failed to open. The file can't be found or you have invalid permissions.");
        }
        else if (file.bad())
        {
            char errorMessage[256];
#ifdef WINDOWS
            strerror_s(errorMessage, sizeof(errorMessage), errno);
#else
            strerror_r(errno, errorMessage, sizeof(errorMessage));
#endif
            ConsoleLogger::WarningLog("The scene \"" + scene->GetPath().stem().string() + "\" failed to save. Error: " + std::string(errorMessage));
        }
        else if (file.is_open())
        {
            ConsoleLogger::ErrorLog("The scene \"" + scene->GetPath().string() + "\" failed to save because it is open in another program");
        }
        else
        {
            char errorMessage[256];
#ifdef WINDOWS
            strerror_s(errorMessage, sizeof(errorMessage), errno);
#else
            strerror_r(errno, errorMessage, sizeof(errorMessage));
#endif
            ConsoleLogger::WarningLog("The scene \"" + scene->GetPath().stem().string() + "\" failed to save. Error: " + std::string(errorMessage));
        }
        return false;
    }
    ConsoleLogger::InfoLog("The scene \"" + scene->GetPath().stem().string() + "\" has been saved");
    return true;
}

bool SceneManager::LoadScene(std::filesystem::path filePath)
{
    if (!std::filesystem::exists(filePath) || filePath.extension() != ".scene")
    {
        ConsoleLogger::WarningLog("The path for the scene \"" + filePath.stem().string() + "\" is invalid, \"" + filePath.string() + "\"");
        return false;
    }

    std::string filePathString = filePath.string();
    filePathString.erase(std::remove_if(filePathString.begin(), filePathString.end(),
        [](char c) { return !std::isprint(c); }), filePathString.end());

    // Todo: Check if the scene is already loaded

    // Load file into string
    std::ifstream file(filePathString);
    if (!file.is_open()) {
        ConsoleLogger::WarningLog("Can not open the scene \"" + filePath.stem().string() + "\" at the path \"" + filePath.string() + "\"");
        return false;
    }
    std::stringstream fileStream;
    fileStream << file.rdbuf();
    file.close();

    //ConsoleLogger::InfoLog(fileStream.str());

    // Parse JSON data
    json sceneData = json::parse(fileStream.str());

    // Create new scene
    Scene scene = Scene(filePath, {});

    std::unordered_map<int, int> parentObjects;

    auto setExposedVariables = [&](Component& component, auto& componentData)
    {
        component.SetActive(componentData["active"]);
        // Sets exposed variables, and updates them if needed
#if defined(EDITOR)
        if (component.exposedVariables == nullptr)
            component.exposedVariables = componentData["exposed_variables"];
        else if (!componentData["exposed_variables"].is_null())
        {
            for (auto jsonExposedVariable = componentData["exposed_variables"][1].begin(); jsonExposedVariable != componentData["exposed_variables"][1].end(); ++jsonExposedVariable)
            {
                for (auto exposedVariable = component.exposedVariables[1].begin(); exposedVariable != component.exposedVariables[1].end(); ++exposedVariable)
                {
                    if ((*jsonExposedVariable)[0] == (*exposedVariable)[0] && (*jsonExposedVariable)[1] == (*exposedVariable)[1])
                    {
                        (*exposedVariable)[2] = (*jsonExposedVariable)[2];
                        break;
                    }
                }
            }
        }
#endif
    };

    // Create game objects
    for (const auto& gameObjectData : sceneData["game_objects"])
    {
        // Create new game object
        GameObject* gameObject = scene.AddGameObject(gameObjectData["id"]);
        std::string objectName = gameObjectData["name"];

        // Load name, position, real size, size, rotation, id, tint, zOrder
        //gameObject->SetModelPath(gameObjectData["model_path"]);
        //gameObject->SetModel(LoadModel(gameObject->GetModelPath().string().c_str()));
        gameObject->SetName(gameObjectData["name"]);
        gameObject->transform.SetPosition(Vector3{ gameObjectData["position"][0], gameObjectData["position"][1], gameObjectData["position"][2] });
        //gameObject->transform.SetRealSize(Vector3{gameObjectData["real_size"][0], gameObjectData["real_size"][1], gameObjectData["real_size"][2] });
        gameObject->transform.SetScale(Vector3{ gameObjectData["size"][0], gameObjectData["size"][1], gameObjectData["size"][2] });
        //gameObject->SetRotation(Quaternion{ gameObjectData["rotation"][0], gameObjectData["rotation"][1], gameObjectData["rotation"][2] });
        gameObject->transform.SetRotation(Quaternion{ gameObjectData["rotation"][0], gameObjectData["rotation"][1], gameObjectData["rotation"][2], gameObjectData["rotation"][3]});
        //gameObject->SetTint(ImVec4(gameObjectData["tint"][0], gameObjectData["tint"][1], gameObjectData["tint"][2], gameObjectData["tint"][3]));
        //gameObject->SetZOrder(gameObjectData["z_order"]);
        gameObject->SetActive(gameObjectData["active"]);
        gameObject->SetGlobalActive(gameObjectData["globalActive"]);

        parentObjects[gameObjectData["id"]] = gameObjectData["parent_id"];

        // Load components
        for (const auto& componentData : gameObjectData["components"])
        {
            // Temporary solution
            if (componentData["name"] == "MeshRenderer")
            {
                MeshRenderer& component = gameObject->AddComponentInternal<MeshRenderer>(componentData["id"]);
                //component.gameObject = &gameObject;
                component.SetModelPath(componentData["model_path"]);
                setExposedVariables(component, componentData);

                if (component.GetModelPath().string() == "Cube")
                    component.SetModel(Cube, component.GetModelPath().string(), ShaderManager::LitStandard);
                else if (component.GetModelPath().string() == "Plane")
                    component.SetModel(Plane, component.GetModelPath().string(), ShaderManager::LitStandard);
                else if (component.GetModelPath().string() == "Sphere")
                    component.SetModel(Sphere, component.GetModelPath().string(), ShaderManager::LitStandard);
                else if (component.GetModelPath().string() == "Cylinder")
                    component.SetModel(Cylinder, component.GetModelPath().string(), ShaderManager::LitStandard);
                else if (component.GetModelPath().string() == "Cone")
                    component.SetModel(Cone, component.GetModelPath().string(), ShaderManager::LitStandard);
                else
                    component.SetModel(Custom, component.GetModelPath().string(), ShaderManager::LitStandard);
            }
            else if (componentData["name"] == "SpriteRenderer")
            {
                SpriteRenderer& component = gameObject->AddComponentInternal<SpriteRenderer>(componentData["id"]);
                setExposedVariables(component, componentData);
            }
            else if (componentData["name"] == "ScriptComponent")
            {
#if defined(EDITOR)
                ScriptComponent& component = gameObject->AddComponentInternal<ScriptComponent>(componentData["id"]);
                //component.gameObject = &gameObject;
                component.SetCppPath(componentData["cpp_path"]);
                component.SetHeaderPath(componentData["header_path"]);
                component.SetName(component.GetHeaderPath().stem().string());
                component.SetActive(componentData["active"]);
                component.exposedVariables = componentData["exposed_variables"];
                //component.name = component.GetName();
#else
                SetupScriptComponent(gameObject, componentData["id"], componentData["active"], std::filesystem::path(componentData["header_path"]).stem().string());
#endif
            }
			else if (componentData["name"] == "Terrain")
			{
				Terrain& component = gameObject->AddComponentInternal<Terrain>(componentData["id"]);
				setExposedVariables(component, componentData);
                component.LoadHeightData(componentData["height_data"]);
			}
            else if (componentData["name"] == "CameraComponent")
            {
                CameraComponent& component = gameObject->AddComponentInternal<CameraComponent>(componentData["id"]);
                setExposedVariables(component, componentData);
            }
            else if (componentData["name"] == "Lighting")
            {
                Lighting& component = gameObject->AddComponentInternal<Lighting>(componentData["id"]);
                setExposedVariables(component, componentData);
            }
            else if (componentData["name"] == "Collider2D")
            {
                Collider2D& component = gameObject->AddComponentInternal<Collider2D>(componentData["id"]);
                setExposedVariables(component, componentData);
            }
            else if (componentData["name"] == "Rigidbody2D")
            {
                Rigidbody2D& component = gameObject->AddComponentInternal<Rigidbody2D>(componentData["id"]);
                setExposedVariables(component, componentData);
            }
#if defined(IS3D) || defined(EDITOR)
            else if (componentData["name"] == "Collider3D")
            {
                Collider3D& component = gameObject->AddComponentInternal<Collider3D>(componentData["id"]);
                setExposedVariables(component, componentData);
            }
            else if (componentData["name"] == "Rigidbody3D")
            {
                Rigidbody3D& component = gameObject->AddComponentInternal<Rigidbody3D>(componentData["id"]);
                setExposedVariables(component, componentData);
            }
#endif
            else if (componentData["name"] == "AnimationPlayer")
            {
                AnimationPlayer& component = gameObject->AddComponentInternal<AnimationPlayer>(componentData["id"]);
                setExposedVariables(component, componentData);
            }
            else if (componentData["name"] == "AudioPlayer")
            {
                AudioPlayer& component = gameObject->AddComponentInternal<AudioPlayer>(componentData["id"]);
                setExposedVariables(component, componentData);
            }
            else if (componentData["name"] == "TilemapRenderer")
            {
                TilemapRenderer& component = gameObject->AddComponentInternal<TilemapRenderer>(componentData["id"]);
                setExposedVariables(component, componentData);
            }
            else if (componentData["name"] == "Label")
            {
                Label& component = gameObject->AddComponentInternal<Label>(componentData["id"]);
                setExposedVariables(component, componentData);
            }
            else if (componentData["name"] == "Image")
            {
                Image& component = gameObject->AddComponentInternal<Image>(componentData["id"]);
                setExposedVariables(component, componentData);
            }
            else if (componentData["name"] == "Button")
            {
                Button& component = gameObject->AddComponentInternal<Button>(componentData["id"]);
                setExposedVariables(component, componentData);
            }
            else if (componentData["name"] == "CanvasRenderer")
            {
                CanvasRenderer& component = gameObject->AddComponentInternal<CanvasRenderer>(componentData["id"]);
                setExposedVariables(component, componentData);
            }

            // Todo: make Component of type Component so then I can set the Component variables like SetActive, id, expoedVariables, etc only once and not in each if statement
        }

        // Set exposed variables values, then call Awake() and Enable()
#if !defined(EDITOR)
        for (Component* component : gameObject->GetComponents())
        {
            component->SetExposedVariables();
            component->initialized = true;
            if (gameObject->IsActive() && gameObject->IsGlobalActive())
            {
                component->Awake();
                component->awakeCalled = true;
                if (component->IsActive() && gameObject->IsActive() && gameObject->IsGlobalActive())
                    component->Enable();
            }
        }
#endif

        // Add game object to scene
        //scene.AddGameObject();
    }

    // Moved this below
    //for (GameObject* gameObject : scene.GetGameObjects())
    //{
    //    // Set parents
    //    if (parentObjects[gameObject->GetId()] != 0)
    //        for (GameObject* go : scene.GetGameObjects())
    //            if (go->GetId() == parentObjects[gameObject->GetId()])
    //            {
    //                gameObject->SetParent(go);
    //                break;
    //            }
    //}

    // Todo: I'm not sure if this is needed. I should check if the scene is already loaded above.
    bool sceneFound = false;
    for (Scene& scenes : m_scenes)
        if (scenes.GetPath() == scene.GetPath())
        {
            SetActiveScene(&scenes);
            sceneFound = true;
        }

    if (!sceneFound)
    {
        AddScene(scene);
        SetActiveScene(&GetScenes()->back());

        for (GameObject* gameObject : GetActiveScene()->GetGameObjects())
            for (Component* component : gameObject->GetComponents())
                component->gameObject = gameObject;
    }

    for (GameObject* gameObject : scene.GetGameObjects())
    {
        // Set parents
        if (parentObjects[gameObject->GetId()] != 0)
            for (GameObject* go : scene.GetGameObjects())
                if (go->GetId() == parentObjects[gameObject->GetId()])
                {
                    gameObject->SetParent(go);
                    break;
                }

#if !defined(EDITOR)
        if (!gameObject->IsActive() || !gameObject->IsGlobalActive())
            continue;
        for (Component* component : gameObject->GetComponents())
        {
            if (!component->IsActive())
                continue;
            component->Start();
            component->startCalled = true;
        }
#endif
    }

    ConsoleLogger::InfoLog("The scene \"" + filePath.stem().string() + "\" has been loaded");

    return true;
}

void SceneManager::UnloadScene(Scene* scene)
{
    std::deque<GameObject*> gameObjectsCopy = m_activeScene->GetGameObjects();
    for (GameObject* gameObject : gameObjectsCopy)
        scene->RemoveGameObject(gameObject);

    auto it = std::find_if(m_scenes.begin(), m_scenes.end(), [scene](const Scene& s) { return *scene == s; });
    if (it != m_scenes.end())
        m_scenes.erase(it);
}

void SceneManager::AddScene(Scene scene) {
    m_scenes.push_back(scene);
}

void SceneManager::CreateScene(std::filesystem::path path)
{
    Scene scene;
    //AddScene(scene);
    scene.SetPath(path);

#if defined(EDITOR)
    GameObject* cameraObject = scene.AddGameObject();
    if (ProjectManager::projectData.is3D)
    {
        cameraObject->transform.SetPosition({ 0,0, -25 });
        cameraObject->transform.SetRotation(Quaternion::Identity());
    }
    else
    {
        cameraObject->transform.SetPosition({ 0,0,0 });
        cameraObject->transform.SetRotationEuler({ 180, 0, 0 });
    }
    cameraObject->transform.SetScale({ 1,1,1 });
    cameraObject->SetName("Camera");
    CameraComponent* camera = cameraObject->AddComponent<CameraComponent>();
    camera->gameObject = cameraObject;
#endif

    if (!std::filesystem::exists(path.parent_path()))
        std::filesystem::create_directories(path.parent_path());
        std::filesystem::create_directories(path.parent_path());

    SaveScene(&scene);

    // Only removing game objects here since the scene will be deleted once out of scope
    for (GameObject* gameObject : scene.GetGameObjects())
        scene.RemoveGameObject(gameObject);
}

void SceneManager::ResetScene(Scene* scene)
{
    //scene->GetGameObjects().clear();
    //for (GameObject obj : scene->gameObjectsBackup)
    //{
    //    scene->AddGameObject(obj);
    //}
}

void SceneManager::BackupScene(Scene* scene) 
{
    //scene->gameObjectsBackup.clear();
    //for (GameObject obj : scene->GetGameObjects())
    //{
    //    scene->gameObjectsBackup.push_back(obj);
    //}
}

// Todo:: Add Backup Cleanup function


GameObject* SpawnGameObject(std::string path, Vector3 position, Quaternion rotation)
{
    return SceneManager::GetActiveScene()->SpawnObject(path, position, rotation);
}

static GameObject* SpawnGameObject(std::string path)
{
    return SceneManager::GetActiveScene()->SpawnObject(path);
}

static GameObject* SpawnGameObject(std::string path, Vector3 position)
{
    return SceneManager::GetActiveScene()->SpawnObject(path, position);
}

static GameObject* SpawnGameObject(std::string path, Vector3 position, Vector3 rotation)
{
    return SceneManager::GetActiveScene()->SpawnObject(path, position, rotation);
}

static GameObject* SpawnGameObject(GameObject* gameObject, Vector3 position, Quaternion rotation)
{
    return SceneManager::GetActiveScene()->SpawnObject(gameObject, position, rotation);
}

static GameObject* SpawnGameObject(GameObject* gameObject)
{
    return SceneManager::GetActiveScene()->SpawnObject(gameObject);
}

static GameObject* SpawnGameObject(GameObject* gameObject, Vector3 position)
{
    return SceneManager::GetActiveScene()->SpawnObject(gameObject, position);
}

static GameObject* SpawnGameObject(GameObject* gameObject, Vector3 position, Vector3 rotation)
{
    return SceneManager::GetActiveScene()->SpawnObject(gameObject, position, rotation);
}

static GameObject* SpawnObject(std::string path)
{
    return SceneManager::GetActiveScene()->SpawnObject(path);
}

static GameObject* SpawnObject(std::string path, Vector3 position)
{
    return SceneManager::GetActiveScene()->SpawnObject(path, position);
}

static GameObject* SpawnObject(std::string path, Vector3 position, Quaternion rotation)
{
    return SceneManager::GetActiveScene()->SpawnObject(path, position, rotation);
}

static GameObject* SpawnObject(std::string path, Vector3 position, Vector3 eulerRotation)
{
    return SceneManager::GetActiveScene()->SpawnObject(path, position, eulerRotation);
}

static GameObject* SpawnObject(GameObject* gameObject)
{
    return SceneManager::GetActiveScene()->SpawnObject(gameObject);
}

static GameObject* SpawnObject(GameObject* gameObject, Vector3 position)
{
    return SceneManager::GetActiveScene()->SpawnObject(gameObject, position);
}

static GameObject* SpawnObject(GameObject* gameObject, Vector3 position, Quaternion rotation)
{
    return SceneManager::GetActiveScene()->SpawnObject(gameObject, position, rotation);
}

static GameObject* SpawnObject(GameObject* gameObject, Vector3 position, Vector3 eulerRotation)
{
    return SceneManager::GetActiveScene()->SpawnObject(gameObject, position, eulerRotation);
}

void Destroy(GameObject* gameObject)
{
    SceneManager::GetActiveScene()->RemoveGameObject(gameObject);
}
void Destroy(Component* component)
{
    component->gameObject->RemoveComponent(component);
}