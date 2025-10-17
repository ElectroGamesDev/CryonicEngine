#include "EditorWindow.h"
#include <cstdlib>
#include <ctime>
#include <filesystem>

ImGuiWindowClass EditorWindow::defaultWindowClass;

void EditorWindow::Init(std::string name, std::string id, bool useIdForName, std::string icon, ImVec4 backgroundColor)
{
	windowName = name;
	windowID = id;
	if (windowID.empty())
	{
		srand(static_cast<unsigned int>(time(0)));
		windowID = std::to_string(100000 + rand() % 900000);
	}

	std::string iconString = "";
	if (!icon.empty())
		iconString = icon + " ";

	if (useIdForName)
		fullWindowName = iconString + std::filesystem::path(id).stem().string() + "##" + id;
	else
		fullWindowName = iconString + name + "##" + id;

	windowColor = backgroundColor;

	//ImGui::DockBuilderDockWindow((fullWindowName).c_str(), ImGui::GetID("DockSpace"));
}