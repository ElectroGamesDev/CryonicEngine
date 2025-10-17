#include "Collider3D.h"
#include "Rigidbody3D.h"
#include "Systems/Events/EventSystem.h"
#if !defined(EDITOR)
#include "Game.h"
#else
#include "Core/Editor.h"
#endif

Collider3D::Collider3D(GameObject* obj, int id) : Component(obj, id) {
	name = "Collider3D";
	iconUnicode = "\xef\x89\x8d";

#if defined(EDITOR)
	std::string variables = R"(
        [
            0,
            [
                [
                    "Shape",
                    "shape",
                    "Box",
                    "Shape",
                    [
                        "Box",
                        "Sphere",
						"Plane",
						"Cylinder",
						"Cone"
                    ]
                ],
                [
                    "bool",
                    "trigger",
                    false,
                    "Trigger"
                ],
                [
                    "Vector3",
                    "offset",
                    [0,0,0],
                    "Offset"
                ],
				[
                    "Vector3",
                    "size",
                    [1,1,1],
                    "Size"
                ]
            ]
        ]
    )";
	exposedVariables = nlohmann::json::parse(variables);
#endif
}

// Todo: Check if the game object or component is enabled/disabled, if it is then body->SetActive(). Also check if Rigidbody3D is destroyed, if it is then look for a new one, or create one. (Check when its destroyed in the Rigidbody3D Destroy() )

void Collider3D::CreateShape()
{
#if !defined(EDITOR)
	switch (shape)
	{
		default:
		case Box:
		{
			JPH::BoxShapeSettings shape({ (gameObject->transform.GetScale().x * 3 * size.x / 2) / 3, (gameObject->transform.GetScale().y * 3 * size.y / 2) / 3, (gameObject->transform.GetScale().z * 3 * size.z / 2) / 3 });
			joltShape = shape.Create().Get();
			break;
		}
		case Sphere:
		{
			JPH::SphereShapeSettings shape((gameObject->transform.GetScale().x * size.x) / 2.0f);
			joltShape = shape.Create().Get();
			break;
		}
		case Plane:
		{
			JPH::BoxShapeSettings shape({ (gameObject->transform.GetScale().x * 3 * size.x / 2) / 3, 0.06f, (gameObject->transform.GetScale().z * 3 * size.z / 2) / 3 });
			joltShape = shape.Create().Get();
			// Todo: Implementan actual plane
			//JPH::Plane plane({ 0.0f, 1.0f, 0.0f }, distanceFromOrigin); // First parameter is the plane normal, the second parameter is confusing
			//plane = plane.Scaled({ (gameObject->transform.GetScale().x * 3 * size.x / 2) / 3, 1, (gameObject->transform.GetScale().z * 3 * size.z / 2) / 3 });
			//JPH::PlaneShapeSettings shape(plane, nullptr); // Can set physics material instead of nullptr
			//joltShape = shape.Create().Get();
			break;
		}
		case Cylinder:
		{
			float radius = (gameObject->transform.GetScale().x * size.x) / 2;
			float height = gameObject->transform.GetScale().y * size.y;

			JPH::CylinderShapeSettings shape(height, radius);
			joltShape = shape.Create().Get();
			break;
		}
		case Cone: // Todo: This needs to be changed to use a mesh shape
		{
			float radius = (gameObject->transform.GetScale().x * size.x) / 2;
			float height = gameObject->transform.GetScale().y * size.y / 2; 

			JPH::CylinderShapeSettings shape(height, radius);
			joltShape = shape.Create().Get();
			break;
		}
	}
#endif
}

