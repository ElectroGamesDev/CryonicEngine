#pragma once
#include "ThirdParty/imgui/imgui_internal.h"
#include <string>

namespace AssetManager
{
	void RenderWindow();
	void CheckForUpdates();
	bool InstallAsset(std::string url, std::string name);
	bool UpdateAsset(std::string name, std::string url);
	bool UninstallAsset(std::string url);
	bool FetchAssets();
	void Init(ImGuiWindowClass* windowClass);
	void Cleanup();

	extern bool open;
}