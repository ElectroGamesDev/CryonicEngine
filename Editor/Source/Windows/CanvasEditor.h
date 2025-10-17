#pragma once

#include "ThirdParty/Misc/json.hpp"
//#include "GUI/Element.h"
#include "Core/GameObject.h"

namespace CanvasEditor
{
	bool RenderHierarchyNode(nlohmann::json* gameObject, bool normalColor, bool& childDoubleClicked);
	void RenderHierarchy();
	void Render();

	extern bool windowOpen;
	extern nlohmann::json canvasData;
}