void Collider3D::Start()
{
#if !defined(EDITOR)
	if (body)
		return;

	// Putting this in Start() instead of Awake() to ensure the rigidbody component gets set up first

	Rigidbody3D* rb = gameObject->GetComponent<Rigidbody3D>();
	if (!trigger && rb != nullptr && rb->IsActive() && rb->gameObject->IsActive() && rb->gameObject->IsGlobalActive())
		SetRigidbody(rb);
	else
	{
		//fixtureDef.density = rb->GetMass();
		//fixtureDef.friction = 0.3f;
		CreateShape();
		Vector3 goPosition = gameObject->transform.GetPosition() + offset;
		Quaternion goRotation = gameObject->transform.GetRotation();
		if (joltShape == nullptr)
			return;

		JPH::BodyCreationSettings bodySettings(joltShape, { goPosition.x, goPosition.y, goPosition.z }, { goRotation.x, goRotation.y, goRotation.z, goRotation.w }, JPH::EMotionType::Static, 0); // Last parameter is the layer
		bodySettings.mGravityFactor = 0;
		bodySettings.mIsSensor = trigger;
		bodySettings.mMotionQuality = (continuousDetection ? JPH::EMotionQuality::LinearCast : JPH::EMotionQuality::Discrete);
		bodySettings.mUserData = reinterpret_cast<uint64_t>(this);
		body = Rigidbody3D::bodyInterface->CreateBody(bodySettings);
		Rigidbody3D::bodyInterface->AddBody(body->GetID(), JPH::EActivation::Activate);

		ownBody = true;
	}
	SetTrigger(trigger);
#endif

	// Subscribes to the ObjectSelected and ObjectDeselected editor events

	//EventSystem::Subscribe("ObjectSelected", Test);
	// Todo: Move this to the above if defined, and handle if the object is selected, deslected, changed, moved, or if this object moves.
	//EventSystem::Subscribe("ObjectSelected", [](GameObject* gameObject) {
	//	ConsoleLogger::InfoLog("Object selected: " + gameObject->GetName());
	//	});

	//EventSystem::Subscribe("ObjectMoved", [](GameObject* gameObject) {
	//	});

	//EventSystem::Subscribe("ObjectDeselected", [](GameObject* gameObject) {
	//	ConsoleLogger::InfoLog("Object deselected: " + gameObject->GetName());
	//	});

	//EventSystem::Subscribe("ObjectSelected", [](GameObject* gameObject) {});
}

void Collider3D::FixedUpdate()
{
#if !defined(EDITOR)
	if (!ownBody)
		return;

	// Todo: Use SetPositionAndRotation() when possible
	
	// Update position if the game object has moved
	Vector3 position = gameObject->transform.GetPosition();
	if (lastGOPosition != position)
	{
		// Update physics body position (with offset if needed)
		Rigidbody3D::bodyInterface->SetPosition(body->GetID(), { position.x + offset.x, position.y + offset.y, position.z + offset.z }, JPH::EActivation::DontActivate);
		lastGOPosition = position;
	}

	// Update rotation if the game object has rotated
	Quaternion rotation = gameObject->transform.GetRotation();
	if (lastGORotation != rotation)
	{
		// Update physics body rotation (correct method: SetRotation)
		Rigidbody3D::bodyInterface->SetRotation(body->GetID(), { rotation.x, rotation.y, rotation.z, rotation.w }, JPH::EActivation::DontActivate);
		lastGORotation = rotation;
}

	// Update the game object's position if the physics body moved
	JPH::Vec3 physicsPosition = Rigidbody3D::bodyInterface->GetPosition(body->GetID());
	if (lastPhysicsPosition != physicsPosition)
	{
		gameObject->transform.SetPosition({ physicsPosition.GetX() - offset.x, physicsPosition.GetY() - offset.y, physicsPosition.GetZ() - offset.z});
		lastPhysicsPosition = physicsPosition;
	}

	// Update the game object's rotation if the physics body rotated
	JPH::Quat physicsRotation = Rigidbody3D::bodyInterface->GetRotation(body->GetID());
	if (lastPhysicsRotation != physicsRotation)
	{
		gameObject->transform.SetRotation({ physicsRotation.GetX(), physicsRotation.GetY(), physicsRotation.GetZ(), physicsRotation.GetW() });
		lastPhysicsRotation = physicsRotation;
	}
#endif
}

