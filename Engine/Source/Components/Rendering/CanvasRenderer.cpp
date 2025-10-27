#include "CanvasRenderer.h"
#include "Components/UI/Label.h"
#include "Components/UI/Button.h"
#include "Components/UI/Image.h"
#include "Components/UI/RectTransform.h"
#include "Utilities/ConsoleLogger.h"
#include "Raylib/RaylibWrapper.h"

void CanvasRenderer::Awake()
{
	SetCanvas(canvas);
}

void CanvasRenderer::Start()
{
	for (GameObject* go : gameObjects)
	{
		// We dont need to check if go is active, since its checked in Traverse()

		Traverse(go, [](GameObject* traversedGo) {
			if (!traversedGo->IsActive() || !traversedGo->IsGlobalActive())
				return true; // Stop traversing children

			for (Component* component : traversedGo->GetComponents())
			{
				if (component->active)
				{
					component->Start();
					component->startCalled = true;
				}
			}
			return false; // continue traversal
		});
	}
}

// Todo: This doesn't actually set the gameobject's globalActive to false. This needs to be added, but we would need to keep track of which ones were disable before the CanvasRenderer is disabled, and while it's disabled
void CanvasRenderer::Enable()
{
	for (GameObject* go : gameObjects)
	{
		// We dont need to check if go is active, since its checked in Traverse()

		Traverse(go, [](GameObject* traversedGo) {
			if (!traversedGo->IsActive() || !traversedGo->IsGlobalActive())
				return true; // Stop traversing children

			for (Component* component : traversedGo->GetComponents())
			{
				if (component->active)
					component->Enable();
			}
			return false; // continue traversal
		});
	}
}

void CanvasRenderer::Disable()
{
	for (GameObject* go : gameObjects)
	{
		// We dont need to check if go is active, since its checked in Traverse()

		Traverse(go, [](GameObject* traversedGo) {
			if (!traversedGo->IsActive() || !traversedGo->IsGlobalActive())
				return true; // Stop traversing children

			for (Component* component : traversedGo->GetComponents())
			{
				if (component->active)
					component->Disable();
			}
			return false; // continue traversal
		});
	}
}

void CanvasRenderer::RenderGui()
{
	if (!canvas)
		return;

	for (GameObject* go : gameObjects)
		TraverseAndRender(go);
}

#if defined(EDITOR)
void CanvasRenderer::EditorUpdate()
{
	std::string newPath = exposedVariables[1][0][2].get<std::string>();
	if ((!canvas && !newPath.empty() && newPath != "nullptr") || (canvas && canvas->GetRelativePath() != newPath))
	{
		exposedVariables[1][0][2] = newPath;

		if (newPath.empty() || newPath == "nullptr")
			SetCanvas(nullptr);
		else
			SetCanvas(new Canvas(newPath));
	}

	// Call EditorUpdate() on all elements
	for (GameObject* go : gameObjects)
	{
		// We dont need to check if go is active, since its checked in Traverse()

		Traverse(go, [](GameObject* traversedGo) {
			if (!traversedGo->IsActive() || !traversedGo->IsGlobalActive())
				return true; // Stop traversing children

			for (Component* component : traversedGo->GetComponents())
			{
				if (component->active)
					component->EditorUpdate();
			}
			return false; // continue traversal
		});
	}
}
#endif

void CanvasRenderer::Destroy()
{
	for (GameObject* go : gameObjects)
		DestroyElement(go);
}

void CanvasRenderer::SetCanvas(Canvas* canvas)
{
	if (this->canvas)
	{
		this->canvas->onDataChangeEvent.Unsubscribe(onDataChangeEventId);

		// Only deleting the canvas if its the editor, just in case the user set their own canvas. This could lead to a memory leak, but there isn't a good solution at the moment
#if defined(EDITOR)
		delete this->canvas;
#endif
		for (GameObject* go : gameObjects)
			DestroyElement(go);
	}

	this->canvas = canvas;

	if (!canvas)
		return;

	ConsoleLogger::ErrorLog("called");

	// Subscribe to the onDataChange so it's up-to-date
	onDataChangeEventId = canvas->onDataChangeEvent.Subscribe([this, canvas]() {
		LoadCanvas();
	});

	LoadCanvas();
}

