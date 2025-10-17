#pragma once
#include <string>
#include "ThirdParty/imgui/imgui_internal.h"

class EditorWindow
{
public:
	void Init(std::string windowName, std::string id = "", bool useIdForName = false, std::string icon = "", ImVec4 backgroundColor = {});
	virtual void Render() = 0;
	virtual void OnClose() {};

	bool windowOpen = true;
	std::string windowName = "";
	std::string windowID = "";
	std::string fullWindowName = "";
	ImVec4 windowColor = {};

	static ImGuiWindowClass defaultWindowClass;
};