void Collider3D::Highlight(Color color, bool highlightChildren)
{
	// Must go inside this if defined to prevent compiler error
#if defined(EDITOR)
	offset.x = exposedVariables[1][2][2][0].get<float>();
	offset.y = exposedVariables[1][2][2][1].get<float>();
	offset.z = exposedVariables[1][2][2][2].get<float>();
	size.x = exposedVariables[1][3][2][0].get<float>();
	size.y = exposedVariables[1][3][2][1].get<float>();
	size.z = exposedVariables[1][3][2][2].get<float>();

	//ConsoleLogger::WarningLog("Highlighting " + gameObject->GetName());

	Vector3 position = gameObject->transform.GetPosition();
	Vector3 scale = gameObject->transform.GetScale();

	// Getting type from exposedVariables since shape variable isn't up-to-date with exposed variable
	std::string type = exposedVariables[1][0][2].get<std::string>();

	if (type == "Square") // Todo: For children, this is being renderered before spriteRenderer
	{
		RaylibWrapper::DrawRectangleOutline({ position.x + offset.x, position.y + offset.y, scale.x * 3 * size.x, scale.y * 3 * size.y },
			{ scale.x * 3 * size.x / 2, scale.y * 3 * size.y / 2 },
			gameObject->transform.GetRotationEuler().z,
			0.1f,
			{ color.r, color.g, color.b, color.a });
	}
	else if (type == "Circle")
	{
		// Todo: This is not a good solution
		for (int i = 0; i < 7; i++)
			RaylibWrapper::DrawCircleLinesV({ position.x + offset.x, position.y + offset.y }, scale.x * 1.5f * size.x + 0.01f * i, { color.r, color.g, color.b, color.a });
	}

	// Highlights child objects
	if (!highlightChildren)
		return;

	for (GameObject* child : gameObject->GetChildren())
	{
		// Using a bool so it only highlights children once
		bool colliderFound = false;
		for (Component* component : child->GetComponents())
		{
			Collider3D* collider = dynamic_cast<Collider3D*>(component);
			if (collider)
			{
				collider->highlight = true; // Using a highlight variable instead of calling Highlight since it will render the highlight before the sprite
				//collider->Highlight({ 24, 201, 24, 200 }, !colliderFound);
				colliderFound = true;
			}
		}
	}
#endif
}

void Collider3D::EditorUpdate()
{
	// Must be inside the if defined since the game compiled version doesn't include Varient
#if defined(EDITOR)
	// Todo: Replace this solution with Events. Another possible solution would be to have another EditorUpdate() with a selectedGameObject parameter, but this is still a bad solution.
	if (highlight)
	{
		highlight = false; // Resetting the highlight variable so it doesn't stay highlighted when it shouldn't
		Highlight({ 24, 201, 24, 200 }, true);
	}
	else if (std::holds_alternative<GameObject*>(Editor::objectInProperties) && std::get<GameObject*>(Editor::objectInProperties) == gameObject)
		Highlight({ 0, 255, 0, 200 }, true);
#endif
}

void Collider3D::Destroy()
{
#if !defined(EDITOR)
	if (!body)
		return;

	const JPH::BodyID& bodyID = ownBody ? body->GetID() : rb->body->GetID();

	Rigidbody3D::bodyInterface->RemoveBody(bodyID);
	Rigidbody3D::bodyInterface->DestroyBody(bodyID);
#endif
}

void Collider3D::Enable()
{
#if !defined(EDITOR)
	if (!body) // This is needed since the body is set in Start(), and Enable() runs before Start()
		return;

	if (ownBody)
		Rigidbody3D::bodyInterface->AddBody(body->GetID(), JPH::EActivation::Activate);
	else
		rb->AddCollider(this);
#endif
}

void Collider3D::Disable()
{
#if !defined(EDITOR)
	if (ownBody)
		Rigidbody3D::bodyInterface->RemoveBody(body->GetID());
	else
		rb->RemoveCollider(this);
#endif
}

void Collider3D::SetRigidbody(Rigidbody3D* rigidbody)
{
#if !defined(EDITOR)
	if (body)
	{
		if (ownBody)
		{
			Rigidbody3D::bodyInterface->RemoveBody(body->GetID());
			Rigidbody3D::bodyInterface->DestroyBody(body->GetID());
			ownBody = false;
		}
		else
		{
			rb->RemoveCollider(this);
			rb = nullptr;
		}
	}
	CreateShape();
	rb = rigidbody;
	body = rb->body;
	rb->AddCollider(this);
	//fixtureDef.density = rb->GetMass();
	//fixtureDef.friction = 0.3f;
	//fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(this);
	//Createb2Fixture();
#endif
}