void CanvasRenderer::LoadCanvas()
{
	nlohmann::json* data = canvas->GetData();
	if (!data || data->is_null())
		return;

	std::unordered_map<int, GameObject*> idMap;

	std::function<GameObject* (const nlohmann::json&, GameObject*)> createGameObject =
		[&](const nlohmann::json& goJson, GameObject* parent) -> GameObject*
		{
			int id = goJson["id"].get<int>();
			GameObject* go = new GameObject(id);
			idMap[id] = go;

			//go->transform.SetPosition(Vector3{ goJson["position"][0], goJson["position"][1], goJson["position"][2] });
			//go->transform.SetScale(Vector3{ goJson["size"][0], goJson["size"][1], goJson["size"][2] });
			//go->transform.SetRotation(Quaternion{ goJson["rotation"][0], goJson["rotation"][1], goJson["rotation"][2], goJson["rotation"][3] });

			go->SetName(goJson["name"].get<std::string>());
			go->SetActive(goJson["active"].get<bool>());
			go->SetGlobalActive(goJson["global_active"].get<bool>());

			if (parent)
				go->SetParent(parent);
			else
				gameObjects.push_back(go);

			// Load components
			if (goJson.contains("components"))
			{
				for (auto& compJson : goJson["components"])
				{
					std::string type = compJson["name"].get<std::string>();
					int compId = compJson["id"].get<int>();
					bool active = compJson.value("active", true);
					nlohmann::json exposed = compJson.value("exposed_variables", nlohmann::json::object());

					if (type == "RectTransform")
					{
						RectTransform& rt = go->AddComponentInternal<RectTransform>(compId);
						rt.active = active;
						rt.exposedVariables = exposed;
					}
					else if (type == "Label")
					{
						Label& label = go->AddComponentInternal<Label>(compId);
						label.active = active;
						label.exposedVariables = exposed;
					}
					else if (type == "Button")
					{
						Button& button = go->AddComponentInternal<Button>(compId);
						button.active = active;
						button.exposedVariables = exposed;
					}
					else if (type == "Image")
					{
						Image& image = go->AddComponentInternal<Image>(compId);
						image.active = active;
						image.exposedVariables = exposed;
					}
				}
			}

			for (Component* component : go->GetComponents())
			{
				component->Awake();
				component->awakeCalled = true;
			}

			// Recursively create children
			if (goJson.contains("children"))
			{
				for (auto& childJson : goJson["children"])
					createGameObject(childJson, go);
			}		

			return go;
		};

	// Create root objects
	if (data->contains("GameObjects"))
	{
		for (auto& goJson : (*data)["GameObjects"])
			createGameObject(goJson, nullptr);
	}

#if !defined(EDITOR)
	// Set exposed variables for all components
	for (auto& go : gameObjects)
	{
		for (Component* comp : go->GetComponents())
			comp->SetExposedVariables();
	}
#endif
}
void CanvasRenderer::TraverseAndRender(GameObject* go)
{
	if (!go->IsActive() || !go->IsGlobalActive())
		return;

	for (Component* comp : go->GetComponents())
	{
		UIElement* uie = dynamic_cast<UIElement*>(comp);
		if (uie && comp->IsActive())
			uie->RenderGui();
	}

	for (GameObject* child : go->GetChildren())
		TraverseAndRender(child);
}

GameObject* CanvasRenderer::GetElement(const std::string& name)
{
	GameObject* result = nullptr;

	for (GameObject* go : gameObjects)
	{
		if (Traverse(go, [&](GameObject* traversedGo) {
			if (traversedGo->GetName() == name)
			{
				result = traversedGo;
				return true;
			}
			return false;
			}))
		{
			break;
		}
	}

	return result;
}

void CanvasRenderer::DestroyElement(GameObject* go) // Todo: This will cause a crash if removed while it's iterating the game objects. Do a similar approach as DestroyGameObject() in Scene
{
	if (!go)
		return;

	std::deque<GameObject*> childrenCopy = go->GetChildren();
	for (GameObject* child : childrenCopy)
		DestroyElement(child);

	for (Component* component : go->GetComponents())
		go->RemoveComponent(component);

	if (go->GetParent() != nullptr)
		go->SetParent(nullptr);
	else
	{
		auto it = std::find_if(gameObjects.begin(), gameObjects.end(),
			[go](const GameObject* newGo) { return newGo->GetId() == go->GetId(); });
		if (it != gameObjects.end())
			gameObjects.erase(it);
	}

	delete go;

	go = nullptr;
}

bool CanvasRenderer::Traverse(GameObject* go, const std::function<bool(GameObject*)>& func)
{
	if (!go)
		return false;

	if (func(go))
		return true;

	for (GameObject* child : go->GetChildren())
		if (Traverse(child, func))
			return true;

	return false;
}