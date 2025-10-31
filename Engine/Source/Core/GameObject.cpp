#include "Core/GameObject.h"
#include "Systems/Scene/SceneManager.h"
#include "Utilities/ConsoleLogger.h"
#include <cstdlib>
#include <ctime>
#include "Components/Component.h"
#include <iostream>

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

void GameObject::SetParent(GameObject* newParent)
{
	std::cout << "\n=== CALLING SetParent ===" << std::endl;
	std::cout << "Child ID: " << id << std::endl;
	std::cout << "New Parent ID: " << (newParent ? newParent->id : -1) << std::endl;

	if (newParent == parentGameObject)
		return;

	Vector3 worldPos = transform.GetPosition();
	Quaternion worldRotQ = transform.GetRotation();
	Vector3 worldScale = transform.GetScale();

	std::cout << "Child world pos (before reparent): " << worldPos << std::endl;
	std::cout << "Child world rot (before reparent): " << worldRotQ << std::endl;
	std::cout << "Child world scale (before reparent): " << worldScale << std::endl;


	if (parentGameObject != nullptr)
	{
		auto& siblings = parentGameObject->childGameObjects;
		siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
	}

	parentGameObject = newParent;

	if (parentGameObject != nullptr)
		parentGameObject->childGameObjects.push_back(this);

	if (parentGameObject)
	{
		Transform& parentTransform = parentGameObject->transform;
		Vector3 parentPos = parentTransform.GetPosition();
		Quaternion parentRotQ = parentTransform.GetRotation();
		Vector3 parentScale = parentTransform.GetScale();

		Quaternion invParentRot = parentRotQ.Inverse();

		// Compute local position
		Vector3 offset = worldPos - parentPos;
		Vector3 localPos = offset * invParentRot;
		if (parentScale.x != 0) localPos.x /= parentScale.x;
		if (parentScale.y != 0) localPos.y /= parentScale.y;
		if (parentScale.z != 0) localPos.z /= parentScale.z;

		// Compute local rotation (quaternion)
		Quaternion localRotQ = invParentRot * worldRotQ;

		// Compute local euler from local quaternion
		Vector3 localEulerRot = QuaternionToEuler(localRotQ) * RAD2DEG;
		NormalizeEuler(localEulerRot);

		// Compute local scale
		Vector3 localScale;
		if (parentScale.x != 0) localScale.x = worldScale.x / parentScale.x;
		else localScale.x = 0;
		if (parentScale.y != 0) localScale.y = worldScale.y / parentScale.y;
		else localScale.y = 0;
		if (parentScale.z != 0) localScale.z = worldScale.z / parentScale.z;
		else localScale.z = 0;

		transform._localPosition = localPos;
		transform._position = worldPos;
		transform._localRotation = localRotQ;
		transform._rotation = worldRotQ;
		transform._localEulerRotation = localEulerRot;
#ifdef EDITOR
		transform._eulerRotation = QuaternionToEuler(worldRotQ) * RAD2DEG;
		NormalizeEuler(transform._eulerRotation);
#endif
		transform._localScale = localScale;
		transform._scale = worldScale;
	}
	else
	{
		transform._localPosition = worldPos;
		transform._position = worldPos;
		transform._localRotation = worldRotQ;
		transform._rotation = worldRotQ;
		transform._localEulerRotation = QuaternionToEuler(worldRotQ) * RAD2DEG;
		NormalizeEuler(transform._localEulerRotation);
#ifdef EDITOR
		transform._eulerRotation = transform._localEulerRotation;
#endif
		transform._localScale = worldScale;
		transform._scale = worldScale;
	}
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
    for (GameObject* child : childGameObjects)
    {
        if (child->GetName() == name)
            return child;
    }

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
