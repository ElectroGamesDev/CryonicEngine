#pragma once

#include <string>
#include "../GameObject.h"
//class GameObject;
#if defined(EDITOR)
#include <json.hpp>
#endif

class Collider2D;
class Collider3D;

#define Expose
#define Hide
#define PUBLIC  
#define PRIVATE
#define PROTECTED
#define STATIC

class Component
{
public:
    Component(GameObject* obj, int id = -1);
    virtual ~Component() {};
    GameObject* GetGameObject()
    {
        return gameObject;
    }
    
    // Hide in API
    std::string iconUnicode = "";
    std::string name;
    bool runInEditor = false;
    bool runInGame = true;
    // Hide In API
    bool active = true;
    // Hide In API
    int id;
    // Hide in API
    bool startCalled = false;
    // Hide in API
    bool awakeCalled = false;
    // Hide in API
    bool initialized = false; // Whether the component is ready to call functions like Start, Awake, Enable, Disable, etc.
    // Hide in API
    bool valid = true; // Used in component construcors to let AddComponent<>() know if it should continue with adding the component. Used in Rigidbody2D

#if defined(EDITOR)
    nlohmann::json exposedVariables = nullptr;
#endif

    void SetActive(bool active)
    {
        if (active == this->active)
            return;
        this->active = active;
        if (gameObject->IsGlobalActive() && initialized)
        {
            if (active)
            {
                if (!awakeCalled)
                {
                    awakeCalled = true;
                    Awake();
                }
                Enable();
                if (!startCalled)
                {
                    startCalled = true;
                    Start();
                }
            }
            else
                Disable();
        }
    };
    bool IsActive() const { return active; };

    // Hide In API
    virtual void SetExposedVariables() {};
    // Hide In API
    virtual Component* Clone() { return nullptr; };

    // Called once when the game object or component is first created
    virtual void Awake() {}

    // Called after Awake() and before the first frame update
    virtual void Start() {};

    // Called once per frame to update game logic
    virtual void Update() {};

    // Called at fixed time intervals for physics-related game logic
    virtual void FixedUpdate() {};

    // Called once per frame within the editor
    virtual void EditorUpdate() {};

    // Called once per frame to handle GUI rendering
    virtual void RenderGui() {};

    // Todo: Change the parameter name to "isShadowPass"
    // Called once per frame to handle mesh rendering
    virtual void Render(bool renderShadows = false) {};

    // Called when the game object or component is destroyed, or the scene is unloaded
    virtual void Destroy() {};

    // Called when the component or gameobject is enabled/activated
    virtual void Enable() {};

    // Called when the component or gameobject is disabled/deactivated
    virtual void Disable() {};

    // Called when the object starts colliding with another 2D collider
    virtual void OnCollisionEnter2D(Collider2D* other) {};

    // Called when the object stops colliding with another 2D collider
    virtual void OnCollisionExit2D(Collider2D* other) {};

    // Called every frame while the object is colliding with another 2D collider
    virtual void OnCollisionStay2D(Collider2D* other) {};

    // Called when the object starts colliding with another 3D collider
    virtual void OnCollisionEnter3D(Collider3D* other) {};

    // Called when the object stops colliding with another 3D collider
    virtual void OnCollisionExit3D(Collider3D* other) {};

    // Called every frame while the object is colliding with another 3D collider
    virtual void OnCollisionStay3D(Collider3D* other) {};

    // Hide in API
    static std::vector<Component*> markedForDeletion;

    GameObject* gameObject;
};
