#include "Scene.h"
#include <algorithm>
#include "../Components/Component.h"
#include "../Components/SpriteRenderer.h"
#if defined(EDITOR)
#include "../ProjectManager.h"
#else
#include "../RaylibWrapper.h" 
#include "../Game.h"
#endif
#include "../Components/MeshRenderer.h"

Scene::Scene(const std::filesystem::path& path, std::deque<GameObject*> gameObjects)
    : m_Path(path), m_GameObjects(gameObjects)
{
}

Scene::~Scene()
{
}

std::filesystem::path Scene::GetPath()
{
    return m_Path;
}

void Scene::SetPath(std::filesystem::path path)
{
    m_Path = path;
}

// Todo: Internal only
GameObject* Scene::AddGameObject(int id)
{
    m_GameObjects.push_back(new GameObject(id));

    return m_GameObjects.back();
}

//void Scene::RemoveGameObject(GameObject* gameObject)
//{
//    auto it = std::find_if(m_GameObjects.begin(), m_GameObjects.end(),
//        [gameObject](const GameObject& go) { return go.GetId() == gameObject->GetId(); });
//    if (it != m_GameObjects.end()) {
//        //delete& (*it);
//        m_GameObjects.erase(it);
//
//    }
//}

void Scene::RemoveGameObject(GameObject* gameObject)
{
    if (!gameObject) // In case the gameObject has already been removed
        return;

    if (GameObject::markForDeletion)
    {
        GameObject::markedForDeletion.push_back(gameObject);
        return;
    }

    std::deque<GameObject*> childrenCopy = gameObject->GetChildren();
    for (GameObject* child : childrenCopy)
        RemoveGameObject(child);

    for (Component* component : gameObject->GetComponents())
        gameObject->RemoveComponent(component);

    if (gameObject->GetParent() != nullptr)
        gameObject->SetParent(nullptr);

    auto it = std::find_if(m_GameObjects.begin(), m_GameObjects.end(),
        [gameObject](const GameObject* go) { return go->GetId() == gameObject->GetId(); });
    if (it != m_GameObjects.end()) {
        delete *it;
        m_GameObjects.erase(it);
    }
}

// An alias for RemoveGameObject()
void Scene::DestroyGameObject(GameObject* gameObject)
{
    RemoveGameObject(gameObject);
}

// An alias for RemoveGameObject()
void Scene::Destroy(GameObject* gameObject)
{
    RemoveGameObject(gameObject);
}


std::deque<GameObject*>& Scene::GetGameObjects()
{
    return m_GameObjects;
}

GameObject* Scene::GetGameObject(const std::string& name)
{
    for (GameObject* gameObject : m_GameObjects)
        if (gameObject->GetName() == name)
            return gameObject;
    return nullptr;
}

GameObject* Scene::SpawnGameObject(std::string path, Vector3 position, Quaternion rotation)
{
    // Todo: Add Asset Template/Prefab support for SpawnGameObject
    // Todo: Add support for parents

    std::filesystem::path newPath = path;
#if !defined(EDITOR)
    if (exeParent.empty())
        newPath = "Resources/Assets" / newPath;
    else
        newPath = std::filesystem::path(exeParent) / "Resources" / "Assets" / newPath;
#else
    newPath = ProjectManager::projectData.path / "Assets" / newPath;
#endif

    int type = 0; // 0 = image, 1 = model, 2 = primitive model

    if (path == "Cube" || path == "Plane" || path == "Sphere" || path == "Cylinder" || path == "Cone")
    {
        type = 2;
        newPath = path;
    }

    if (type != 2 && !newPath.has_extension())
    {
        ConsoleLogger::WarningLog("Failed to spawn a game object. The specified file must be a PNG, JPG, JPEG, GLB, GLTF, OBJ, or M3D from a relative path. specified path: " + newPath.string());
        return nullptr;
    }

    if (type != 2)
    {
        if (newPath.extension() == ".png" || newPath.extension() == ".jpg" || newPath.extension() == ".jpeg")
            type = 0;
        else if (newPath.extension() == ".glb" || newPath.extension() == ".gltf" || newPath.extension() == ".obj" || newPath.extension() == ".m3d")
            type = 1;

        if (!std::filesystem::exists(newPath))
        {
            if (type == 0)
                ConsoleLogger::WarningLog("Failed to spawn a game object. Invalid sprite path. specified path: " + newPath.string());
            else if (type == 1)
                ConsoleLogger::WarningLog("Failed to spawn a game object. Invalid model path. specified path: " + newPath.string());

            return nullptr;
        }
    }

    GameObject* gameObject = AddGameObject();
    gameObject->transform.SetPosition(position);
    gameObject->transform.SetScale({ 1,1,1 });
    gameObject->transform.SetRotation(rotation);
    gameObject->SetName(newPath.stem().string());

    if (type == 0)
    {
        SpriteRenderer* spriteRenderer = gameObject->AddComponent<SpriteRenderer>();
        spriteRenderer->SetSprite(new Sprite(path)); // Using path here since the Sprite constructor takes in a relative path
        spriteRenderer->gameObject = gameObject;
    }
    else if (type == 1)
    {
        MeshRenderer* component = gameObject->AddComponent<MeshRenderer>();
        component->SetModelPath(newPath);

        if (component->GetModelPath().string() == "Cube")
            component->SetModel(Cube, component->GetModelPath().string(), ShaderManager::LitStandard);
        else if (component->GetModelPath().string() == "Plane")
            component->SetModel(Plane, component->GetModelPath().string(), ShaderManager::LitStandard);
        else if (component->GetModelPath().string() == "Sphere")
            component->SetModel(Sphere, component->GetModelPath().string(), ShaderManager::LitStandard);
        else if (component->GetModelPath().string() == "Cylinder")
            component->SetModel(Cylinder, component->GetModelPath().string(), ShaderManager::LitStandard);
        else if (component->GetModelPath().string() == "Cone")
            component->SetModel(Cone, component->GetModelPath().string(), ShaderManager::LitStandard);
        else
            component->SetModel(Custom, component->GetModelPath().string(), ShaderManager::LitStandard);
    }

    return gameObject;
}

