#pragma once

#include "CryonicCore.h"
#include <string>
#include <vector>
#include <deque>
#include "Utilities/ConsoleLogger.h"
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
			Vector3 delta = position - _position;
			_position = position;

			for (GameObject* child : gameObject->childGameObjects)
				child->transform.SetPosition(child->transform.GetPosition() + delta);
		}
        void SetPosition(Vector2 position) { SetPosition({position.x, position.y, _position.z}); }
        void SetPosition(float x, float y, float z) { SetPosition({ x, y, z }); }
        void SetPosition(float x, float y) { SetPosition({ x, y,  _position.z }); }
        Vector3 GetPosition() { return _position; }

		void SetLocalPosition(Vector3 position)
        {
			if (gameObject->parentGameObject == nullptr)
            {
				SetPosition(position);
				return;
			}

			Vector3 parentScale = gameObject->parentGameObject->transform.GetScale();
			Vector3 scaled;
			scaled.x = position.x * parentScale.x;
			scaled.y = position.y * parentScale.y;
			scaled.z = position.z * parentScale.z;
			Vector3 worldOffset = scaled * gameObject->parentGameObject->transform.GetRotation();
			SetPosition(gameObject->parentGameObject->transform.GetPosition() + worldOffset);
		}

		void SetLocalPosition(Vector2 position)
        {
			float preservedZ = GetLocalPosition().z;
			SetLocalPosition({ position.x, position.y, preservedZ });
		}
        void SetLocalPosition(float x, float y, float z) { SetLocalPosition({ x, y, z }); }
        void SetLocalPosition(float x, float y) { SetLocalPosition({ x, y,  GetLocalPosition().z }); }

		Vector3 GetLocalPosition()
        {
			if (gameObject->parentGameObject == nullptr)
				return _position;

			Vector3 offset = _position - gameObject->parentGameObject->transform.GetPosition();
			Vector3 localOffset = offset * gameObject->parentGameObject->transform.GetRotation().Inverse();
			Vector3 parentScale = gameObject->parentGameObject->transform.GetScale();
			if (parentScale.x == 0 || parentScale.y == 0 || parentScale.z == 0)
				return { 0, 0, 0 };

			localOffset.x /= parentScale.x;
			localOffset.y /= parentScale.y;
			localOffset.z /= parentScale.z;
			return localOffset;
		}

        void MovePosition(Vector3 displacement) { SetPosition(GetPosition() + displacement); };
        void MovePosition(Vector2 displacement) { SetPosition(Vector2(GetPosition().x + displacement.x, GetPosition().y + displacement.y)); };
        void MovePosition(float x, float y) { MovePosition(Vector2{ x, y }); };

        void MoveLocalPosition(Vector3 displacement) { SetLocalPosition(GetLocalPosition() + displacement); }
		void MoveLocalPosition(Vector2 displacement)
        {
			Vector3 localPos = GetLocalPosition();
			SetLocalPosition({ localPos.x + displacement.x, localPos.y + displacement.y, localPos.z });
		}
        void MoveLocalPosition(float x, float y) { MoveLocalPosition(Vector2(x, y)); };

        /**
        Set the game object's rotation with a quaternion
        */
		void SetRotation(Quaternion rotation)
        {
			Quaternion deltaRotation = rotation * _rotation.Inverse();
			_rotation = rotation;
#ifdef EDITOR
			eulerRotation = QuaternionToEuler(rotation) * RAD2DEG;
			NormalizeEuler(eulerRotation);
#endif

			for (GameObject* child : gameObject->childGameObjects)
				child->transform.ApplyWorldDeltaRotationAroundCenter(deltaRotation, _position);
		}

        Quaternion GetRotation() { return _rotation; }

		void SetLocalRotation(Quaternion quaternion)
        {
			if (gameObject->parentGameObject == nullptr)
				SetRotation(quaternion);
			else
            {
				Quaternion parentRot = gameObject->parentGameObject->transform.GetRotation();
				SetRotation(parentRot * quaternion);
			}
		}

        /**
        Set the game object's rotation in degrees
        */
		void SetRotationEuler(Vector3 rotation)
        {
			eulerRotation = rotation;
			NormalizeEuler(eulerRotation);

			Quaternion newRotation = EulerToQuaternion(
				eulerRotation.x * DEG2RAD,
				eulerRotation.y * DEG2RAD,
				eulerRotation.z * DEG2RAD
			);

			SetRotation(newRotation);
		}

        /**
        Set the game object's rotation in degrees
        */
        void SetRotationEuler(Vector2 rotation)
        {
            float preservedZ;
#ifdef EDITOR
            preservedZ = eulerRotation.z;
#else
            preservedZ = QuaternionToEuler(_rotation).z * RAD2DEG;
#endif
            SetRotationEuler({ rotation.x, rotation.y, preservedZ });
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
            NormalizeEuler(rotation);

			// Convert local Euler to quaternion
			Quaternion localQuat = EulerToQuaternion(
				rotation.x * DEG2RAD,
				rotation.y * DEG2RAD,
				rotation.z * DEG2RAD
			);

			if (gameObject->parentGameObject == nullptr)
				SetRotation(localQuat);
			else
            {
				Quaternion parentRot = gameObject->parentGameObject->transform.GetRotation();
				Quaternion newWorldRot = parentRot * localQuat;
				SetRotation(newWorldRot);
			}
		}
        /**
        Set the game object's local rotation in degrees
        */
        void SetLocalRotationEuler(Vector2 rotation)
        {
			float preservedZ = GetLocalRotationEuler().z;
			SetLocalRotationEuler({ rotation.x, rotation.y, preservedZ });
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
		Vector3 GetLocalRotationEuler() {
			Quaternion localQuat = GetLocalRotation();
			Vector3 localEuler = QuaternionToEuler(localQuat) * RAD2DEG;
			NormalizeEuler(localEuler);
			return localEuler;
		}

		Quaternion GetLocalRotation()
        {
			if (gameObject->parentGameObject == nullptr)
				return _rotation;
			else
            {
				Quaternion parentRot = gameObject->parentGameObject->transform.GetRotation();
				return parentRot.Inverse() * _rotation;
			}
		}

		void ApplyWorldDeltaRotationAroundCenter(Quaternion worldDelta, Vector3 center)
        {
			_rotation = worldDelta * _rotation; 
#ifdef EDITOR
			eulerRotation = QuaternionToEuler(_rotation) * RAD2DEG;
			NormalizeEuler(eulerRotation);
#endif
			Vector3 offset = _position - center;
			Vector3 rotatedOffset = offset * worldDelta;
			_position = center + rotatedOffset;

			for (GameObject* child : gameObject->childGameObjects)
				child->transform.ApplyWorldDeltaRotationAroundCenter(worldDelta, center);
		}

        /**
        Rotates the game object by the specified Euler angles in degrees.
        */
		void Rotate(Vector3 euler)
        {
            NormalizeEuler(euler);

			// Convert relative rotation to quaternion
			Quaternion deltaRot = EulerToQuaternion(
				euler.x * DEG2RAD,
				euler.y * DEG2RAD,
				euler.z * DEG2RAD
			);

			// Compute equivalent world delta for propagation (conjugate local delta)
			Quaternion worldDelta = _rotation * deltaRot * _rotation.Inverse();

			// Apply local delta to this rotation (post-multiply)
			_rotation = _rotation * deltaRot;

			// Update Euler cache accurately from new quat (avoids drift)
			eulerRotation = QuaternionToEuler(_rotation) * RAD2DEG;
			NormalizeEuler(eulerRotation);

			// Propagate to children recursively around this position (center)
			for (GameObject* child : gameObject->childGameObjects)
				child->transform.ApplyWorldDeltaRotationAroundCenter(worldDelta, _position);
		}

        /**
        Rotates the game object by the specified Euler angles in degrees.
        */
		void Rotate(Vector2 euler)
        {
			Rotate({ euler.x, euler.y, 0 });
		}
        /**
        Rotates the game object by the specified Euler angles in degrees.
        */
        void Rotate(float x, float y, float z) { Rotate({ x, y, z }); }

        /**
        Rotates the game object by the specified Euler angles in degrees.
        */
        void Rotate(float x, float y) { Rotate(Vector2(x, y)); }

		void ApplyWorldDeltaScaleAroundCenter(Vector3 factor, Vector3 center, Quaternion parentRot)
        {
			_scale.x *= factor.x;
			_scale.y *= factor.y;
			_scale.z *= factor.z;

			Vector3 offset = _position - center;
			Vector3 localOffset = offset * parentRot.Inverse();
			Vector3 scaledLocal;
			scaledLocal.x = localOffset.x * factor.x;
			scaledLocal.y = localOffset.y * factor.y;
			scaledLocal.z = localOffset.z * factor.z;
			Vector3 newOffset = scaledLocal * parentRot;
			_position = center + newOffset;

			for (GameObject* child : gameObject->childGameObjects)
				child->transform.ApplyWorldDeltaScaleAroundCenter(factor, center, parentRot);
		}

		void SetScale(Vector3 scale)
		{
			if (_scale.x == 0 || _scale.y == 0 || _scale.z == 0)
            {
				_scale = scale;
				return;
			}
			Vector3 factor;
			factor.x = scale.x / _scale.x;
			factor.y = scale.y / _scale.y;
			factor.z = scale.z / _scale.z;
			_scale = scale;

			for (GameObject* child : gameObject->childGameObjects)
				child->transform.ApplyWorldDeltaScaleAroundCenter(factor, _position, _rotation);
		}

		void SetScale(Vector2 scale) { SetScale({ scale.x, scale.y, _scale.z }); }
		void SetScale(float x, float y, float z) { SetScale({ x, y, z }); }
		void SetScale(float x, float y) { SetScale({ x, y, _scale.z }); }

        Vector3 GetScale() { return _scale; }

		void SetLocalScale(Vector3 scale)
        {
			if (gameObject->parentGameObject == nullptr)
				SetScale(scale);
			else
            {
				Vector3 parentScale = gameObject->parentGameObject->transform.GetScale();
				Vector3 worldScale;
				worldScale.x = parentScale.x * scale.x;
				worldScale.y = parentScale.y * scale.y;
				worldScale.z = parentScale.z * scale.z;
				SetScale(worldScale);
			}
		}

		void SetLocalScale(Vector2 scale) { SetLocalScale({ scale.x, scale.y, GetLocalScale().z }); }
		void SetLocalScale(float x, float y, float z) { SetLocalScale({ x, y, z }); }
		void SetLocalScale(float x, float y) { SetLocalScale({ x, y, GetLocalScale().z }); }

		Vector3 GetLocalScale()
        {
			if (gameObject->parentGameObject == nullptr)
				return _scale;

			Vector3 parentScale = gameObject->parentGameObject->transform.GetScale();
			if (parentScale.x == 0 || parentScale.y == 0 || parentScale.z == 0)
				return { 0, 0, 0 };

			Vector3 localScale;
			localScale.x = _scale.x / parentScale.x;
			localScale.y = _scale.y / parentScale.y;
			localScale.z = _scale.z / parentScale.z;

			return localScale;
		}

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