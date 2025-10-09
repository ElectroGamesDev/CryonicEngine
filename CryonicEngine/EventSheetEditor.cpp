#include "EventSheetEditor.h"
#include "ConsoleLogger.h"
#include "FontManager.h"
#include <fstream>
#include <IconsFontAwesome6.h>

// Helper function implementations for EventSheetSystem
namespace EventSheetSystem
{
	std::string Param::GetDisplayString() const
	{
		if (type == "string")
			return name + " = " + std::any_cast<std::string>(value);
		else if (type == "int")
			return name + " = " + std::to_string(std::any_cast<int>(value));
		else if (type == "float")
			return name + " = " + std::to_string(std::any_cast<float>(value));
		else if (type == "bool")
			return name + " = " + (std::any_cast<bool>(value) ? "true" : "false");
		return "";
	}

	std::string Condition::GetFormattedText() const
	{
		if (params.empty())
			return stringType;

		std::string result = stringType + " (";
		for (size_t i = 0; i < params.size(); ++i)
		{
			if (i > 0) result += ", ";
			result += params[i].GetDisplayString();
		}
		result += ")";
		return result;
	}

	std::string Action::GetFormattedText() const
	{
		if (params.empty())
			return stringType;

		std::string result = stringType + " (";
		for (size_t i = 0; i < params.size(); ++i)
		{
			if (i > 0) result += ", ";
			result += params[i].GetDisplayString();
		}
		result += ")";
		return result;
	}
}

void EventSheetEditor::Render()
{
	RenderEventTable();
	RenderSelector();
}

void EventSheetEditor::RenderEventTable()
{
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.38f, 0.38f, 0.38f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.38f, 0.38f, 0.38f, 0.5f));

	for (int eventIndex = 0; eventIndex < static_cast<int>(events.size()); ++eventIndex)
	{
		EventSheetSystem::Event& event = events[eventIndex];
		ImGui::PushID(eventIndex);

		float height = std::max(event.conditions.size(), event.actions.size()) * ROW_HEIGHT;
		ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg;

		ImVec2 currentPos = ImGui::GetCursorPos();
		ImGui::SetCursorPosX(50);

		if (ImGui::BeginTable(("eventTable" + std::to_string(eventIndex)).c_str(), 2, tableFlags,
			{ ImGui::GetWindowWidth() - 100, height }))
		{
			ImGui::TableNextRow();

			// Conditions Column
			ImGui::TableNextColumn();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(48, 50, 52, 255));

			for (size_t i = 0; i < event.conditions.size(); ++i)
			{
				ImGui::PushID(static_cast<int>(i));
				ImGui::SetCursorPosX(53);

				std::string buttonText = event.conditions[i].GetFormattedText() + "##" +
					std::to_string(eventIndex) + "-" + std::to_string(i);

				if (ImGui::Button(buttonText.c_str()))
				{
					eventSelected = eventIndex;
					conditionSelected = static_cast<int>(i);
				}

				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.1f, 0.1f, 1.0f));

				if (ImGui::Button("X"))
				{
					event.conditions.erase(event.conditions.begin() + i);
					--i; // Adjust index after deletion
				}

				ImGui::PopStyleColor();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
				ImGui::PopID();
			}

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
			if (ImGui::Button("Add Condition"))
			{
				eventSelected = eventIndex;
				conditionSelected = -2;
			}
			ImGui::PopStyleColor();

			// Actions Column
			ImGui::TableNextColumn();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(41, 43, 45, 255), 1);

			for (size_t i = 0; i < event.actions.size(); ++i)
			{
				ImGui::PushID(static_cast<int>(i));

				std::string buttonText = event.actions[i].GetFormattedText() + "##" +
					std::to_string(eventIndex) + "-" + std::to_string(i);

				if (ImGui::Button(buttonText.c_str()))
				{
					eventSelected = eventIndex;
					actionSelected = static_cast<int>(i);
				}

				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.1f, 0.1f, 1.0f));

				if (ImGui::Button("X"))
				{
					event.actions.erase(event.actions.begin() + i);
					--i; // Adjust index after deletion
				}

				ImGui::PopStyleColor();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
				ImGui::PopID();
			}

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
			if (ImGui::Button("Add Action"))
			{
				eventSelected = eventIndex;
				actionSelected = -2;
			}
			ImGui::PopStyleColor();

			ImGui::EndTable();
		}

		// Delete Event Button
		ImVec2 cursorPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos({ ImGui::GetWindowWidth() - 45, currentPos.y });
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.1f, 0.1f, 1.0f));

		if (ImGui::Button("X"))
		{
			events.erase(events.begin() + eventIndex);
			ImGui::PopStyleColor();
			ImGui::PopID();
			--eventIndex; // Adjust index after deletion
			continue;
		}

		ImGui::PopStyleColor();
		ImGui::SetCursorPos(cursorPos);
		ImGui::PopID();
	}

	ImGui::PopStyleColor(3);

	// Add Event Button
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Add Event").x) / 2);
	if (ImGui::Button("Add Event"))
	{
		events.push_back({});
	}
}

void EventSheetEditor::RenderSelector()
{
	if (eventSelected == -1)
		return;

	// Initialize temp data on first frame
	if (selectorFirstFrame)
	{
		if (conditionSelected >= 0)
			tempConditionData = events[eventSelected].conditions[conditionSelected];
		else
			tempConditionData = {};

		if (actionSelected >= 0)
			tempActionData = events[eventSelected].actions[actionSelected];
		else
			tempActionData = {};

		selectorFirstFrame = false;
	}

	// Semi-transparent overlay
	ImGui::SetNextWindowBgAlpha(0.3f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 1));
	ImGui::SetCursorPos({ 0, 0 });
	ImGui::BeginChild("BlockOverlay", ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()), 0, ImGuiWindowFlags_NoScrollbar);
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();

	// Main selector window
	ImGui::SetCursorPos({ 25, 50 });
	ImGui::BeginChild("Selector", { ImGui::GetWindowWidth() - 50, ImGui::GetWindowHeight() - 70 });

	ImGui::SetCursorPos({ 10, 10 });
	ImGui::PushFont(FontManager::GetFont("Familiar-Pro-Bold", 30, false));
	ImGui::Text(conditionSelected != -1 ? "Condition" : "Action");
	ImGui::PopFont();

	// Left side: Category tree
	ImGui::BeginChild("SelectingWin", { ImGui::GetWindowWidth() / 2, ImGui::GetWindowHeight() });
	RenderCategoryTree(conditionSelected != -1);
	ImGui::EndChild();

	// Right side: Parameter editor
	RenderParameterEditor();

	// Bottom buttons
	ImGui::SetCursorPos({ ImGui::GetWindowWidth() - 120, ImGui::GetWindowHeight() - 35 });
	if (ImGui::Button("Cancel", { 50, BUTTON_HEIGHT }))
	{
		ResetSelection();
	}

	bool canApply = (conditionSelected != -1 && !tempConditionData.stringType.empty()) ||
		(actionSelected != -1 && !tempActionData.stringType.empty());

	if (!canApply)
		ImGui::BeginDisabled();

	ImGui::SetCursorPos({ ImGui::GetWindowWidth() - 60, ImGui::GetWindowHeight() - 35 });
	if (ImGui::Button("Apply", { 50, BUTTON_HEIGHT }))
	{
		if (conditionSelected != -1)
		{
			if (conditionSelected == -2)
				events[eventSelected].conditions.push_back(tempConditionData);
			else
				events[eventSelected].conditions[conditionSelected] = tempConditionData;
		}
		else if (actionSelected != -1)
		{
			if (actionSelected == -2)
				events[eventSelected].actions.push_back(tempActionData);
			else
				events[eventSelected].actions[actionSelected] = tempActionData;
		}

		ResetSelection();
	}

	if (!canApply)
		ImGui::EndDisabled();

	ImGui::EndChild();
}

