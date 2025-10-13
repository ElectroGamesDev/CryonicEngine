#pragma once

#include "CryonicCore.h"
#include <string>
#include <vector>
#include <deque>
#include "ConsoleLogger.h"
#include <filesystem>
#include <memory>
#include <algorithm>

class Component;

class GameObject
{
public:
    GameObject(int id = 0);
    ~GameObject();

    //Model GetModel() const;
    //void SetModel(Model model);
    //std::filesystem::path GetModelPath() const;
    //void SetModelPath(std::filesystem::path path);
    //BoundingBox GetBounds() const;
    //void SetBounds(BoundingBox bounds);
    //Material GetMaterial() const;
    //void SetMaterial(Material material);
    //std::string GetPath() const;
    //void SetPath(std::string path);
    std::string GetName() const;
    void SetName(std::string name);
    //Vector3 GetPosition() const;
    //void SetPosition(Vector3 position);
    //Vector3 GetRealSize() const;
    //Vector3 GetSize() const;
    //void SetSize(Vector3 size);
    //void SetRealSize(Vector3 realSize);
    //Quaternion GetRotation() const;
    //void SetRotation(Quaternion rotation);
    //Vector3 GetRotationEuler();
    //void SetRotationEuler();
    void SetActive(bool active);
    bool IsActive() const;
    // Hide in API
    void SetGlobalActive(bool active);
    // Hide in API
    bool IsGlobalActive() const;
    int GetId() const;
    // Hide In API
    bool active = true; // local state
    // Hide In API
    bool globalActive = true; // parent's state

    void SetParent(GameObject* gameObject);
    GameObject* GetParent();

    GameObject* GetChild(int index);
    GameObject* FindChild(std::string name);

    std::deque<GameObject*>& GetChildren();

    void SetSiblingIndex(int index);
    int GetSiblingIndex();

    std::deque<GameObject*>& GetSiblings();

    bool IsChild(GameObject& gameObject, GameObject* parent = nullptr);

    // These two functions are used by AddComponent() and AddInternalComponent() since these need to be implemented in the GaemObject.cpp
    // Hide in API
    bool IsComponentValid(Component* component);
    // Hide in API
    void SetComponentGameObject(Component* component);

    template <typename T>
    T* AddComponent() {
        T* newComponent = new T(this, -1);
        if (!IsComponentValid(static_cast<Component*>(newComponent)))
        {
            delete newComponent;
            return nullptr;
        }
        SetComponentGameObject(static_cast<Component*>(newComponent)); // Todo: This may cause a crash if its not a component
        //Component* componentPtr = static_cast<Component*>(newComponent);
        //if (componentPtr)
        //    componentPtr->gameObject = this;
        //else
        //    return nullptr;
        components.push_back(newComponent);

        newComponent->SetExposedVariables();
        newComponent->initialized = true;
        // Todo: if the any of the statements below return false, then the remaining would be false too
#if !defined(EDITOR)
        if (newComponent->IsActive() && IsActive() && IsGlobalActive())
        {
            newComponent->Awake();
            newComponent->awakeCalled = true;
        }
        if (newComponent->IsActive() && IsActive() && IsGlobalActive())
            newComponent->Enable();
        if (newComponent->IsActive() && IsActive() && IsGlobalActive())
        {
            newComponent->Start();
            newComponent->startCalled = true;
        }
#endif
        return newComponent;
    }

    // Hide in API
    // Adds a component to a game object without calling Awake(), Enable(), Start(), SetExposedVariables(), and setting intitialized to true. It also ignores the valid flag
    template <typename T>
    T& AddComponentInternal(int id = -1) {
        T* newComponent = new T(this, id);
        SetComponentGameObject(static_cast<Component*>(newComponent)); // Todo: This may cause a crash if its not a component
        components.push_back(newComponent);
        return *newComponent;
    }

    // Hide in API
    template <typename T>
    T& AddComponent(int id) {
        T* newComponent = new T(this, id);
        SetComponentGameObject(static_cast<Component*>(newComponent)); // Todo: This may cause a crash if its not a component
        components.push_back(newComponent);
        return *newComponent;
    }

    // Hide in API
    void AddClonedComponent(Component* component) {
        //components.push_back(component->Clone()); // This is causing a compile errors
        //components.back()->exposedVariables = component->exposedVariables;
        //components.back()->gameObject = this;
    }

    template<typename T>
    bool RemoveComponent();

    bool RemoveComponent(Component* component);

    void Destroy();
    void Destroy(Component* component);

    template<typename T>
    T* GetComponent()
    {
        for (Component* comp : components)
        {
            T* tcomp = dynamic_cast<T*>(comp);
            if (tcomp != nullptr)
            {
                return tcomp;
            }
        }
        return nullptr;
    }


