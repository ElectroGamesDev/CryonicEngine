#pragma once

#include "Components/Component.h"
#include "Raylib/RaylibCameraWrapper.h"
#include <filesystem>

class CameraComponent : public Component
{
public:
	CameraComponent(GameObject* obj, int id);
	CameraComponent* Clone() override
	{
		return new CameraComponent(gameObject, -1);
	}
	void Start() override;
	void Update() override;
	void Destroy() override;
#ifdef EDITOR
	void EditorUpdate() override;
#endif
	Vector2 GetWorldToScreen(Vector3 position);

	RaylibCamera raylibCamera;;
	static CameraComponent* main;
	bool setMain = false;
};