void EventSheetEditor::RenderCategoryTree(bool isCondition)
{
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.38f, 0.38f, 0.38f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.38f, 0.38f, 0.38f, 0.5f));
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0, 0.5f });

	std::unordered_set<std::string> drawnCategories;

	if (isCondition)
	{
		const auto& conditions = EventSheetSystem::GetAllConditions();

		for (const EventSheetSystem::Condition& condition : conditions)
		{
			std::string currentPath;
			bool allParentsExpanded = true;

			// Draw category hierarchy
			for (size_t i = 0; i < condition.category.size(); ++i)
			{
				if (!currentPath.empty())
					currentPath += " > ";
				currentPath += condition.category[i];

				// Only draw each category once
				if (drawnCategories.insert(currentPath).second)
				{
					bool isExpanded = expandedCategories.find(currentPath) != expandedCategories.end();
					std::string buttonLabel = (isExpanded ? ICON_FA_ANGLE_DOWN : ICON_FA_ANGLE_RIGHT);
					buttonLabel += " " + condition.category[i];

					float indentX = INDENT_SIZE * i;
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indentX);

					if (ImGui::Button(buttonLabel.c_str(), { ImGui::GetWindowWidth() - indentX, BUTTON_HEIGHT }))
					{
						if (isExpanded)
							expandedCategories.erase(currentPath);
						else
							expandedCategories.insert(currentPath);
					}
				}

				// Check if category is collapsed
				if (expandedCategories.find(currentPath) == expandedCategories.end())
				{
					allParentsExpanded = false;
					break;
				}
			}

			// Draw condition item if all parents are expanded
			if (allParentsExpanded)
			{
				float indentX = INDENT_SIZE * condition.category.size();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indentX);

				if (ImGui::Button(condition.stringType.c_str(), { ImGui::GetWindowWidth() - indentX, BUTTON_HEIGHT }))
					tempConditionData = condition;
			}
		}
	}
	else
	{
		const auto& actions = EventSheetSystem::GetAllActions();

		for (const EventSheetSystem::Action& action : actions)
		{
			std::string currentPath;
			bool allParentsExpanded = true;

			// Draw category hierarchy
			for (size_t i = 0; i < action.category.size(); ++i)
			{
				if (!currentPath.empty())
					currentPath += " > ";
				currentPath += action.category[i];

				if (drawnCategories.insert(currentPath).second)
				{
					bool isExpanded = expandedCategories.find(currentPath) != expandedCategories.end();
					std::string buttonLabel = (isExpanded ? ICON_FA_ANGLE_DOWN : ICON_FA_ANGLE_RIGHT);
					buttonLabel += " " + action.category[i];

					float indentX = INDENT_SIZE * i;
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indentX);

					if (ImGui::Button(buttonLabel.c_str(), { ImGui::GetWindowWidth() - indentX, BUTTON_HEIGHT }))
					{
						if (isExpanded)
							expandedCategories.erase(currentPath);
						else
							expandedCategories.insert(currentPath);
					}
				}

				if (expandedCategories.find(currentPath) == expandedCategories.end())
				{
					allParentsExpanded = false;
					break;
				}
			}

			// Draw action item if all parents are expanded
			if (allParentsExpanded)
			{
				float indentX = INDENT_SIZE * action.category.size();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indentX);

				if (ImGui::Button(action.stringType.c_str(), { ImGui::GetWindowWidth() - indentX, BUTTON_HEIGHT }))
					tempActionData = action;
			}
		}
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleColor(3);
}

void EventSheetEditor::RenderParameterEditor()
{
	bool hasData = !tempConditionData.stringType.empty() || !tempActionData.stringType.empty();
	if (!hasData)
		return;

	ImGui::SetCursorPos({ ImGui::GetWindowWidth() / 2, 0 });
	ImGui::BeginChild("ParameterWin", { ImGui::GetWindowWidth() / 2, ImGui::GetWindowHeight() - 60 });

	ImGui::PushFont(FontManager::GetFont("Familiar-Pro-Bold", 30, false));
	ImGui::SetCursorPos({ 10, 10 });
	ImGui::Text(!tempConditionData.stringType.empty() ? tempConditionData.stringType.c_str() : tempActionData.stringType.c_str());
	ImGui::PopFont();

	std::vector<EventSheetSystem::Param>* params = !tempConditionData.stringType.empty() ?
		&tempConditionData.params : &tempActionData.params;

	float yPos = 60;
	for (int i = 0; i < static_cast<int>(params->size()); ++i)
	{
		EventSheetSystem::Param& param = (*params)[i];
		ImGui::SetCursorPos({ 10, yPos });
		ImGui::Text((param.name + ":").c_str());

		RenderParameterInput(param, i);
		yPos += 30;
	}

	ImGui::EndChild();
}

void EventSheetEditor::RenderParameterInput(EventSheetSystem::Param& param, int index)
{
	const std::string id = "##param" + param.name + std::to_string(index);

	// Special parameter handlers
	if (param.name == "Key" && param.type == "string")
	{
		RenderKeySelector(param);
	}
	else if (param.name == "Button" && param.type == "int")
	{
		bool isGamepad = (!tempConditionData.category.empty() && tempConditionData.category.back() == "Gamepad") ||
			(!tempActionData.category.empty() && tempActionData.category.back() == "Gamepad");

		if (isGamepad)
			RenderGamepadButtonSelector(param);
		else
			RenderMouseButtonSelector(param);
	}
	else if (param.name == "Axis" && param.type == "int")
	{
		RenderGamepadAxisSelector(param);
	}
	else if (param.name == "GameObject" && param.type == "string")
	{
		RenderGameObjectSelector(param);
	}
	// Standard parameter types
	else if (param.type == "string")
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - ImGui::CalcTextSize((param.name + ":").c_str()).x - 30);

		char inputBuffer[256] = "";
		strncpy_s(inputBuffer, std::any_cast<std::string>(param.value).c_str(), sizeof(inputBuffer) - 1);
		if (ImGui::InputText(id.c_str(), inputBuffer, sizeof(inputBuffer)))
			param.value = std::string(inputBuffer);
	}
	else if (param.type == "int")
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - ImGui::CalcTextSize((param.name + ":").c_str()).x - 30);

		int value = std::any_cast<int>(param.value);
		if (ImGui::InputInt(id.c_str(), &value))
			param.value = value;
	}
	else if (param.type == "float")
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - ImGui::CalcTextSize((param.name + ":").c_str()).x - 30);

		float value = std::any_cast<float>(param.value);
		if (ImGui::InputFloat(id.c_str(), &value, 0.1f, 1.0f, "%.2f"))
			param.value = value;
	}
	else if (param.type == "bool")
	{
		ImGui::SameLine();
		bool value = std::any_cast<bool>(param.value);
		if (ImGui::Checkbox(id.c_str(), &value))
			param.value = value;
	}
}