    std::vector<Component*> GetComponents();
    //GameObject& operator=(const GameObject& other);
    bool operator==(const GameObject& other) const;
    bool operator!=(const GameObject& other) const;

    struct Transform
    {
        void SetPosition(Vector3 position)
        {
            for (GameObject* child : gameObject->childGameObjects)
                child->transform.SetPosition(child->transform.GetLocalPosition() + position);

            _position = position;
        }
        void SetPosition(Vector2 position) { SetPosition({position.x, position.y, _position.z}); }
        void SetPosition(float x, float y, float z) { SetPosition({ x, y, z }); }
        void SetPosition(float x, float y) { SetPosition({ x, y,  _position.z }); }
        Vector3 GetPosition() { return _position; }

        void SetLocalPosition(Vector3 position) { gameObject->parentGameObject == nullptr ? SetPosition(position) : SetPosition(gameObject->parentGameObject->transform._position + position); }
        void SetLocalPosition(Vector2 position) { SetLocalPosition({ position.x, position.y, _position.z }); }
        void SetLocalPosition(float x, float y, float z) { SetLocalPosition({ x, y, z }); }
        void SetLocalPosition(float x, float y) { SetLocalPosition({ x, y,  _position.z }); }
        Vector3 GetLocalPosition() { return gameObject->parentGameObject == nullptr ? _position : (_position - gameObject->parentGameObject->transform._position); }

        void MovePosition(Vector3 displacement) { SetPosition(GetPosition() + displacement); };
        void MovePosition(Vector2 displacement) { SetPosition(Vector2(GetPosition().x + displacement.x, GetPosition().y + displacement.y)); };
        void MovePosition(float x, float y) { MovePosition(Vector2{ x, y }); };

        void MoveLocalPosition(Vector3 displacement) { SetLocalPosition(GetLocalPosition() + displacement); }
        void MoveLocalPosition(Vector2 displacement) { SetLocalPosition(Vector2(GetLocalPosition().x + displacement.x, GetLocalPosition().y + displacement.y)); };
        void MoveLocalPosition(float x, float y) { MoveLocalPosition(Vector2(x, y)); };

        void SetRotation(Quaternion rotation)
        {
#ifdef EDITOR
            eulerRotation = QuaternionToEuler(rotation) * RAD2DEG;
#endif
            _rotation = rotation;
        }
        Quaternion GetRotation() { return _rotation; }

        /**
        Set the game object's rotation in degrees
        */
        void SetRotationEuler(Vector3 rotation)
        {
#ifdef EDITOR
            eulerRotation = rotation;
            NormalizeEuler(eulerRotation); // Todo: Should this run when not in the editor too?
#endif
            for (GameObject* child : gameObject->childGameObjects)
                child->transform.SetRotationEuler(child->transform.GetLocalRotationEuler() + rotation);

            _rotation = EulerToQuaternion((float)rotation.x * DEG2RAD, rotation.y * DEG2RAD, rotation.z * DEG2RAD);
        }
        /**
        Set the game object's rotation in degrees
        */
        void SetRotationEuler(Vector2 rotation)
        {
#ifdef EDITOR
            SetRotationEuler({ rotation.x, rotation.y, eulerRotation.z });
#else
            SetRotationEuler({ rotation.x, rotation.y, QuaternionToEuler(_rotation).z * RAD2DEG });
#endif
        }
        /**
        Set the game object's rotation in degrees
        */
        void SetRotationEuler(float x, float y, float z) { SetRotationEuler({ x, y, z }); }
        /**
        Set the game object's rotation in degrees
        */
        void SetRotationEuler(float x, float y) { SetRotationEuler(Vector2(x, y)); }
        /**
        Get the game object's rotation in degrees
        @return Vector3 euler of the rotation
        */
        Vector3 GetRotationEuler()
        {
#ifdef EDITOR
            return eulerRotation;
#else
            return QuaternionToEuler(_rotation) * RAD2DEG;
#endif
        }

