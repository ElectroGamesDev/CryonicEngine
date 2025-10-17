#include "Core/GameObject.h"
#include "Systems/Scene/SceneManager.h"
#include "Utilities/ConsoleLogger.h"
#include <cstdlib>
#include <ctime>
#include "Components/Component.h"

std::vector<GameObject*> GameObject::markedForDeletion;
bool GameObject::markForDeletion = false;

GameObject::GameObject(int id)
{
    //this->model = model;
    //this->modelPath = modelPath;
    //this->bounds = bounds;
    //this->active = active;
    //this->name = name;
    transform.gameObject = this;
    this->id = id;
    static bool seeded = false;
    if (!seeded)
    {
        srand(static_cast<unsigned int>(time(0)));
        seeded = true;
    }

    if (id == 0)
        this->id = 100000 + rand() % 900000; // Todo: Make sure this is always unique
}

bool GameObject::IsComponentValid(Component* component)
{
    return component->valid;
}

void GameObject::SetComponentGameObject(Component* component)
{
    component->gameObject = this;
}

//Model GameObject::GetModel() const
//{
//    return model;
//}
//
//void GameObject::SetModel(Model model)
//{
//    this->model = model;
//    this->bounds = GetMeshBoundingBox(model.meshes[0]); // Todo: Set the correct scale depending on size variable.
//}
//
//std::filesystem::path GameObject::GetModelPath() const
//{
//    return modelPath;
//}
//
//void GameObject::SetModelPath(std::filesystem::path path)
//{
//    this->modelPath = path;
//}
//
//BoundingBox GameObject::GetBounds() const
//{
//    return bounds;
//}
//
//void GameObject::SetBounds(BoundingBox bounds)
//{
//    this->bounds = bounds;
//}

bool GameObject::IsActive() const
{
    return active;
}

void GameObject::SetActive(bool active)
{
    if (active == this->active)
        return;
    this->active = active;

#if !defined(EDITOR)
    if (IsGlobalActive())
    {
        for (Component* component : components)
        {
            if (component->IsActive() && component->initialized)
            {
                if (active)
                {
                    if (!component->awakeCalled)
                    {
                        component->awakeCalled = true;
                        component->Awake();
                    }
                    component->Enable();
                    if (!component->startCalled)
                    {
                        component->startCalled = true;
                        component->Start();
                    }
                }
                else
                    component->Disable();
            }
        }
    }
#endif

    for (GameObject* child : childGameObjects)
        child->SetGlobalActive(active);
}

// Hide in API
void GameObject::SetGlobalActive(bool globalActive)
{
    if (globalActive == this->globalActive)
        return;
    this->globalActive = globalActive;

#if !defined(EDITOR)
    for (Component* component : components)
    {
        if (component->IsActive() && component->initialized)
        {
            if (globalActive)
            {
                if (!component->awakeCalled)
                {
                    component->awakeCalled = true;
                    component->Awake();
                }
                component->Enable();
                if (!component->startCalled)
                {
                    component->startCalled = true;
                    component->Start();
                }
            }
            else
                component->Disable();
        }
    }
#endif

    for (GameObject* child : childGameObjects)
        child->SetGlobalActive(globalActive);
};

bool GameObject::IsGlobalActive() const
{
    return globalActive;
}

//Material GameObject::GetMaterial() const
//{
//    return material;
//}
//
//void GameObject::SetMaterial(Material material)
//{
//    this->material = material;
//}

//std::string GameObject::GetPath() const
//{
//    return path;
//}
//
//void GameObject::SetPath(std::string path)
//{
//    this->path = path;
//}

std::string GameObject::GetName() const
{
    return name;
}

void GameObject::SetName(std::string name)
{
    this->name = name;
}


int GameObject::GetId() const
{
    return id;
}

//template<typename T>
//T& GameObject::AddComponent()
//{
//    T* newComponent = new T();
//    components.push_back(newComponent);
//    return newComponent;
//}

//template<typename T>
//bool GameObject::RemoveComponent()
//{
//    for (size_t i = 0; i < components.size(); ++i)
//    {
//        T* comp = dynamic_cast<T*>(components[i]);
//        if (comp != nullptr)
//        {
//            delete comp;
//            components.erase(components.begin() + i);
//            return true;
//        }
//    }
//    return false;
//}
//
//template<typename T>
//T& GameObject::GetComponent()
//{
//    for (Component* comp : components)
//    {
//        T* tcomp = dynamic_cast<T*>(comp);
//        if (tcomp != nullptr)
//        {
//            return tcomp;
//        }
//    }
//    return nullptr;
//}