void EventSheetEditor::RenderKeySelector(EventSheetSystem::Param& param)
{
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);

	std::string buttonText = !std::any_cast<std::string>(param.value).empty() ?
		(std::any_cast<std::string>(param.value) + " - Click To Change") : "Select Key";

	if (ImGui::Button(buttonText.c_str()))
		ImGui::OpenPopup("KeySelector");

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);

	if (ImGui::BeginPopup("KeySelector"))
	{
		auto AddKeyMenu = [&](const char* label, const std::vector<std::string>& keys)
			{
				if (ImGui::BeginMenu(label))
				{
					for (const auto& key : keys)
					{
						if (ImGui::Selectable(key.c_str()))
							param.value = key;
					}
					ImGui::EndMenu();
				}
			};

		AddKeyMenu("Letters", [] {
			std::vector<std::string> letters;
			for (char c = 'A'; c <= 'Z'; ++c)
				letters.push_back(std::string(1, c));
			return letters;
			}());

		AddKeyMenu("Numbers", [] {
			std::vector<std::string> numbers;
			for (char c = '0'; c <= '9'; ++c)
				numbers.push_back(std::string(1, c));
			return numbers;
			}());

		AddKeyMenu("Function Keys", [] {
			std::vector<std::string> fkeys;
			for (int i = 1; i <= 12; ++i)
				fkeys.push_back("F" + std::to_string(i));
			return fkeys;
			}());

		AddKeyMenu("Modifier Keys", {
			"SHIFT", "LEFT_SHIFT", "RIGHT_SHIFT",
			"CTRL", "LEFT_CTRL", "RIGHT_CTRL",
			"ALT", "LEFT_ALT", "RIGHT_ALT",
			"CAPSLOCK", "TAB"
			});

		AddKeyMenu("Arrow Keys", {
			"UP", "DOWN", "LEFT", "RIGHT"
			});

		AddKeyMenu("Control Keys", {
			"ENTER", "ESCAPE", "SPACE", "BACKSPACE", "DELETE",
			"INSERT", "HOME", "END", "PAGEUP", "PAGEDOWN",
			"PRINTSCREEN", "PAUSE", "NUMLOCK", "SCROLLLOCK"
			});

		AddKeyMenu("Numpad", {
			"NUMPAD_0", "NUMPAD_1", "NUMPAD_2", "NUMPAD_3", "NUMPAD_4",
			"NUMPAD_5", "NUMPAD_6", "NUMPAD_7", "NUMPAD_8", "NUMPAD_9",
			"NUMPAD_ADD", "NUMPAD_SUBTRACT", "NUMPAD_MULTIPLY", "NUMPAD_DIVIDE",
			"NUMPAD_DECIMAL", "NUMPAD_ENTER"
			});

		AddKeyMenu("Symbols", {
			"`", "-", "=", "[", "]", "\\", ";", "'", ",", ".", "/"
			});

		ImGui::EndPopup();
	}
}

void EventSheetEditor::RenderMouseButtonSelector(EventSheetSystem::Param& param)
{
	ImGui::SameLine();

	static const char* buttonNames[] = { "Left (0)", "Right (1)", "Middle (2)" };
	int currentValue = std::any_cast<int>(param.value);
	const char* currentText = (currentValue >= 0 && currentValue <= 2) ?
		buttonNames[currentValue] : "Select Button";

	if (ImGui::Button((std::string(currentText) + " - Click To Change").c_str()))
		ImGui::OpenPopup("MouseButtonSelector");

	if (ImGui::BeginPopup("MouseButtonSelector"))
	{
		for (int i = 0; i < 3; i++)
		{
			if (ImGui::Selectable(buttonNames[i]))
				param.value = i;
		}
		ImGui::EndPopup();
	}
}

void EventSheetEditor::RenderGamepadButtonSelector(EventSheetSystem::Param& param)
{
	ImGui::SameLine();

	static const char* buttonNames[] = {
		"A/Cross (0)", "B/Circle (1)", "X/Square (2)", "Y/Triangle (3)",
		"Left Shoulder (4)", "Right Shoulder (5)",
		"Back/Select (6)", "Start (7)", "Left Thumb (8)", "Right Thumb (9)"
	};

	int currentValue = std::any_cast<int>(param.value);
	const char* currentText = (currentValue >= 0 && currentValue <= 9) ?
		buttonNames[currentValue] : "Select Button";

	if (ImGui::Button((std::string(currentText) + " - Click To Change").c_str()))
		ImGui::OpenPopup("GamepadButtonSelector");

	if (ImGui::BeginPopup("GamepadButtonSelector"))
	{
		for (int i = 0; i < 10; i++)
		{
			if (ImGui::Selectable(buttonNames[i]))
				param.value = i;
		}
		ImGui::EndPopup();
	}
}

void EventSheetEditor::RenderGamepadAxisSelector(EventSheetSystem::Param& param)
{
	ImGui::SameLine();

	static const char* axisNames[] = {
		"Left X (0)", "Left Y (1)", "Right X (2)", "Right Y (3)",
		"Left Trigger (4)", "Right Trigger (5)"
	};

	int currentValue = std::any_cast<int>(param.value);
	const char* currentText = (currentValue >= 0 && currentValue <= 5) ?
		axisNames[currentValue] : "Select Axis";

	if (ImGui::Button((std::string(currentText) + " - Click To Change").c_str()))
		ImGui::OpenPopup("GamepadAxisSelector");

	if (ImGui::BeginPopup("GamepadAxisSelector"))
	{
		for (int i = 0; i < 6; i++)
		{
			if (ImGui::Selectable(axisNames[i]))
				param.value = i;
		}
		ImGui::EndPopup();
	}
}

void EventSheetEditor::RenderGameObjectSelector(EventSheetSystem::Param& param)
{
	ImGui::SameLine();

	std::string currentValue = std::any_cast<std::string>(param.value);
	std::string buttonText = !currentValue.empty() ?
		(currentValue + " - Click To Change") : "Select GameObject";

	if (ImGui::Button(buttonText.c_str()))
		ImGui::OpenPopup("GameObjectSelector");

	if (ImGui::BeginPopup("GameObjectSelector"))
	{
		if (ImGui::Selectable("self"))
			param.value = std::string("self");

		// Todo: Add list of GameObjects in scene
		ImGui::EndPopup();
	}
}