void Collider3D::RemoveRigidbody()
{
#if !defined(EDITOR)
	if (!body)
		return;

	if (ownBody)
	{
		Rigidbody3D::bodyInterface->RemoveBody(body->GetID());
		Rigidbody3D::bodyInterface->DestroyBody(body->GetID());
		ownBody = false;
	}
	else
	{
		rb->RemoveCollider(this);
		rb = nullptr;
	}
	// Tries to find a new body, if it fails to find one, then create one
	// Removed this code because only one Rigidbody can be attached to a gameobjerct. If this function is called, then its assumed its called by the only rigidbody component
	//Rigidbody3D* rb = gameObject->GetComponent<Rigidbody3D>();
	//if (rb == nullptr || !rb->IsActive() || !rb->gameObject->IsActive() || !rb->gameObject->IsGlobalActive())
	//{

	CreateShape();
	//b2BodyDef bodyDef;
	//bodyDef.type = b2_staticBody;
	//bodyDef.position.Set(gameObject->transform.GetPosition().x, gameObject->transform.GetPosition().y);
	//body = world->CreateBody(&bodyDef);
	Vector3 goPosition = gameObject->transform.GetPosition() + offset;
	Quaternion goRotation = gameObject->transform.GetRotation();
	JPH::BodyCreationSettings bodySettings(joltShape, { goPosition.x, goPosition.y, goPosition.z }, { goRotation.x, goRotation.y, goRotation.z, goRotation.w }, JPH::EMotionType::Static, Layers::NON_MOVING);
	bodySettings.mGravityFactor = 0;
	bodySettings.mIsSensor = trigger;
	bodySettings.mMotionQuality = (continuousDetection ? JPH::EMotionQuality::LinearCast : JPH::EMotionQuality::Discrete);
	bodySettings.mUserData = reinterpret_cast<uint64_t>(this);
	body = Rigidbody3D::bodyInterface->CreateBody(bodySettings);
	Rigidbody3D::bodyInterface->AddBody(body->GetID(), JPH::EActivation::Activate);
	ownBody = true;

	// Setting the density and friction in case a Rigidbody3D is added to the game object
	//fixtureDef.density = 1.0f;
	//fixtureDef.friction = 0.3f;
	//}
	//else
	//{
	//	fixtureDef.density = rb->GetMass();
	//	fixtureDef.friction = 0.3f;
	//	body = rb->body;
	//	rb->colliders.push_back(this);
	//}

	//fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(this);
	//Createb2Fixture();
#endif
}

#if defined(EDITOR)
void Collider3D::SetShapeInternal(std::string shape)
{
	exposedVariables[1][0][2] = shape;
}
#endif


Collider3D::Shape Collider3D::GetShape()
{
	return shape;
}

void Collider3D::SetTrigger(bool value)
{
	trigger = value;
#if !defined(EDITOR)
	if (!body)
		return;

	if (ownBody)
		body->SetIsSensor(trigger);
	else
		RemoveRigidbody();
#endif
}

bool Collider3D::IsTrigger()
{
#if !defined(EDITOR)
	return trigger;
#else
	return false;
#endif
}

void Collider3D::SetOffset(Vector3 offset)
{
	this->offset = offset;
#if defined(EDITOR)
	exposedVariables[1][2][2] = { offset.x, offset.y, offset.z };
#else
	CreateShape();
	if (ownBody)
	{
		Vector3 goPosition = gameObject->transform.GetPosition();
		Rigidbody3D::bodyInterface->SetPosition(body->GetID(), { goPosition.x + offset.x, goPosition.y + offset.y, goPosition.z + offset.z }, JPH::EActivation::DontActivate);
	}
	else
		rb->UpdateShape(this, true);
#endif
}

Vector3 Collider3D::GetOffset()
{
	return offset;
}

void Collider3D::SetSize(Vector3 size)
{
	this->size = size;
#if !defined(EDITOR)
	CreateShape();
	if (ownBody)
		Rigidbody3D::bodyInterface->SetShape(body->GetID(), joltShape, true, JPH::EActivation::DontActivate);
	else
		rb->UpdateShape(this);
#endif
}

Vector3 Collider3D::GetSize()
{
	return size;
}