template<typename T>
bool GameObject::RemoveComponent()
{

    for (auto it = components.begin(); it != components.end(); ++it)
    {
        T* component = dynamic_cast<T*>(*it);
        if (component != nullptr)
        {
            if (markForDeletion)
            {
                Component::markedForDeletion.push_back(component);
                return true;
            }

            component->Disable();
            component->Destroy();
            delete component;
            components.erase(it);
            return true;
        }
    }
    return false;
}

template bool GameObject::RemoveComponent<Component>();


bool GameObject::RemoveComponent(Component* component)
{
    if (markForDeletion)
    {
        Component::markedForDeletion.push_back(component);
        return true;
    }
    auto it = std::find(components.begin(), components.end(), component);
    if (it != components.end())
    {
        component->Disable();
        component->Destroy();
        delete* it;
        components.erase(it);
        return true;
    }
    return false;
}

void GameObject::Destroy()
{
    SceneManager::GetActiveScene()->RemoveGameObject(this); // Todo: This assumes the game object is in the active scene.
}

// An alias for RemoveComponent()
void GameObject::Destroy(Component* component)
{
    RemoveComponent(component);
}

std::vector<Component*> GameObject::GetComponents()
{
    return components;
}

bool GameObject::operator==(const GameObject& other) const
{
    return this->id == other.id;
}

bool GameObject::operator!=(const GameObject& other) const
{
    return this->id != other.id;
}

//GameObject& GameObject::operator=(const GameObject& other) {
//    if (this != &other) {
//        name = other.name;
//        id = other.id;
//        transform = other.transform;
//    }
//    return *this;
//}

GameObject::~GameObject()
{

}

bool GameObject::IsChild(GameObject& gameObject, GameObject* parent)
{
    if (parent == nullptr)
        parent = &gameObject;

    if (std::find(childGameObjects.begin(), childGameObjects.end(), parent) != childGameObjects.end())
        return true;

    for (GameObject* child : childGameObjects)
        if (child->IsChild(*parent))
            return true;

    return false;

}

void GameObject::SetParent(GameObject* gameObject)
{
    // Todo: Set Local and Global position/rotation/scale

    if (parentGameObject != nullptr && gameObject != nullptr && gameObject->GetId() == parentGameObject->GetId())
        return;

    if (parentGameObject != nullptr)
        parentGameObject->childGameObjects.erase(std::remove(parentGameObject->childGameObjects.begin(), parentGameObject->childGameObjects.end(), this), parentGameObject->childGameObjects.end());

    if (gameObject != nullptr && childGameObjects.size() > 0 && IsChild(*gameObject))
        gameObject->SetParent(parentGameObject);

    parentGameObject = gameObject;
    if (parentGameObject != nullptr) // If its nullptr, then the game object will become a root game object.
    {
        parentGameObject->childGameObjects.push_back(this);
        if (!parentGameObject->active || !parentGameObject->globalActive)
            SetGlobalActive(false);
    }
    else if (!IsGlobalActive())
        SetGlobalActive(true);
}

GameObject* GameObject::GetParent()
{
    return parentGameObject;
}

GameObject* GameObject::GetChild(int index)
{
    if (index < 0 || index >= childGameObjects.size())
    {
        ConsoleLogger::WarningLog("Unknown user script called GetChild() with an out of bounds index. The index must be less than the child count. Index: " + std::to_string(index) + ", Number of children: " + std::to_string(childGameObjects.size()), false);
        return nullptr;
    }
    else
        return childGameObjects[index];
}

GameObject* GameObject::FindChild(std::string name)
{
    return nullptr;
}

std::deque<GameObject*>& GameObject::GetChildren()
{
    return childGameObjects;
}

void GameObject::SetSiblingIndex(int index)
{
    // Todo: Add this
}

int GameObject::GetSiblingIndex()
{
    // Todo: Add this
    return 0;
}

std::deque<GameObject*>& GameObject::GetSiblings()
{
    return parentGameObject->childGameObjects;
}