void EventSheetEditor::ResetSelection()
{
	tempConditionData = {};
	tempActionData = {};
	expandedCategories.clear();
	eventSelected = -1;
	conditionSelected = -1;
	actionSelected = -1;
	selectorFirstFrame = true;
}

void EventSheetEditor::LoadData(nlohmann::json data, std::string path)
{
	filePath = path;

	if (!data.contains("events") || !data["events"].is_array())
		return;

	auto convertFromJson = [](const nlohmann::json& jsonValue) -> std::any {
		if (jsonValue.is_boolean())
			return std::any(jsonValue.get<bool>());
		if (jsonValue.is_number_integer())
			return std::any(jsonValue.get<int>());
		if (jsonValue.is_number_float())
			return std::any(jsonValue.get<float>());
		if (jsonValue.is_string())
			return std::any(jsonValue.get<std::string>());
		return std::any();
		};

	events.clear(); // Clear existing events before loading

	for (const auto& eventJson : data["events"])
	{
		EventSheetSystem::Event event;

		if (eventJson.contains("conditions") && eventJson["conditions"].is_array())
		{
			for (const auto& conditionJson : eventJson["conditions"])
			{
				EventSheetSystem::Condition condition;
				condition.type = static_cast<EventSheetSystem::Conditions>(conditionJson.value("type", 0));
				condition.stringType = conditionJson.value("stringType", "");

				if (conditionJson.contains("category") && conditionJson["category"].is_array())
				{
					for (const auto& cat : conditionJson["category"])
						condition.category.push_back(cat.get<std::string>());
				}

				if (conditionJson.contains("params") && conditionJson["params"].is_array())
				{
					for (const auto& paramJson : conditionJson["params"])
					{
						condition.params.push_back({
							paramJson.value("name", ""),
							paramJson.value("type", ""),
							convertFromJson(paramJson["value"])
							});
					}
				}

				event.conditions.push_back(condition);
			}
		}

		if (eventJson.contains("actions") && eventJson["actions"].is_array())
		{
			for (const auto& actionJson : eventJson["actions"])
			{
				EventSheetSystem::Action action;
				action.type = static_cast<EventSheetSystem::Actions>(actionJson.value("type", 0));
				action.stringType = actionJson.value("stringType", "");

				if (actionJson.contains("category") && actionJson["category"].is_array())
				{
					for (const auto& cat : actionJson["category"])
						action.category.push_back(cat.get<std::string>());
				}

				if (actionJson.contains("params") && actionJson["params"].is_array())
				{
					for (const auto& paramJson : actionJson["params"])
					{
						action.params.push_back({
							paramJson.value("name", ""),
							paramJson.value("type", ""),
							convertFromJson(paramJson["value"])
							});
					}
				}

				event.actions.push_back(action);
			}
		}

		events.push_back(event);
	}
}

void EventSheetEditor::SaveData()
{
	nlohmann::json json = {
		{"version", 1},
		{"path", filePath},
		{"events", nlohmann::json::array()}
	};

	auto convertToJson = [](const std::any& value) -> nlohmann::json {
		if (value.type() == typeid(bool))
			return std::any_cast<bool>(value);
		if (value.type() == typeid(int))
			return std::any_cast<int>(value);
		if (value.type() == typeid(float))
			return std::any_cast<float>(value);
		if (value.type() == typeid(std::string))
			return std::any_cast<std::string>(value);
		return nullptr;
		};

	for (const EventSheetSystem::Event& event : events)
	{
		nlohmann::json eventJson;
		eventJson["conditions"] = nlohmann::json::array();
		eventJson["actions"] = nlohmann::json::array();

		for (const EventSheetSystem::Condition& condition : event.conditions)
		{
			nlohmann::json conditionJson;
			conditionJson["type"] = static_cast<int>(condition.type);
			conditionJson["stringType"] = condition.stringType;
			conditionJson["category"] = condition.category;
			conditionJson["params"] = nlohmann::json::array();

			for (const auto& param : condition.params)
			{
				nlohmann::json paramJson;
				paramJson["name"] = param.name;
				paramJson["type"] = param.type;
				paramJson["value"] = convertToJson(param.value);
				conditionJson["params"].push_back(paramJson);
			}

			eventJson["conditions"].push_back(conditionJson);
		}

		for (const EventSheetSystem::Action& action : event.actions)
		{
			nlohmann::json actionJson;
			actionJson["type"] = static_cast<int>(action.type);
			actionJson["stringType"] = action.stringType;
			actionJson["category"] = action.category;
			actionJson["params"] = nlohmann::json::array();

			for (const auto& param : action.params)
			{
				nlohmann::json paramJson;
				paramJson["name"] = param.name;
				paramJson["type"] = param.type;
				paramJson["value"] = convertToJson(param.value);
				actionJson["params"].push_back(paramJson);
			}

			eventJson["actions"].push_back(actionJson);
		}

		json["events"].push_back(eventJson);
	}

	std::ofstream file(filePath, std::ofstream::trunc);
	if (file.is_open())
	{
		file << json.dump(4);
		file.close();
	}
	else
	{
		ConsoleLogger::ErrorLog("There was an error saving your " +
			std::filesystem::path(filePath).stem().string() + " event sheet.");
		// Todo: If its being saved from OnClose(), give a popup saying there was an issue saving it and if they would like to try and save again or force close.
	}
}

void EventSheetEditor::OnClose()
{
	// Todo: Ask if the user would like to save before closing
	SaveData();
}