        /**
        Set the game object's local rotation in degrees
        */
        void SetLocalRotationEuler(Vector3 rotation)
        {
            gameObject->parentGameObject == nullptr
                ? SetRotationEuler(rotation)
                : SetRotationEuler({ gameObject->parentGameObject->transform.GetRotationEuler() + rotation });
        }
        /**
        Set the game object's local rotation in degrees
        */
        void SetLocalRotationEuler(Vector2 rotation)
        {
#ifdef EDITOR
            SetLocalRotationEuler( {rotation.x, rotation.y, eulerRotation.z} );
#else
            SetLocalRotationEuler({ rotation.x, rotation.y, QuaternionToEuler(_rotation).z * RAD2DEG });
#endif
        }
        /**
        Set the game object's local rotation in degrees
        */
        void SetLocalRotationEuler(float x, float y, float z) { SetLocalRotationEuler({ x, y, z }); }
        /**
        Set the game object's local rotation in degrees
        */
        void SetLocalRotationEuler(float x, float y) { SetLocalRotationEuler(Vector2(x, y)); }
        /**
        Get the game object's local rotation in degrees
        @return Vector3 euler of the rotation
        */
        Vector3 GetLocalRotationEuler()
        {
            return gameObject->parentGameObject == nullptr
                ? GetRotationEuler()
                : (GetRotationEuler() - gameObject->parentGameObject->transform.GetRotationEuler());
        }

        /**
        Rotates the game object by the specified Euler angles in degrees.
        */
        void Rotate(Vector3 euler)
        {
#ifdef EDITOR
            eulerRotation += euler;
            NormalizeEuler(eulerRotation);
#endif
            for (GameObject* child : gameObject->childGameObjects)
                child->transform.Rotate(euler);

            _rotation *= EulerToQuaternion(euler.x * DEG2RAD, euler.y * DEG2RAD, euler.z * DEG2RAD);
            //euler += QuaternionToEuler(_rotation);
            //_rotation = EulerToQuaternion(euler.x * DEG2RAD, euler.y * DEG2RAD, euler.z * DEG2RAD); // Todo: It would be best to convert euler to a quaternion, then add the two quaternions
        }

        /**
        Rotates the game object by the specified Euler angles in degrees.
        */
        void Rotate(Vector2 euler) { Rotate({ euler.x, euler.y, 0 }); }

        /**
        Rotates the game object by the specified Euler angles in degrees.
        */
        void Rotate(float x, float y, float z) { Rotate({ x, y, z }); }

        /**
        Rotates the game object by the specified Euler angles in degrees.
        */
        void Rotate(float x, float y) { Rotate(Vector2(x, y)); }


        void SetScale(Vector3 scale)
        { 
            for (GameObject* child : gameObject->childGameObjects)
                child->transform.SetScale(child->transform.GetLocalScale() * scale);

            _scale = scale;
        }
        void SetScale(Vector2 scale) { SetScale({ scale.x, scale.y, _scale.z }); }
        void SetScale(float x, float y, float z) { SetScale({ x, y, z }); }
        void SetScale(float x, float y) { SetScale({ x, y,  _scale.z }); }
        Vector3 GetScale() { return _scale; }

        void SetLocalScale(Vector3 scale) { gameObject->parentGameObject == nullptr ? SetScale(scale) : SetScale(gameObject->parentGameObject->transform._scale * scale); }
        void SetLocalScale(Vector2 scale) { SetLocalScale({ scale.x, scale.y, _scale.z }); }
        void SetLocalScale(float x, float y, float z) { SetLocalScale({ x, y, z }); }
        void SetLocalScale(float x, float y) { SetLocalScale({ x, y, _scale.z }); }
        Vector3 GetLocalScale() { return gameObject->parentGameObject == nullptr ? _scale : (_scale / gameObject->parentGameObject->transform._scale); }

        Transform& operator=(const Transform& other) {
            if (this != &other) {
                _position = other._position;
                _rotation = other._rotation;
                _scale = other._scale;
            }
            return *this;
        }

        bool operator==(const Transform& other) const {
            return (_position.x == other._position.x &&
                _position.y == other._position.y &&
                _position.z == other._position.z &&
                _rotation.x == other._rotation.x &&
                _rotation.y == other._rotation.y &&
                _rotation.z == other._rotation.z &&
                _rotation.w == other._rotation.w &&
                _scale.x == other._scale.x &&
                _scale.y == other._scale.y &&
                _scale.z == other._scale.z);
        }

        bool operator!=(const Transform& other) const {
            return !(*this == other);
        }

        GameObject* gameObject;

    private:
        Vector3 _position = { 0,0,0 };
        Quaternion _rotation = Quaternion::Identity();
        Vector3 eulerRotation = { 0,0,0 }; // Used only in the editor
        Vector3 _scale = { 1,1,1 };
    };
    Transform transform;

    // Hide in API
    static std::vector<GameObject*> markedForDeletion;
    // Hide in API
    static bool markForDeletion;

private:
    //Model model;
    //std::filesystem::path modelPath;
    //BoundingBox bounds;
    //Material material;

    std::string name;
    int id;
    std::vector<Component*> components;
    GameObject* parentGameObject = nullptr;
    std::deque<GameObject*> childGameObjects;
};