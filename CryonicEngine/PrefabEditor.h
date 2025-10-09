#pragma once
#include "EditorWindow.h"
#include <json.hpp>
#include <any>
#include <unordered_set>
#include "EventSheetSystem.h"

class PrefabEditor : public EditorWindow
{
public:
	void Render() override;
	void OnClose() override;
	void LoadData(nlohmann::json data, std::string path);
	void SaveData();
};