// Define the static condition and action lists
namespace EventSheetSystem
{
	const std::vector<Condition>& GetAllConditions()
	{
		static const std::vector<Condition> conditions = {
			// Input - Keyboard
			{ Conditions::AnyKeyPressed, "Any Key Pressed", {"Input", "Keyboard"}, {} },
			{ Conditions::AnyKeyReleased, "Any Key Released", {"Input", "Keyboard"}, {} },
			{ Conditions::KeyPressed, "Key Pressed", {"Input", "Keyboard"}, { {"Key", "string", std::string("")} }},
			{ Conditions::KeyReleased, "Key Released", {"Input", "Keyboard"}, { {"Key", "string", std::string("")} }},
			{ Conditions::KeyDown, "Key Down", {"Input", "Keyboard"}, { {"Key", "string", std::string("")} }},

			// Input - Mouse
			{ Conditions::MouseButtonPressed, "Mouse Button Pressed", {"Input", "Mouse"}, { {"Button", "int", 0} }},
			{ Conditions::MouseButtonReleased, "Mouse Button Released", {"Input", "Mouse"}, { {"Button", "int", 0} }},
			{ Conditions::MouseButtonDown, "Mouse Button Down", {"Input", "Mouse"}, { {"Button", "int", 0} }},
			{ Conditions::MouseMoved, "Mouse Moved", {"Input", "Mouse"}, {} },
			{ Conditions::MouseInZone, "Mouse In Zone", {"Input", "Mouse"}, {
				{"X", "float", 0.0f}, {"Y", "float", 0.0f}, {"Width", "float", 100.0f}, {"Height", "float", 100.0f}
			}},
			{ Conditions::MouseOverObject, "Mouse Over Object", {"Input", "Mouse"}, { {"GameObject", "string", std::string("self")} }},
			{ Conditions::MouseX, "Compare Mouse X", {"Input", "Mouse"}, { {"Comparison", "string", std::string("==")}, {"Value", "float", 0.0f} }},
			{ Conditions::MouseY, "Compare Mouse Y", {"Input", "Mouse"}, { {"Comparison", "string", std::string("==")}, {"Value", "float", 0.0f} }},

			// Input - Gamepad
			{ Conditions::GamepadButtonPressed, "Gamepad Button Pressed", {"Input", "Gamepad"}, {
				{"Gamepad", "int", 0}, {"Button", "int", 0}
			}},
			{ Conditions::GamepadButtonReleased, "Gamepad Button Released", {"Input", "Gamepad"}, {
				{"Gamepad", "int", 0}, {"Button", "int", 0}
			}},
			{ Conditions::GamepadButtonDown, "Gamepad Button Down", {"Input", "Gamepad"}, {
				{"Gamepad", "int", 0}, {"Button", "int", 0}
			}},
			{ Conditions::GamepadAxisMoved, "Gamepad Axis Moved", {"Input", "Gamepad"}, {
				{"Gamepad", "int", 0}, {"Axis", "int", 0}, {"MinValue", "float", -1.0f}, {"MaxValue", "float", 1.0f}
			}},
			{ Conditions::GamepadConnected, "Gamepad Connected", {"Input", "Gamepad"}, { {"Gamepad", "int", 0} }},
			{ Conditions::GamepadDisconnected, "Gamepad Disconnected", {"Input", "Gamepad"}, { {"Gamepad", "int", 0} }},

			// GameObject - Transform
			{ Conditions::ComparePosition, "Compare Position", {"GameObject", "Transform"}, {
				{"GameObject", "string", std::string("self")}, {"X", "float", 0.0f}, {"Y", "float", 0.0f}, {"Z", "float", 0.0f}, {"Tolerance", "float", 0.1f}
			}},
			{ Conditions::CompareX, "Compare X Position", {"GameObject", "Transform"}, {
				{"GameObject", "string", std::string("self")}, {"Comparison", "string", std::string("==")}, {"Value", "float", 0.0f}
			}},
			{ Conditions::CompareY, "Compare Y Position", {"GameObject", "Transform"}, {
				{"GameObject", "string", std::string("self")}, {"Comparison", "string", std::string("==")}, {"Value", "float", 0.0f}
			}},
			{ Conditions::CompareZ, "Compare Z Position", {"GameObject", "Transform"}, {
				{"GameObject", "string", std::string("self")}, {"Comparison", "string", std::string("==")}, {"Value", "float", 0.0f}
			}},
			{ Conditions::CompareRotation, "Compare Rotation", {"GameObject", "Transform"}, {
				{"GameObject", "string", std::string("self")}, {"Comparison", "string", std::string("==")}, {"Angle", "float", 0.0f}
			}},
			{ Conditions::CompareScale, "Compare Scale", {"GameObject", "Transform"}, {
				{"GameObject", "string", std::string("self")}, {"Comparison", "string", std::string("==")}, {"Scale", "float", 1.0f}
			}},
			{ Conditions::CompareDistance, "Compare Distance To", {"GameObject", "Transform"}, {
				{"Object1", "string", std::string("self")}, {"Object2", "string", std::string("")}, {"Comparison", "string", std::string("<")}, {"Distance", "float", 10.0f}
			}},
			{ Conditions::IsVisible, "Is Visible", {"GameObject", "Transform"}, { {"GameObject", "string", std::string("self")} }},

			// GameObject - Properties
			{ Conditions::IsActive, "Is Active", {"GameObject", "Properties"}, { {"GameObject", "string", std::string("self")} }},
			{ Conditions::HasTag, "Has Tag", {"GameObject", "Properties"}, {
				{"GameObject", "string", std::string("self")}, {"Tag", "string", std::string("")}
			}},
			{ Conditions::HasComponent, "Has Component", {"GameObject", "Properties"}, {
				{"GameObject", "string", std::string("self")}, {"Component", "string", std::string("")}
			}},
			{ Conditions::CompareLayer, "Compare Layer", {"GameObject", "Properties"}, {
				{"GameObject", "string", std::string("self")}, {"Layer", "int", 0}
			}},
			{ Conditions::CompareName, "Compare Name", {"GameObject", "Properties"}, {
				{"GameObject", "string", std::string("self")}, {"Name", "string", std::string("")}
			}},

			// Physics
			{ Conditions::IsColliding, "Is Colliding With", {"Physics"}, {
				{"GameObject", "string", std::string("self")}, {"OtherObject", "string", std::string("")}
			}},
			{ Conditions::HasRigidbody, "Has Rigidbody", {"Physics"}, { {"GameObject", "string", std::string("self")} }},
			{ Conditions::CompareVelocity, "Compare Velocity", {"Physics"}, {
				{"GameObject", "string", std::string("self")}, {"Comparison", "string", std::string(">")}, {"Speed", "float", 0.0f}
			}},
			{ Conditions::IsGrounded, "Is Grounded", {"Physics"}, { {"GameObject", "string", std::string("self")} }},
			{ Conditions::IsTrigger, "Is Trigger", {"Physics"}, { {"GameObject", "string", std::string("self")} }},

			// Time
			{ Conditions::TimerElapsed, "Timer Elapsed", {"Time"}, { {"Timer", "string", std::string("timer1")}, {"Duration", "float", 1.0f} }},
			{ Conditions::EveryXSeconds, "Every X Seconds", {"Time"}, { {"Interval", "float", 1.0f} }},
			{ Conditions::CompareTime, "Compare Time", {"Time"}, { {"Comparison", "string", std::string(">")}, {"Value", "float", 0.0f} }},
			{ Conditions::CompareDeltaTime, "Compare Delta Time", {"Time"}, { {"Comparison", "string", std::string(">")}, {"Value", "float", 0.016f} }},

			// Logic
			{ Conditions::CompareVariable, "Compare Variable", {"Logic"}, {
				{"Variable", "string", std::string("")}, {"Comparison", "string", std::string("==")}, {"Value", "float", 0.0f}
			}},
			{ Conditions::IsTrue, "Is True", {"Logic"}, { {"Variable", "string", std::string("")} }},
			{ Conditions::IsFalse, "Is False", {"Logic"}, { {"Variable", "string", std::string("")} }},

			// System
			{ Conditions::OnStart, "On Start", {"System"}, {} },
			{ Conditions::OnUpdate, "On Update", {"System"}, {} },
			{ Conditions::FrameCount, "Compare Frame Count", {"System"}, { {"Comparison", "string", std::string("==")}, {"Frame", "int", 0} }},
			{ Conditions::PlatformIs, "Platform Is", {"System"}, { {"Platform", "string", std::string("Windows")} }},

			// Audio
			{ Conditions::IsSoundPlaying, "Is Sound Playing", {"Audio"}, { {"Sound", "string", std::string("")} }},
			{ Conditions::IsMusicPlaying, "Is Music Playing", {"Audio"}, {} },
			{ Conditions::CompareVolume, "Compare Volume", {"Audio"}, { {"Comparison", "string", std::string("==")}, {"Volume", "float", 1.0f} }},

			// Scene
			{ Conditions::SceneIs, "Scene Is", {"Scene"}, { {"SceneName", "string", std::string("")} }},
			{ Conditions::ObjectExists, "Object Exists", {"Scene"}, { {"GameObject", "string", std::string("")} }},
			{ Conditions::ObjectCount, "Compare Object Count", {"Scene"}, {
				{"Tag", "string", std::string("")}, {"Comparison", "string", std::string("==")}, {"Count", "int", 0}
			}},

			// Rendering
			{ Conditions::IsOnScreen, "Is On Screen", {"Rendering"}, { {"GameObject", "string", std::string("self")} }},
			{ Conditions::CompareOpacity, "Compare Opacity", {"Rendering"}, {
				{"GameObject", "string", std::string("self")}, {"Comparison", "string", std::string("==")}, {"Opacity", "float", 1.0f}
			}},
			{ Conditions::CompareColor, "Compare Color", {"Rendering"}, {
				{"GameObject", "string", std::string("self")}, {"R", "float", 1.0f}, {"G", "float", 1.0f}, {"B", "float", 1.0f}
			}},

			// Animation
			{ Conditions::IsAnimationPlaying, "Is Animation Playing", {"Animation"}, {
				{"GameObject", "string", std::string("self")}, {"Animation", "string", std::string("")}
			}},
			{ Conditions::AnimationFinished, "Animation Finished", {"Animation"}, {
				{"GameObject", "string", std::string("self")}, {"Animation", "string", std::string("")}
			}},
			{ Conditions::CurrentFrame, "Compare Current Frame", {"Animation"}, {
				{"GameObject", "string", std::string("self")}, {"Comparison", "string", std::string("==")}, {"Frame", "int", 0}
			}},

			// UI
			{ Conditions::ButtonClicked, "Button Clicked", {"UI"}, { {"ButtonName", "string", std::string("")} }},
			{ Conditions::ButtonHovered, "Button Hovered", {"UI"}, { {"ButtonName", "string", std::string("")} }},
			{ Conditions::TextInputChanged, "Text Input Changed", {"UI"}, { {"InputName", "string", std::string("")} }},
			{ Conditions::SliderChanged, "Slider Changed", {"UI"}, { {"SliderName", "string", std::string("")} }},

			// Math
			{ Conditions::CompareNumbers, "Compare Numbers", {"Math"}, {
				{"Value1", "float", 0.0f}, {"Comparison", "string", std::string("==")}, {"Value2", "float", 0.0f}
			}},
			{ Conditions::IsEven, "Is Even", {"Math"}, { {"Value", "int", 0} }},
			{ Conditions::IsOdd, "Is Odd", {"Math"}, { {"Value", "int", 0} }},
			{ Conditions::InRange, "In Range", {"Math"}, {
				{"Value", "float", 0.0f}, {"Min", "float", 0.0f}, {"Max", "float", 100.0f}
			}},
			{ Conditions::RandomChance, "Random Chance", {"Math"}, { {"Percentage", "float", 50.0f} }},
		};

		return conditions;
	}

