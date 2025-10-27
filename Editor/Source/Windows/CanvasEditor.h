#pragma once

#include "EditorWindow.h"
#include "Core/CryonicCore.h"
#include "ThirdParty/Misc/json.hpp"
#include "Core/GameObject.h"
#include "Raylib/RaylibWrapper.h"
#include "Resources/Canvas.h"
#include <ctime>

class CanvasEditor : public EditorWindow
{
public:
    void Render();
    void SaveCanvas();
	void LoadCanvas(Canvas* canvas);

private:
	RaylibWrapper::Rectangle GetJsonRect(nlohmann::json* goJson, RaylibWrapper::Rectangle parentRect);
	Vector2 GetJsonPivotPosition(nlohmann::json* goJson, RaylibWrapper::Rectangle rect);
	void DeleteElement(int id);
	void RenderElements(nlohmann::json* goJson, RaylibWrapper::Rectangle parentRect, RaylibWrapper::Rectangle clipRect);
	bool RenderHierarchyNode(nlohmann::json* gameObject, bool normalColor, bool& childDoubleClicked, nlohmann::json* objectHovered);
	void RenderHierarchy();
	void RenderProperties();

	Canvas* canvas;
    nlohmann::json canvasData;
	nlohmann::json* selectedObject = nullptr;
	bool hierarchyObjectClicked = false;
	nlohmann::json* objectInProperties = nullptr;
	nlohmann::json* objectInHierarchyContextMenu = nullptr;
	bool hierarchyContextMenuOpen = false;
	bool dragging = false;
	Vector2 dragStart = { 0,0 };
	Vector2 mouseStart = { 0,0 };
};