GameObject* Scene::SpawnGameObject(std::string path)
{
    return SpawnGameObject(path, { 0,0,0 }, Quaternion::Identity());
}

GameObject* Scene::SpawnGameObject(std::string path, Vector3 position)
{
    return SpawnGameObject(path, position, Quaternion::Identity());

}

GameObject* Scene::SpawnGameObject(std::string path, Vector3 position, Vector3 rotation)
{
    return SpawnGameObject(path, position, EulerToQuaternion(rotation.x * DEG2RAD, rotation.y * DEG2RAD, rotation.z * DEG2RAD));
}

GameObject* Scene::SpawnGameObject(GameObject* gameObject, Vector3 position, Quaternion rotation) // INCOMPLETE
{
    if (gameObject == nullptr)
    {
        ConsoleLogger::WarningLog("Failed to spawn a game object. Invalid Game Object to clone.");
        return nullptr;
    }

    GameObject* clonedGameObject = AddGameObject();
    clonedGameObject->transform.SetPosition(position);
    clonedGameObject->transform.SetScale(gameObject->transform.GetScale());
    clonedGameObject->transform.SetRotation(rotation);
    clonedGameObject->SetName(gameObject->GetName());

    // Todo: Clone children & set the parent of the children & reset the current children list and add these children to it. Just use SpawnGameObject() to clone children

    for (auto& component : gameObject->GetComponents())
    {
        // Todo: if an exposed variable or variable is set to a gameobject or component on this gameobject or a child, then set it to the cloned one
        // Todo: This will cause issues if it tries to call Clone() on an external script.
        // Todo: Check who owns the new cloned component, I think the original Component might

        // Todo: Inside of each of the Clone() functions it needs to set the variables, but I need to figure out how to do that while letting components vector own the Component

        clonedGameObject->AddClonedComponent(component);

        // Make this work in builds. So don't clone custom components, just internal & ScriptComponent. Then run the SetupScriptComponent()
        //  It may be best just to Add Clone() to custom components when building them. BuildScripts() in ScriptLoader() does make sense to use to put it in, but ExposedVariables already has most of the code.
        // Only need to set exposed variables
        // Make sure to call SetExposedVariables if not in Editor
        // Get component and set gameObject
    }

    //for (Component* component : SceneManager::GetActiveScene()->GetGameObjects().back()->GetComponents())
    //    component->gameObject = SceneManager::GetActiveScene()->GetGameObjects().back();

    return clonedGameObject;
}

GameObject* Scene::SpawnGameObject(GameObject* gameObject)
{
    return SpawnGameObject(gameObject, gameObject->transform.GetPosition(), gameObject->transform.GetRotation());
}

GameObject* Scene::SpawnGameObject(GameObject* gameObject, Vector3 position)
{
    return SpawnGameObject(gameObject, position, gameObject->transform.GetRotation());
}

GameObject* Scene::SpawnGameObject(GameObject* gameObject, Vector3 position, Vector3 rotation)
{
    return SpawnGameObject(gameObject, position, EulerToQuaternion(rotation.x * DEG2RAD, rotation.y * DEG2RAD, rotation.z * DEG2RAD));
}

GameObject* Scene::SpawnObject(std::string path)
{
    return SpawnGameObject(path);
}

GameObject* Scene::SpawnObject(std::string path, Vector3 position)
{
    return SpawnGameObject(path, position);
}

GameObject* Scene::SpawnObject(std::string path, Vector3 position, Quaternion rotation)
{
    return SpawnGameObject(path, position, rotation);
}

GameObject* Scene::SpawnObject(std::string path, Vector3 position, Vector3 eulerRotation)
{
    return SpawnGameObject(path, position, eulerRotation);
}

GameObject* Scene::SpawnObject(GameObject* gameObject)
{
    return SpawnGameObject(gameObject);
}

GameObject* Scene::SpawnObject(GameObject* gameObject, Vector3 position)
{
    return SpawnGameObject(gameObject, position);
}

GameObject* Scene::SpawnObject(GameObject* gameObject, Vector3 position, Quaternion rotation)
{
    return SpawnGameObject(gameObject, position, rotation);
}

GameObject* Scene::SpawnObject(GameObject* gameObject, Vector3 position, Vector3 eulerRotation)
{
    return SpawnGameObject(gameObject, position, eulerRotation);
}