	const std::vector<Action>& GetAllActions()
	{
		static const std::vector<Action> actions = {
			// Transform - Position
			{ Actions::Position, "Set Position", {"Transform", "Position"}, {
				{"GameObject", "string", std::string("self")}, {"X", "float", 0.0f}, {"Y", "float", 0.0f}, {"Z", "float", 0.0f}
			}},
			{ Actions::MovePosition, "Move Position", {"Transform", "Position"}, {
				{"GameObject", "string", std::string("self")}, {"X", "float", 0.0f}, {"Y", "float", 0.0f}, {"Z", "float", 0.0f}
			}},
			{ Actions::XPosition, "Set X Position", {"Transform", "Position"}, {
				{"GameObject", "string", std::string("self")}, {"X", "float", 0.0f}
			}},
			{ Actions::YPosition, "Set Y Position", {"Transform", "Position"}, {
				{"GameObject", "string", std::string("self")}, {"Y", "float", 0.0f}
			}},
			{ Actions::ZPosition, "Set Z Position", {"Transform", "Position"}, {
				{"GameObject", "string", std::string("self")}, {"Z", "float", 0.0f}
			}},
			{ Actions::MoveTowards, "Move Towards", {"Transform", "Position"}, {
				{"GameObject", "string", std::string("self")}, {"Target", "string", std::string("")}, {"Speed", "float", 1.0f}
			}},
			{ Actions::LookAt, "Look At", {"Transform", "Position"}, {
				{"GameObject", "string", std::string("self")}, {"Target", "string", std::string("")}
			}},

			// Transform - Rotation
			{ Actions::SetRotation, "Set Rotation", {"Transform", "Rotation"}, {
				{"GameObject", "string", std::string("self")}, {"X", "float", 0.0f}, {"Y", "float", 0.0f}, {"Z", "float", 0.0f}
			}},
			{ Actions::SetRotationX, "Set X Rotation", {"Transform", "Rotation"}, {
				{"GameObject", "string", std::string("self")}, {"X", "float", 0.0f}
			}},
			{ Actions::SetRotationY, "Set Y Rotation", {"Transform", "Rotation"}, {
				{"GameObject", "string", std::string("self")}, {"Y", "float", 0.0f}
			}},
			{ Actions::SetRotationZ, "Set Z Rotation", {"Transform", "Rotation"}, {
				{"GameObject", "string", std::string("self")}, {"Z", "float", 0.0f}
			}},
			{ Actions::Rotate, "Rotate", {"Transform", "Rotation"}, {
				{"GameObject", "string", std::string("self")}, {"X", "float", 0.0f}, {"Y", "float", 0.0f}, {"Z", "float", 0.0f}
			}},
			{ Actions::RotateTowards, "Rotate Towards", {"Transform", "Rotation"}, {
				{"GameObject", "string", std::string("self")}, {"Target", "string", std::string("")}, {"Speed", "float", 1.0f}
			}},

			// Transform - Scale
			{ Actions::SetScale, "Set Scale", {"Transform", "Scale"}, {
				{"GameObject", "string", std::string("self")}, {"X", "float", 1.0f}, {"Y", "float", 1.0f}, {"Z", "float", 1.0f}
			}},
			{ Actions::SetScaleX, "Set X Scale", {"Transform", "Scale"}, {
				{"GameObject", "string", std::string("self")}, {"X", "float", 1.0f}
			}},
			{ Actions::SetScaleY, "Set Y Scale", {"Transform", "Scale"}, {
				{"GameObject", "string", std::string("self")}, {"Y", "float", 1.0f}
			}},
			{ Actions::SetScaleZ, "Set Z Scale", {"Transform", "Scale"}, {
				{"GameObject", "string", std::string("self")}, {"Z", "float", 1.0f}
			}},
			{ Actions::ScaleBy, "Scale By", {"Transform", "Scale"}, {
				{"GameObject", "string", std::string("self")}, {"Factor", "float", 1.0f}
			}},

			// GameObject
			{ Actions::SetActive, "Set Active", {"GameObject"}, {
				{"GameObject", "string", std::string("self")}, {"Active", "bool", true}
			}},
			{ Actions::Destroy,"Destroy", {"GameObject"}, {
				{"GameObject", "string", std::string("self")}, {"Delay", "float", 0.0f}
			}},
			{ Actions::Instantiate, "Instantiate", {"GameObject"}, {
				{"Prefab", "string", std::string("")}, {"X", "float", 0.0f}, {"Y", "float", 0.0f}, {"Z", "float", 0.0f}
			}},
			{ Actions::SetName, "Set Name", {"GameObject"}, {
				{"GameObject", "string", std::string("self")}, {"Name", "string", std::string("")}
			}},
			{ Actions::SetTag, "Set Tag", {"GameObject"}, {
				{"GameObject", "string", std::string("self")}, {"Tag", "string", std::string("")}
			}},
			{ Actions::SetLayer, "Set Layer", {"GameObject"}, {
				{"GameObject", "string", std::string("self")}, {"Layer", "int", 0}
			}},
			{ Actions::AddComponent, "Add Component", {"GameObject"}, {
				{"GameObject", "string", std::string("self")}, {"Component", "string", std::string("")}
			}},
			{ Actions::RemoveComponent, "Remove Component", {"GameObject"}, {
				{"GameObject", "string", std::string("self")}, {"Component", "string", std::string("")}
			}},
			{ Actions::SetParent, "Set Parent", {"GameObject"}, {
				{"GameObject", "string", std::string("self")}, {"Parent", "string", std::string("")}
			}},

			// Physics
			{ Actions::SetVelocity, "Set Velocity", {"Physics"}, {
				{"GameObject", "string", std::string("self")}, {"X", "float", 0.0f}, {"Y", "float", 0.0f}, {"Z", "float", 0.0f}
			}},
			{ Actions::AddForce, "Add Force", {"Physics"}, {
				{"GameObject", "string", std::string("self")}, {"X", "float", 0.0f}, {"Y", "float", 0.0f}, {"Z", "float", 0.0f}
			}},
			{ Actions::SetGravity, "Set Gravity", {"Physics"}, {
				{"GameObject", "string", std::string("self")}, {"UseGravity", "bool", true}
			}},
			{ Actions::SetMass, "Set Mass", {"Physics"}, {
				{"GameObject", "string", std::string("self")}, {"Mass", "float", 1.0f}
			}},
			{ Actions::SetDrag, "Set Drag", {"Physics"}, {
				{"GameObject", "string", std::string("self")}, {"Drag", "float", 0.0f}
			}},
			{ Actions::SetAngularVelocity, "Set Angular Velocity", {"Physics"}, {
				{"GameObject", "string", std::string("self")}, {"X", "float", 0.0f}, {"Y", "float", 0.0f}, {"Z", "float", 0.0f}
			}},
			{ Actions::AddTorque, "Add Torque", {"Physics"}, {
				{"GameObject", "string", std::string("self")}, {"X", "float", 0.0f}, {"Y", "float", 0.0f}, {"Z", "float", 0.0f}
			}},
			{ Actions::FreezePosition, "Freeze Position", {"Physics"}, {
				{"GameObject", "string", std::string("self")}, {"X", "bool", false}, {"Y", "bool", false}, {"Z", "bool", false}
			}},
			{ Actions::FreezeRotation, "Freeze Rotation", {"Physics"}, {
				{"GameObject", "string", std::string("self")}, {"X", "bool", false}, {"Y", "bool", false}, {"Z", "bool", false}
			}},

			// Rendering
			{ Actions::SetColor, "Set Color", {"Rendering"}, {
				{"GameObject", "string", std::string("self")}, {"R", "float", 1.0f}, {"G", "float", 1.0f}, {"B", "float", 1.0f}, {"A", "float", 1.0f}
			}},
			{ Actions::SetOpacity, "Set Opacity", {"Rendering"}, {
				{"GameObject", "string", std::string("self")}, {"Opacity", "float", 1.0f}
			}},
			{ Actions::SetSprite, "Set Sprite", {"Rendering"}, {
				{"GameObject", "string", std::string("self")}, {"Sprite", "string", std::string("")}
			}},
			{ Actions::SetMaterial, "Set Material", {"Rendering"}, {
				{"GameObject", "string", std::string("self")}, {"Material", "string", std::string("")}
			}},
			{ Actions::FlipX, "Flip X", {"Rendering"}, {
				{"GameObject", "string", std::string("self")}, {"Flipped", "bool", true}
			}},
			{ Actions::FlipY, "Flip Y", {"Rendering"}, {
				{"GameObject", "string", std::string("self")}, {"Flipped", "bool", true}
			}},
			{ Actions::SetSortingOrder, "Set Sorting Order", {"Rendering"}, {
				{"GameObject", "string", std::string("self")}, {"Order", "int", 0}
			}},
			{ Actions::SetVisible, "Set Visible", {"Rendering"}, {
				{"GameObject", "string", std::string("self")}, {"Visible", "bool", true}
			}},

			// Animation
			{ Actions::PlayAnimation, "Play Animation", {"Animation"}, {
				{"GameObject", "string", std::string("self")}, {"Animation", "string", std::string("")}
			}},
			{ Actions::StopAnimation, "Stop Animation", {"Animation"}, {
				{"GameObject", "string", std::string("self")}
			}},
			{ Actions::PauseAnimation, "Pause Animation", {"Animation"}, {
				{"GameObject", "string", std::string("self")}
			}},
			{ Actions::SetAnimationSpeed, "Set Animation Speed", {"Animation"}, {
				{"GameObject", "string", std::string("self")}, {"Speed", "float", 1.0f}
			}},
			{ Actions::SetFrame, "Set Frame", {"Animation"}, {
				{"GameObject", "string", std::string("self")}, {"Frame", "int", 0}
			}},

			// Audio
			{ Actions::PlaySound, "Play Sound", {"Audio"}, {
				{"Sound", "string", std::string("")}, {"Volume", "float", 1.0f}, {"Loop", "bool", false}
			}},
			{ Actions::StopSound, "Stop Sound", {"Audio"}, {
				{"Sound", "string", std::string("")}
			}},
			{ Actions::PauseSound, "Pause Sound", {"Audio"}, {
				{"Sound", "string", std::string("")}
			}},
			{ Actions::ResumeSound, "Resume Sound", {"Audio"}, {
				{"Sound", "string", std::string("")}
			}},
			{ Actions::SetVolume, "Set Volume", {"Audio"}, {
				{"Sound", "string", std::string("")}, {"Volume", "float", 1.0f}
			}},
			{ Actions::SetPitch, "Set Pitch", {"Audio"}, {
				{"Sound", "string", std::string("")}, {"Pitch", "float", 1.0f}
			}},
			{ Actions::PlayMusic, "Play Music", {"Audio"}, {
				{"Music", "string", std::string("")}, {"Volume", "float", 1.0f}, {"FadeIn", "float", 0.0f}
			}},
			{ Actions::StopMusic, "Stop Music", {"Audio"}, {
				{"FadeOut", "float", 0.0f}
			}},
			{ Actions::FadeIn, "Fade In Audio", {"Audio"}, {
				{"Sound", "string", std::string("")}, {"Duration", "float", 1.0f}
			}},
			{ Actions::FadeOut, "Fade Out Audio", {"Audio"}, {
				{"Sound", "string", std::string("")}, {"Duration", "float", 1.0f}
			}},

			// Variables
			{ Actions::SetVariable, "Set Variable", {"Variables"}, {
				{"Variable", "string", std::string("")}, {"Value", "float", 0.0f}
			}},
			{ Actions::AddToVariable, "Add To Variable", {"Variables"}, {
				{"Variable", "string", std::string("")}, {"Value", "float", 1.0f}
			}},
			{ Actions::SubtractFromVariable, "Subtract From Variable", {"Variables"}, {
				{"Variable", "string", std::string("")}, {"Value", "float", 1.0f}
			}},
			{ Actions::MultiplyVariable, "Multiply Variable", {"Variables"}, {
				{"Variable", "string", std::string("")}, {"Value", "float", 2.0f}
			}},
			{ Actions::DivideVariable, "Divide Variable", {"Variables"}, {
				{"Variable", "string", std::string("")}, {"Value", "float", 2.0f}
			}},

			// Timer
			{ Actions::StartTimer, "Start Timer", {"Timer"}, {
				{"Timer", "string", std::string("timer1")}
			}},
			{ Actions::StopTimer, "Stop Timer", {"Timer"}, {
				{"Timer", "string", std::string("timer1")}
			}},
			{ Actions::ResetTimer, "Reset Timer", {"Timer"}, {
				{"Timer", "string", std::string("timer1")}
			}},
			{ Actions::PauseTimer, "Pause Timer", {"Timer"}, {
				{"Timer", "string", std::string("timer1")}
			}},

			// Scene
			{ Actions::LoadScene, "Load Scene", {"Scene"}, {
				{"SceneName", "string", std::string("")}
			}},
			{ Actions::ReloadScene, "Reload Scene", {"Scene"}, {} },
			{ Actions::QuitGame, "Quit Game", {"Scene"}, {} },
			{ Actions::PauseGame, "Pause Game", {"Scene"}, {} },
			{ Actions::ResumeGame, "Resume Game", {"Scene"}, {} },
			{ Actions::SetTimeScale, "Set Time Scale", {"Scene"}, {
				{"TimeScale", "float", 1.0f}
			}},

			// Camera
			{ Actions::SetCameraPosition, "Set Camera Position", {"Camera"}, {
				{"X", "float", 0.0f}, {"Y", "float", 0.0f}, {"Z", "float", 0.0f}
			}},
			{ Actions::SetCameraRotation, "Set Camera Rotation", {"Camera"}, {
				{"X", "float", 0.0f}, {"Y", "float", 0.0f}, {"Z", "float", 0.0f}
			}},
			{ Actions::CameraFollow, "Camera Follow", {"Camera"}, {
				{"Target", "string", std::string("")}, {"Speed", "float", 1.0f}
			}},
			{ Actions::SetCameraSize, "Set Camera Size", {"Camera"}, {
				{"Size", "float", 5.0f}
			}},
			{ Actions::ShakeCamera, "Shake Camera", {"Camera"}, {
				{"Intensity", "float", 1.0f}, {"Duration", "float", 0.5f}
			}},

			// Effects
			{ Actions::CreateParticles, "Create Particles", {"Effects"}, {
				{"GameObject", "string", std::string("self")}, {"Particles", "string", std::string("")}
			}},
			{ Actions::StopParticles, "Stop Particles", {"Effects"}, {
				{"GameObject", "string", std::string("self")}
			}},
			{ Actions::ScreenFlash, "Screen Flash", {"Effects"}, {
				{"R", "float", 1.0f}, {"G", "float", 1.0f}, {"B", "float", 1.0f}, {"Duration", "float", 0.2f}
			}},

			// UI
			{ Actions::SetText, "Set Text", {"UI"}, {
				{"TextObject", "string", std::string("")}, {"Text", "string", std::string("")}
			}},
			{ Actions::SetButtonEnabled, "Set Button Enabled", {"UI"}, {
				{"Button", "string", std::string("")}, {"Enabled", "bool", true}
			}},
			{ Actions::SetSliderValue, "Set Slider Value", {"UI"}, {
				{"Slider", "string", std::string("")}, {"Value", "float", 0.0f}
			}},
			{ Actions::ShowUI, "Show UI", {"UI"}, {
				{"UIObject", "string", std::string("")}
			}},
			{ Actions::HideUI, "Hide UI", {"UI"}, {
				{"UIObject", "string", std::string("")}
			}},

			// Debug
			{ Actions::Log, "Log Message", {"Debug"}, {
				{"Message", "string", std::string("")}
			}},
			{ Actions::LogWarning, "Log Warning", {"Debug"}, {
				{"Message", "string", std::string("")}
			}},
			{ Actions::LogError, "Log Error", {"Debug"}, {
				{"Message", "string", std::string("")}
			}},
			{ Actions::DrawLine, "Draw Debug Line", {"Debug"}, {
				{"StartX", "float", 0.0f}, {"StartY", "float", 0.0f}, {"StartZ", "float", 0.0f},
				{"EndX", "float", 1.0f}, {"EndY", "float", 1.0f}, {"EndZ", "float", 1.0f}, {"Duration", "float", 0.0f}
			}},
			{ Actions::DrawCircle, "Draw Debug Circle", {"Debug"}, {
				{"X", "float", 0.0f}, {"Y", "float", 0.0f}, {"Z", "float", 0.0f}, {"Radius", "float", 1.0f}, {"Duration", "float", 0.0f}
			}},

			// Flow Control
			{ Actions::Wait, "Wait", {"Flow Control"}, {
				{"Duration", "float", 1.0f}
			}},
			{ Actions::CallFunction, "Call Function", {"Flow Control"}, {
				{"Function", "string", std::string("")}
			}},

			// Input
			{ Actions::SetCursorVisible, "Set Cursor Visible", {"Input"}, {
				{"Visible", "bool", true}
			}},
			{ Actions::SetCursorLocked, "Set Cursor Locked", {"Input"}, {
				{"Locked", "bool", false}
			}},
			{ Actions::VibrateGamepad, "Vibrate Gamepad", {"Input"}, {
				{"Gamepad", "int", 0}, {"LeftMotor", "float", 0.5f}, {"RightMotor", "float", 0.5f}, {"Duration", "float", 0.5f}
			}},
		};

		return actions;
	}
}