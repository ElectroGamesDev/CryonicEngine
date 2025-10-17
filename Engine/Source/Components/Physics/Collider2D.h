#pragma once

#include "Components/Component.h"
#if !defined(EDITOR)
#include "ThirdParty/box2d/include/box2d.h"
#endif

class Rigidbody2D;

class Collider2D : public Component {
public:
    Collider2D(GameObject* obj, int id);
    Collider2D* Clone() override
    {
        return new Collider2D(gameObject, -1);
    }

    enum Shape
    {
        Square,
        Circle
    };

    // Hide in API
    void Highlight(Color color, bool highlightChildren);
    // Hide in API
    void SetRigidbody(Rigidbody2D* rb);
    // Hide in API
    void RemoveRigidbody();

    void Start() override;
    void EditorUpdate() override;
    void Destroy() override;
    void Enable() override;
    void Disable() override;

    Shape GetShape();
    void SetTrigger(bool value);
    bool IsTrigger();
    void SetOffset(Vector2 offset);
    Vector2 GetOffset();
    void SetSize(Vector2 size);
    Vector2 GetSize();

    // Hide in API
    bool highlight = false;

private:
    Shape shape;
    bool trigger;
    Vector2 offset;
    Vector2 size = {1, 1};


#if !defined(EDITOR)
    void Createb2Fixture();

    b2FixtureDef fixtureDef;
    b2Body* body = nullptr;
    b2Fixture* fixture = nullptr;
    bool ownBody = false;
#endif
};
