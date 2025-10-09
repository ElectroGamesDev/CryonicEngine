#pragma once
#include "EditorWindow.h"
#include <json.hpp>
#include <any>
#include <unordered_set>
#include "EventSheetSystem.h"

class EventSheetEditor : public EditorWindow
{
public:
	void Render() override;
	void OnClose() override;
	void LoadData(nlohmann::json data, std::string path);
	void SaveData();

private:
	// Rendering methods
	void RenderEventTable();
	void RenderSelector();
	void RenderCategoryTree(bool isCondition);
	void RenderParameterEditor();

	// Helper methods
	void ResetSelection();
	std::string GetParameterDisplayString(const EventSheetSystem::Param& param) const;
	void RenderParameterInput(EventSheetSystem::Param& param, int index);
	void RenderKeySelector(EventSheetSystem::Param& param);
	void RenderMouseButtonSelector(EventSheetSystem::Param& param);
	void RenderGamepadButtonSelector(EventSheetSystem::Param& param);
	void RenderGamepadAxisSelector(EventSheetSystem::Param& param);
	void RenderGameObjectSelector(EventSheetSystem::Param& param);

	// Data members
	std::string filePath;
	std::vector<EventSheetSystem::Event> events;

	// Selection state
	int eventSelected = -1;      // Index of the event selected. -1 = none
	int conditionSelected = -1;  // Index of the condition selected. -1 = none, -2 = new
	int actionSelected = -1;     // Index of the action selected. -1 = none, -2 = new

	// Temporary data for editing
	EventSheetSystem::Condition tempConditionData = {};
	EventSheetSystem::Action tempActionData = {};

	// UI state
	std::unordered_set<std::string> expandedCategories;
	bool selectorFirstFrame = true;

	// Constants
	static constexpr float INDENT_SIZE = 20.0f;
	static constexpr float ROW_HEIGHT = 30.0f;
	static constexpr float BUTTON_HEIGHT = 25.0f;
};