#pragma once

#include "Components/Component.h"
#include "ThirdParty/Misc/json.hpp"
#include "Core/GameObject.h"
#include "Resources/Canvas.h"

class CanvasRenderer : public Component
{
public:
	CanvasRenderer(GameObject* obj, int id) : Component(obj, id)
	{
		name = "CanvasRenderer";
		iconUnicode = "\xef\x89\x87";

#if defined(EDITOR)
		std::string variables = R"(
        [
            0,
            [
                [
                    "Canvas",
                    "canvas",
                    "nullptr",
                    "Canvas",
                    {
                        "Extensions": [".canvas"]
                    }
                ]
            ]
        ]
    )";
		exposedVariables = nlohmann::json::parse(variables);
#endif
	}
	CanvasRenderer* Clone() override { return new CanvasRenderer(gameObject, -1); }
	void Awake() override;
	void Start() override;
	void Enable() override;
	void Disable() override;
	void RenderGui() override;
#if defined(EDITOR)
	void EditorUpdate() override;
#endif
	void Destroy() override;

	void SetCanvas(Canvas* canvas);
	void LoadCanvas();

	GameObject* GetElement(const std::string& name);

	template <typename T>
	T* GetElementComponent(GameObject* gameObject)
	{
		// Todo: Make sure the parent is an element in this canvas renderer

		if (!gameObject)
			return nullptr; // Todo: Return error

		return gameObject->GetComponent<T>();
	}

	template <typename T>
	T* GetElementComponent(std::string& gameObject)
	{
		// Todo: Make sure the parent is an element in this canvas renderer

		GameObject* go = GetElement(gameObject);

		if (!go)
			return nullptr; // Todo: Return error

		return go->GetComponent<T>();
	}

	template <typename T>
	T* AddElementComponent(GameObject* parent)
	{
		// Todo: Make sure the parent is an element in this canvas renderer

		if (!canvas)
		{
			ConsoleLogger::ErrorLog("CanvasRenderer: Failed to add element. The gameobject \"" + gameObject->GetName() + "\" does not have a Canvas setup in the CanvasRenderer.");
			return;
		}

		if (!parent)
			return nullptr; // Todo: Return error

		T* newElement = &parent->AddComponentInternal<T>();
		Component* component = static_cast<Component*>(newElement);

		if (awakeCalled)
		{
			component->Awake();
			component->awakeCalled = true;
		}

		if (active && gameObject->active && gameObject->globalActive)
			component->Enable();

		if (startCalled)
		{
			component->Start();
			component->startCalled = true;
		}

		return newElement;
	}

	void DestroyElement(GameObject* go);
private:
	Canvas* canvas = nullptr;
	std::deque<GameObject*> gameObjects;

	void TraverseAndRender(GameObject* go);
	bool Traverse(GameObject* go, const std::function<bool(GameObject*)>& func);
	size_t onDataChangeEventId;
};