#include "CanvasEditor.h"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imnodes/imnodes.h"
#include "ThirdParty/imgui/IconsFontAwesome6.h"
#include "Utilities/FontManager.h"
#include <fstream>
#include "Utilities/ConsoleLogger.h"
#include "Components/UI/RectTransform.h"
#include "Components/UI/Label.h"
#include "Components/UI/Image.h"
#include "Components/UI/Button.h"

RaylibWrapper::Rectangle CanvasEditor::GetJsonRect(nlohmann::json* goJson, RaylibWrapper::Rectangle parentRect)
{
	if (!goJson->contains("components"))
		return { 0,0,0,0 };

	for (auto& comp : (*goJson)["components"])
	{
		if (comp["name"] == "RectTransform")
		{
			Vector2 anchorMin = { comp["exposed_variables"][1][0][2][0].get<float>(), comp["exposed_variables"][1][0][2][1].get<float>() };
			Vector2 anchorMax = { comp["exposed_variables"][1][1][2][0].get<float>(), comp["exposed_variables"][1][1][2][1].get<float>() };
			Vector2 sizeDelta = { comp["exposed_variables"][1][3][2][0].get<float>(), comp["exposed_variables"][1][3][2][1].get<float>() };
			Vector2 anchoredPos = { comp["exposed_variables"][1][4][2][0].get<float>(), comp["exposed_variables"][1][4][2][1].get<float>() };

			float left = parentRect.x + parentRect.width * anchorMin.x;
			float top = parentRect.y + parentRect.height * anchorMin.y;
			float right = parentRect.x + parentRect.width * anchorMax.x;
			float bottom = parentRect.y + parentRect.height * anchorMax.y;

			float width = right - left + sizeDelta.x;
			float height = bottom - top + sizeDelta.y;

			float x = left + anchoredPos.x;
			float y = top + anchoredPos.y;

			return { x, y, width, height };
		}
	}
	return { 0,0,0,0 };
}

Vector2 CanvasEditor::GetJsonPivotPosition(nlohmann::json* goJson, RaylibWrapper::Rectangle rect)
{
	if (!goJson->contains("components"))
		return { rect.x, rect.y };

	for (auto& comp : (*goJson)["components"])
	{
		if (comp["name"] == "RectTransform")
		{
			Vector2 p = { comp["exposed_variables"][1][2][2][0].get<float>(), comp["exposed_variables"][1][2][2][1].get<float>() };
			return { rect.x + rect.width * p.x, rect.y + rect.height * p.y };
		}
	}
	return { rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f };
}

void CanvasEditor::DeleteElement(int id)
{
	// Delete children first
	auto& gos = canvasData["GameObjects"];
	std::vector<int> childrenIds;
	for (auto& g : gos)
	{
		if (g["parent_id"].get<int>() == id)
			childrenIds.push_back(g["id"].get<int>());
	}
	for (int cid : childrenIds)
		DeleteElement(cid);

	// Delete the elemnt itself
	gos.erase(std::remove_if(gos.begin(), gos.end(),
		[&](const auto& g) { return g["id"].get<int>() == id; }),
		gos.end());

	if (objectInProperties && (*objectInProperties)["id"].get<int>() == id)
		objectInProperties = nullptr;

	if (selectedObject && (*selectedObject)["id"].get<int>() == id)
		selectedObject = nullptr;

	if (objectInHierarchyContextMenu && (*objectInHierarchyContextMenu)["id"].get<int>() == id)
		objectInHierarchyContextMenu = nullptr;
}


void CanvasEditor::RenderElements(nlohmann::json* goJson, RaylibWrapper::Rectangle parentRect, RaylibWrapper::Rectangle clipRect) 
{
	RaylibWrapper::Rectangle rect = GetJsonRect(goJson, parentRect);

	// Clip to preview bounds
	if (rect.x + rect.width < clipRect.x || rect.x > clipRect.x + clipRect.width ||
		rect.y + rect.height < clipRect.y || rect.y > clipRect.y + clipRect.height)
		return; // Outside bounds

	// Render components
	if (goJson->contains("components"))
	{
		for (auto& comp : (*goJson)["components"])
		{
			if (comp["name"] == "Label")
			{
				std::string text = comp["exposed_variables"][1][0][2].get<std::string>();
				Color color = { (unsigned char)comp["exposed_variables"][1][1][2][0].get<int>(), (unsigned char)comp["exposed_variables"][1][1][2][1].get<int>(), (unsigned char)comp["exposed_variables"][1][1][2][2].get<int>(), (unsigned char)comp["exposed_variables"][1][1][2][3].get<int>() };
				int fontSize = comp["exposed_variables"][1][3][2].get<int>();
				std::string fontPath = comp["exposed_variables"][1][2][2].get<std::string>();

				if (text.empty())
					continue;

				if (fontPath.empty() || fontPath == "nullptr")
					fontPath = FontManager::GetDefaultFontPath();

				ImGui::PushFont(FontManager::GetFont(fontPath, fontSize, true));

				Vector2 position = GetJsonPivotPosition(goJson, rect);
				ImVec2 textSize = ImGui::CalcTextSize(text.c_str());

				// Get the top-left screen position
				float textScreenX = position.x - (textSize.x * 0.5f);
				float textScreenY = position.y - (textSize.y * 0.5f);

				ImGui::SetCursorScreenPos(ImVec2(textScreenX, textScreenY));
				ImGui::TextColored({ (float)color.r / 255, (float)color.g / 255, (float)color.b / 255, (float)color.a / 255 }, text.c_str());

				// Selection
				ImGui::SetCursorScreenPos(ImVec2(textScreenX, textScreenY));
				ImGui::InvisibleButton(("##select" + std::to_string((*goJson)["id"].get<int>())).c_str(), textSize);

				if (ImGui::IsItemClicked())
				{
					selectedObject = goJson;
					objectInProperties = goJson;
				}

				ImGui::PopFont();

				// If selected, allow dragging
				if (selectedObject == goJson)
				{
					ImDrawList* drawList = ImGui::GetForegroundDrawList();
					ImVec2 min = ImGui::GetItemRectMin();
					ImVec2 max = ImGui::GetItemRectMax();
					drawList->AddRect(min, max, IM_COL32(0, 128, 255, 255), 0.0f, 0, 2.0f);

					// Start dragging when left mouse clicked on the item
					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						dragging = true;
						// store anchored position as starting drag offset
						dragStart = { comp["exposed_variables"][1][4][2][0].get<float>(), comp["exposed_variables"][1][4][2][1].get<float>() };
						// store current mouse screen position
						mouseStart = { ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y };
					}

					if ((ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) ||
						(ImGui::IsItemHovered() && ImGui::IsKeyPressed(ImGuiKey_Delete)))
					{
						int deleteId = (*goJson)["id"].get<int>();

						if (objectInProperties && (*objectInProperties)["id"].get<int>() == deleteId)
							objectInProperties = nullptr;

						selectedObject = nullptr;

						DeleteElement(deleteId);
						return;
					}
				}
			}
			//else if (comp["name"] == "Image")
			//{
			//	std::string spritePath = comp["exposed_variables"][1][0][2].get<std::string>();
			//	Color color = {
			//		(unsigned char)comp["exposed_variables"][1][1][2][0].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][1][2][1].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][1][2][2].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][1][2][3].get<int>()
			//	};

			//	Vector2 position = GetJsonPivotPosition(goJson, rect);
			//	float scale = (*goJson)["transform"]["scale"][0].get<float>();

			//	ImVec2 size = ImVec2(100 * scale, 100 * scale);

			//	if (spritePath == "Square")
			//	{
			//		ImVec2 min = { position.x - size.x / 2, position.y - size.y / 2 };
			//		ImVec2 max = { position.x + size.x / 2, position.y + size.y / 2 };

			//		ImGui::GetForegroundDrawList()->AddRectFilled(min, max,
			//			IM_COL32(color.r, color.g, color.b, color.a), 0.0f);
			//	}
			//	else if (spritePath == "Circle")
			//	{
			//		ImGui::GetForegroundDrawList()->AddCircleFilled(
			//			ImVec2(position.x, position.y), size.x * 0.5f,
			//			IM_COL32(color.r, color.g, color.b, color.a));
			//	}
			//	else
			//	{
			//		RaylibWrapper::Texture2D* texture = TextureManager::GetTexture(spritePath);
			//		if (texture)
			//		{
			//			size = { texture->width * scale, texture->height * scale };
			//			ImGui::SetCursorScreenPos({ position.x - size.x / 2, position.y - size.y / 2 });
			//			RaylibWrapper::rlImGuiImageSizeTintV(
			//				texture, size,
			//				{ (float)color.r / 255, (float)color.g / 255, (float)color.b / 255, (float)color.a / 255 }
			//			);
			//		}
			//	}

			//	// Selection
			//	ImGui::SetCursorScreenPos({ position.x - size.x / 2, position.y - size.y / 2 });
			//	ImGui::InvisibleButton(("##select" + std::to_string((*goJson)["id"].get<int>())).c_str(), size);

			//	if (ImGui::IsItemClicked())
			//	{
			//		selectedObject = goJson;
			//		objectInProperties = goJson;
			//	}

			//	if (selectedObject == goJson)
			//	{
			//		ImDrawList* drawList = ImGui::GetForegroundDrawList();
			//		ImVec2 min = ImGui::GetItemRectMin();
			//		ImVec2 max = ImGui::GetItemRectMax();
			//		drawList->AddRect(min, max, IM_COL32(0, 128, 255, 255), 0.0f, 0, 2.0f);

			//		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			//		{
			//			dragging = true;
			//			dragStart = { rect["anchoredPosition"][0].get<float>(), rect["anchoredPosition"][1].get<float>() };
			//			mouseStart = { ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y };
			//		}

			//		if ((ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) ||
			//			(ImGui::IsItemHovered() && ImGui::IsKeyPressed(ImGuiKey_Delete)))
			//		{
			//			int deleteId = (*goJson)["id"].get<int>();

			//			if (objectInProperties && (*objectInProperties)["id"].get<int>() == deleteId)
			//				objectInProperties = nullptr;

			//			selectedObject = nullptr;
			//			DeleteElement(deleteId);
			//			return;
			//		}
			//	}
			//}
			//else if (comp["name"] == "Button")
			//{
			//	std::string spritePath = comp["exposed_variables"][1][0][2].get<std::string>();

			//	Color normalColor = {
			//		(unsigned char)comp["exposed_variables"][1][1][2][0].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][1][2][1].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][1][2][2].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][1][2][3].get<int>()
			//	};

			//	bool disabled = comp["exposed_variables"][1][5][2].get<bool>();
			//	Color disabledColor = {
			//		(unsigned char)comp["exposed_variables"][1][4][2][0].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][4][2][1].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][4][2][2].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][4][2][3].get<int>()
			//	};

			//	std::string text = comp["exposed_variables"][1][6][2].get<std::string>();
			//	Color textColor = {
			//		(unsigned char)comp["exposed_variables"][1][7][2][0].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][7][2][1].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][7][2][2].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][7][2][3].get<int>()
			//	};
			//	Color disabledTextColor = {
			//		(unsigned char)comp["exposed_variables"][1][8][2][0].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][8][2][1].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][8][2][2].get<int>(),
			//		(unsigned char)comp["exposed_variables"][1][8][2][3].get<int>()
			//	};

			//	std::string fontPath = comp["exposed_variables"][1][9][2].get<std::string>();
			//	int fontSize = comp["exposed_variables"][1][10][2].get<int>();

			//	if (fontPath.empty() || fontPath == "nullptr")
			//		fontPath = FontManager::GetDefaultFontPath();

			//	Vector2 position = GetJsonPivotPosition(goJson, rect);
			//	float scale = (*goJson)["transform"]["scale"][0].get<float>();

			//	ImVec2 size = ImVec2(120 * scale, 50 * scale);

			//	Color bgColor = disabled ? disabledColor : normalColor;

			//	// Render Background
			//	if (spritePath == "Square")
			//	{
			//		ImVec2 min = { position.x - size.x / 2, position.y - size.y / 2 };
			//		ImVec2 max = { position.x + size.x / 2, position.y + size.y / 2 };
			//		ImGui::GetForegroundDrawList()->AddRectFilled(min, max, IM_COL32(bgColor.r, bgColor.g, bgColor.b, bgColor.a));
			//	}
			//	else
			//	{
			//		RaylibWrapper::Texture2D* texture = TextureManager::GetTexture(spritePath);
			//		if (texture)
			//		{
			//			ImVec2 texSize = { texture->width * scale, texture->height * scale };
			//			ImGui::SetCursorScreenPos({ position.x - texSize.x / 2, position.y - texSize.y / 2 });
			//			RaylibWrapper::rlImGuiImageSizeTintV(texture, texSize, { bgColor.r / 255.f, bgColor.g / 255.f, bgColor.b / 255.f, bgColor.a / 255.f });
			//		}
			//	}

			//	// Render Text
			//	if (!text.empty())
			//	{
			//		ImGui::PushFont(FontManager::GetFont(fontPath, fontSize, true));
			//		Color drawColor = disabled ? disabledTextColor : textColor;
			//		ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
			//		ImGui::SetCursorScreenPos({ position.x - textSize.x / 2, position.y - textSize.y / 2 });
			//		ImGui::TextColored({ drawColor.r / 255.f, drawColor.g / 255.f, drawColor.b / 255.f, drawColor.a / 255.f }, text.c_str());
			//		ImGui::PopFont();
			//	}

			//	// Selection
			//	ImGui::SetCursorScreenPos({ position.x - size.x / 2, position.y - size.y / 2 });
			//	ImGui::InvisibleButton(("##select" + std::to_string((*goJson)["id"].get<int>())).c_str(), size);

			//	if (ImGui::IsItemClicked())
			//	{
			//		selectedObject = goJson;
			//		objectInProperties = goJson;
			//	}

			//	if (selectedObject == goJson)
			//	{
			//		ImDrawList* drawList = ImGui::GetForegroundDrawList();
			//		ImVec2 min = ImGui::GetItemRectMin();
			//		ImVec2 max = ImGui::GetItemRectMax();
			//		drawList->AddRect(min, max, IM_COL32(0, 128, 255, 255), 0.0f, 0, 2.0f);

			//		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			//		{
			//			dragging = true;
			//			dragStart = { rect["anchoredPosition"][0].get<float>(), rect["anchoredPosition"][1].get<float>() };
			//			mouseStart = { ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y };
			//		}

			//		if ((ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) ||
			//			(ImGui::IsItemHovered() && ImGui::IsKeyPressed(ImGuiKey_Delete)))
			//		{
			//			int deleteId = (*goJson)["id"].get<int>();

			//			if (objectInProperties && (*objectInProperties)["id"].get<int>() == deleteId)
			//				objectInProperties = nullptr;

			//			selectedObject = nullptr;
			//			DeleteElement(deleteId);
			//			return;
			//		}
			//	}
			//}
		}
	}

	for (nlohmann::json& child : (*goJson)["children"])
		RenderElements(&child, rect, clipRect);
}

bool CanvasEditor::RenderHierarchyNode(nlohmann::json* gameObject, bool normalColor, bool& childDoubleClicked, nlohmann::json* objectHovered)
{
	childDoubleClicked = false;

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, normalColor ? IM_COL32(36, 40, 43, 255) : IM_COL32(45, 48, 51, 255));

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;

	if (selectedObject && (*selectedObject)["id"].get<int>() == (*gameObject)["id"].get<int>())
		flags |= ImGuiTreeNodeFlags_Selected;

	nlohmann::json& children = (*gameObject)["children"];
	flags |= children.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnArrow;

	bool nodeOpen = ImGui::TreeNodeEx((ICON_FA_CUBE + std::string(" ") + (*gameObject)["name"].get<std::string>() + "##" + std::to_string((*gameObject)["id"].get<int>())).c_str(), flags);

	bool currentNodeDoubleClicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

	if (ImGui::IsItemHovered())
		objectHovered = gameObject;

	if (ImGui::IsItemClicked())
	{
		hierarchyObjectClicked = true;
		objectInProperties = selectedObject = gameObject;
	}
	else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		objectInHierarchyContextMenu = gameObject;

	if (nodeOpen)
	{
		bool nColor = !normalColor;
		for (nlohmann::json& child : children)
		{
			bool childClicked = false;
			nColor = RenderHierarchyNode(&child, nColor, childClicked, objectHovered);
			childDoubleClicked |= childClicked;
		}
		ImGui::TreePop();
	}

	childDoubleClicked |= currentNodeDoubleClicked;

	return !normalColor;
}

void CanvasEditor::RenderHierarchy()
{
	ImGui::Text(ICON_FA_SITEMAP " Elements");
	ImGui::Separator();

	nlohmann::json* objectHovered = nullptr;

	if (ImGui::BeginTable("HierarchyTable##CanvasEditor", 1, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
	{
		bool hierarchyRowColor = true;

		for (auto& gameObjectJson : canvasData["GameObjects"])
		{
			if (gameObjectJson["parent_id"].get<int>() == -1)
			{
				bool childDoubleClicked = false;
				hierarchyRowColor = RenderHierarchyNode(&gameObjectJson, hierarchyRowColor, childDoubleClicked, objectHovered);
			}
		}
		ImGui::EndTable();
	}

	//  Context menu
	static nlohmann::json* objectRightClicked = nullptr;

	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		ImGui::OpenPopup("HierarchyContextMenuCanvasEditor");
		objectRightClicked = objectHovered;
	}

	if (ImGui::BeginPopup("HierarchyContextMenuCanvasEditor"))
	{
		ImGui::Separator();

		struct ObjectItem { std::string menuName, name, type; };
		static const std::vector<ObjectItem> menuObjects = {
			{"Create Label", "Label", "Label"},
			{"Create Image", "Image", "Image"},
			{"Create Button", "Button", "Button"}

		};
		ObjectItem objectToCreate = { "", "", "" };

		for (const auto& item : menuObjects)
		{
			if (ImGui::MenuItem(item.menuName.c_str()))
				objectToCreate = item;

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Right-click for options");
		}

		if (objectRightClicked && ImGui::MenuItem("Delete"))
		{
			DeleteElement((*objectRightClicked)["id"].get<int>());
			objectRightClicked = nullptr;
			hierarchyContextMenuOpen = false;
		}

		if (!objectToCreate.name.empty())
		{
			hierarchyObjectClicked = true;
			srand(static_cast<unsigned int>(time(nullptr)));

			nlohmann::json newGo;
			newGo["id"] = 100000 + rand() % 900000; // Todo: Make sure this is always unique
			newGo["name"] = objectToCreate.name;
			newGo["parent_id"] = objectRightClicked ? (*objectRightClicked)["id"].get<int>() : -1;
			newGo["active"] = true;
			newGo["global_active"] = true;
			newGo["children"] = nlohmann::json::array();
			newGo["components"] = nlohmann::json::array();

			// RectTransform
			{
				RectTransform* rt = new RectTransform(nullptr, -1);
				nlohmann::json rtComp;
				rtComp["name"] = "RectTransform";
				rtComp["id"] = 100000 + rand() % 900000;
				rtComp["active"] = true;
				rtComp["exposed_variables"] = rt->exposedVariables;
				newGo["components"].push_back(rtComp);
				delete rt;
			}

			if (objectToCreate.type == "Label")
			{
				Label* label = new Label(nullptr, -1);
				nlohmann::json labelComp;
				labelComp["name"] = "Label";
				labelComp["id"] = 100000 + rand() % 900000;
				labelComp["active"] = true;
				labelComp["exposed_variables"] = label->exposedVariables;
				newGo["components"].push_back(labelComp);
				delete label;
			}
			else if (objectToCreate.type == "Image")
			{
				Image* image = new Image(nullptr, -1);
				nlohmann::json imageComp;
				imageComp["name"] = "Image";
				imageComp["id"] = 100000 + rand() % 900000;
				imageComp["active"] = true;
				imageComp["exposed_variables"] = image->exposedVariables;
				newGo["components"].push_back(imageComp);
				delete image;
			}
			else if (objectToCreate.type == "Button")
			{
				Button* button = new Button(nullptr, -1);
				nlohmann::json buttonComp;
				buttonComp["name"] = "Button";
				buttonComp["id"] = 100000 + rand() % 900000;
				buttonComp["active"] = true;
				buttonComp["exposed_variables"] = button->exposedVariables;
				newGo["components"].push_back(buttonComp);
				delete button;
			}

			if (objectRightClicked)
			{
				(*objectRightClicked)["children"].push_back(newGo);
				selectedObject = &(*objectRightClicked)["children"].back();
			}
			else
			{
				canvasData["GameObjects"].push_back(newGo);
				selectedObject = &canvasData["GameObjects"].back();
			}

			objectInProperties = selectedObject;
			objectRightClicked = nullptr;
		}

		ImGui::EndPopup();
	}

	if (hierarchyContextMenuOpen && !ImGui::IsPopupOpen("HierarchyContextMenuCanvasEditor"))
	{
		objectInHierarchyContextMenu = nullptr;
		hierarchyContextMenuOpen = false;
		objectRightClicked = nullptr;
	}
	else if (!hierarchyObjectClicked && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
	{
		selectedObject = nullptr;
		objectInProperties = nullptr;
		objectRightClicked = nullptr;
	}

	hierarchyObjectClicked = false;
}

void CanvasEditor::RenderProperties()
{
	ImGui::Text(ICON_FA_WRENCH " Properties");
	ImGui::Separator();

	if (!objectInProperties)
		return;

	// Name
	std::string& name = (*objectInProperties)["name"].get_ref<std::string&>();
	char buffer[128];
	strncpy_s(buffer, name.c_str(), _TRUNCATE);
	if (ImGui::InputText("Name", buffer, sizeof(buffer)))
		name = buffer;

	// Components
	for (auto& comp : (*objectInProperties)["components"]) // Todo: Change the properties rendering to how it's done in Editor.cpp
	{
		std::string type = comp["name"].get<std::string>();
		ImGui::Separator();
		ImGui::Text("%s", type.c_str());

		if (type == "RectTransform")
		{
			float amin[2] = { comp["exposed_variables"][1][0][2][0].get<float>(), comp["exposed_variables"][1][0][2][1].get<float>() };
			if (ImGui::DragFloat2("Anchor Min", amin, 0.01f, 0.0f, 1.0f))
			{
				comp["exposed_variables"][1][0][2][0] = amin[0];
				comp["exposed_variables"][1][0][2][1] = amin[1];
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Min anchor fraction (0-1): Top-left relative to parent");

			float amax[2] = { comp["exposed_variables"][1][1][2][0].get<float>(), comp["exposed_variables"][1][1][2][1].get<float>() };
			if (ImGui::DragFloat2("Anchor Max", amax, 0.01f, 0.0f, 1.0f))
			{
				comp["exposed_variables"][1][1][2][0] = amax[0];
				comp["exposed_variables"][1][1][2][1] = amax[1];
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Max anchor fraction (0-1): Bottom-right relative to parent");

			float p[2] = { comp["exposed_variables"][1][2][2][0].get<float>(), comp["exposed_variables"][1][2][2][1].get<float>() };
			if (ImGui::DragFloat2("Pivot", p, 0.01f, 0.0f, 1.0f))
			{
				comp["exposed_variables"][1][2][2][0] = p[0];
				comp["exposed_variables"][1][2][2][1] = p[1];
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Pivot point fraction (0-1) for positioning/rotation");

			float sd[2] = { comp["exposed_variables"][1][3][2][0].get<float>(), comp["exposed_variables"][1][3][2][1].get<float>() };
			if (ImGui::DragFloat2("Size Delta", sd, 1.0f))
			{
				comp["exposed_variables"][1][3][2][0] = sd[0];
				comp["exposed_variables"][1][3][2][1] = sd[1];
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Additional size beyond anchors (pixels)");

			float ap[2] = { comp["exposed_variables"][1][4][2][0].get<float>(), comp["exposed_variables"][1][4][2][1].get<float>() };
			if (ImGui::DragFloat2("Anchored Position", ap, 1.0f))
			{
				comp["exposed_variables"][1][4][2][0] = ap[0];
				comp["exposed_variables"][1][4][2][1] = ap[1];
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Pixel offset from anchor point");
		}
		else if (type == "Label")
		{
			std::string text = comp["exposed_variables"][1][0][2].get<std::string>();
			char textBuffer[256];
			strncpy_s(textBuffer, text.c_str(), _TRUNCATE);
			if (ImGui::InputText("Text", textBuffer, sizeof(textBuffer)))
				comp["exposed_variables"][1][0][2] = textBuffer;

			float col[4] = {
				comp["exposed_variables"][1][1][2][0].get<int>() / 255.0f,
				comp["exposed_variables"][1][1][2][1].get<int>() / 255.0f,
				comp["exposed_variables"][1][1][2][2].get<int>() / 255.0f,
				comp["exposed_variables"][1][1][2][3].get<int>() / 255.0f
			};
			if (ImGui::ColorEdit4("Color", col))
			{
				comp["exposed_variables"][1][1][2][0] = (int)(col[0] * 255);
				comp["exposed_variables"][1][1][2][1] = (int)(col[1] * 255);
				comp["exposed_variables"][1][1][2][2] = (int)(col[2] * 255);
				comp["exposed_variables"][1][1][2][3] = (int)(col[3] * 255);
			}

			std::string font = comp["exposed_variables"][1][2][2].get<std::string>();
			char fontBuffer[256];
			strncpy_s(fontBuffer, font.c_str(), _TRUNCATE);
			if (ImGui::InputText("Font", fontBuffer, sizeof(fontBuffer)))
				comp["exposed_variables"][1][2][2] = fontBuffer;

			int fs = comp["exposed_variables"][1][3][2].get<int>();
			if (ImGui::DragInt("Font Size", &fs, 1, 1, 100))
				comp["exposed_variables"][1][3][2] = fs;
		}
		else if (type == "Image")
		{
			// Sprite path/name
			std::string sprite = comp["exposed_variables"][1][0][2].get<std::string>();
			char spriteBuffer[256];
			strncpy_s(spriteBuffer, sprite.c_str(), _TRUNCATE);
			if (ImGui::InputText("Sprite", spriteBuffer, sizeof(spriteBuffer)))
				comp["exposed_variables"][1][0][2] = spriteBuffer;

			// Tint color
			float col[4] = {
				comp["exposed_variables"][1][1][2][0].get<int>() / 255.0f,
				comp["exposed_variables"][1][1][2][1].get<int>() / 255.0f,
				comp["exposed_variables"][1][1][2][2].get<int>() / 255.0f,
				comp["exposed_variables"][1][1][2][3].get<int>() / 255.0f
			};
			if (ImGui::ColorEdit4("Color", col))
			{
				comp["exposed_variables"][1][1][2][0] = (int)(col[0] * 255);
				comp["exposed_variables"][1][1][2][1] = (int)(col[1] * 255);
				comp["exposed_variables"][1][1][2][2] = (int)(col[2] * 255);
				comp["exposed_variables"][1][1][2][3] = (int)(col[3] * 255);
			}
		}
		else if (type == "Button")
		{
			// Image (sprite)
			std::string sprite = comp["exposed_variables"][1][0][2].get<std::string>();
			char spriteBuffer[256];
			strncpy_s(spriteBuffer, sprite.c_str(), _TRUNCATE);
			if (ImGui::InputText("Image", spriteBuffer, sizeof(spriteBuffer)))
				comp["exposed_variables"][1][0][2] = spriteBuffer;

			// Helper lambda to draw a color picker
			auto drawColor = [&](const char* label, nlohmann::json& colorJson)
				{
					float col[4] = {
						colorJson[0].get<int>() / 255.0f,
						colorJson[1].get<int>() / 255.0f,
						colorJson[2].get<int>() / 255.0f,
						colorJson[3].get<int>() / 255.0f
					};
					if (ImGui::ColorEdit4(label, col))
					{
						colorJson[0] = (int)(col[0] * 255);
						colorJson[1] = (int)(col[1] * 255);
						colorJson[2] = (int)(col[2] * 255);
						colorJson[3] = (int)(col[3] * 255);
					}
				};

			// Draw button color states
			drawColor("Normal Color", comp["exposed_variables"][1][1][2]);
			drawColor("Hovered Color", comp["exposed_variables"][1][2][2]);
			drawColor("Pressed Color", comp["exposed_variables"][1][3][2]);
			drawColor("Disabled Color", comp["exposed_variables"][1][4][2]);

			// Disabled checkbox
			bool disabled = comp["exposed_variables"][1][5][2].get<bool>();
			if (ImGui::Checkbox("Disabled", &disabled))
				comp["exposed_variables"][1][5][2] = disabled;

			// Text
			std::string text = comp["exposed_variables"][1][6][2].get<std::string>();
			char textBuffer[256];
			strncpy_s(textBuffer, text.c_str(), _TRUNCATE);
			if (ImGui::InputText("Text", textBuffer, sizeof(textBuffer)))
				comp["exposed_variables"][1][6][2] = textBuffer;

			// Text colors
			drawColor("Text Color", comp["exposed_variables"][1][7][2]);
			drawColor("Disabled Text Color", comp["exposed_variables"][1][8][2]);

			// Font
			std::string font = comp["exposed_variables"][1][9][2].get<std::string>();
			char fontBuffer[256];
			strncpy_s(fontBuffer, font.c_str(), _TRUNCATE);
			if (ImGui::InputText("Font", fontBuffer, sizeof(fontBuffer)))
				comp["exposed_variables"][1][9][2] = fontBuffer;

			// Font Size
			int fs = comp["exposed_variables"][1][10][2].get<int>();
			if (ImGui::DragInt("Font Size", &fs, 1, 1, 100))
				comp["exposed_variables"][1][10][2] = fs;
		}
	}
}

void CanvasEditor::Render()
{
	if (!windowOpen)
		return;

	if (ImGui::Begin((ICON_FA_BRUSH + std::string(" Canvas Editor")).c_str(), &windowOpen))
	{
		if (!canvas)
		{
			ImGui::PushFont(FontManager::GetFont("Familiar-Pro-Bold", 25, false));
			ImVec2 textSize = ImGui::CalcTextSize("No Canvas selected. Select or create one in the Content Browser.");
			ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - textSize.x) * 0.5f, (ImGui::GetWindowHeight() - textSize.y) * 0.5f));
			ImGui::Text("No Canvas selected. Select or create one in the Content Browser.");
			ImGui::PopFont();
			ImGui::End();
			return;
		}

		// Toolbar
		ImGui::BeginChild("##Toolbar", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() + 10));

		if (ImGui::Button(ICON_FA_FLOPPY_DISK " Save"))
			SaveCanvas();

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Save changes to canvas");

		ImGui::EndChild();

		ImGui::Separator();

		ImVec2 region = ImGui::GetContentRegionAvail();

		// Persistent splitter state
		static float hierarchyWidth = 250.0f;
		static float propertiesWidth = 300.0f;
		const float minWidth = 150.0f;
		hierarchyWidth = ImClamp(hierarchyWidth, minWidth, ImMax(minWidth, region.x - minWidth - propertiesWidth));
		propertiesWidth = ImClamp(propertiesWidth, minWidth, ImMax(minWidth, region.x - minWidth - hierarchyWidth));

		// Push no-padding style
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

		// Hierarchy
		ImGui::BeginChild("##HierarchyCanvasEditor", ImVec2(hierarchyWidth, region.y), true);
		RenderHierarchy();
		ImGui::EndChild();

		// Splitter
		ImGui::SameLine();
		ImGui::InvisibleButton("##hsplitter1", ImVec2(8.0f, -1));
		if (ImGui::IsItemActive())
			hierarchyWidth += ImGui::GetIO().MouseDelta.x;

		// Preview
		ImGui::SameLine();
		ImGui::BeginChild("##Preview", ImVec2(region.x - hierarchyWidth - propertiesWidth - 16.0f, region.y), true);
		{
			// 16:9 bounds calculation
			ImVec2 avail = ImGui::GetContentRegionAvail();
			float aspect = 16.0f / 9.0f;
			float previewWidth = avail.y * aspect;
			float previewHeight = avail.y;
			float offsetX = 0, offsetY = 0;

			if (previewWidth > avail.x)
			{
				previewWidth = avail.x;
				previewHeight = previewWidth / aspect;
				offsetY = (avail.y - previewHeight) / 2;
			}
			else
				offsetX = (avail.x - previewWidth) / 2;

			// Draw background
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 panePos = ImGui::GetCursorScreenPos();
			drawList->AddRectFilled(panePos, ImVec2(panePos.x + avail.x, panePos.y + avail.y), IM_COL32(20, 20, 20, 255));

			// Add margins around the grid
			float gridMargin = 15.0f;

			// Set cursor to offset + margin position
			ImGui::SetCursorPos(ImVec2(offsetX + gridMargin, offsetY + gridMargin));
			ImVec2 boundsPos = ImGui::GetCursorScreenPos();

			// Adjust preview width/height to account for margins
			previewWidth -= gridMargin * 2.0f;
			previewHeight -= gridMargin * 2.0f;

			// Align to fix half-pixel issue
			boundsPos.x = std::floor(boundsPos.x) + 0.5f;
			boundsPos.y = std::floor(boundsPos.y) + 0.5f;

			// Draw grid inside bounds
			float gridSize = 20.0f;
			ImU32 gridColor = IM_COL32(255, 255, 255, 50);
			for (float x = 0; x <= previewWidth; x += gridSize)
				drawList->AddLine(ImVec2(boundsPos.x + x, boundsPos.y), ImVec2(boundsPos.x + x, boundsPos.y + previewHeight), gridColor);

			// Add the right-side boundry line, as it may be missing
			drawList->AddLine(ImVec2(boundsPos.x + previewWidth, boundsPos.y), ImVec2(boundsPos.x + previewWidth, boundsPos.y + previewHeight), gridColor );

			for (float y = 0; y <= previewHeight; y += gridSize)
				drawList->AddLine(ImVec2(boundsPos.x, boundsPos.y + y), ImVec2(boundsPos.x + previewWidth, boundsPos.y + y), gridColor);

			// Add the bottom-side boundary line, as it may be missing
			drawList->AddLine(ImVec2(boundsPos.x, boundsPos.y + previewHeight), ImVec2(boundsPos.x + previewWidth, boundsPos.y + previewHeight), gridColor);

			// Create preview rect
			RaylibWrapper::Rectangle previewRect = { boundsPos.x, boundsPos.y, previewWidth, previewHeight };

			// Render elements
			for (nlohmann::json& goJson : canvasData["GameObjects"])
				RenderElements(&goJson, previewRect, previewRect);

			// Dragging
			if (dragging && selectedObject)
			{
				for (auto& comp : (*selectedObject)["components"])
				{
					if (comp["name"] == "RectTransform")
					{
						Vector2 curMouse = { ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y };
						Vector2 delta = { curMouse.x - mouseStart.x, curMouse.y - mouseStart.y };

						comp["exposed_variables"][1][4][2][0] = dragStart.x + delta.x;
						comp["exposed_variables"][1][4][2][1] = dragStart.y + delta.y;
						break;
					}
				}
				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
					dragging = false;
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();
		ImGui::InvisibleButton("##hsplitter2", ImVec2(8.0f, -1));
		if (ImGui::IsItemActive())
			propertiesWidth -= ImGui::GetIO().MouseDelta.x;

		// Properties
		ImGui::SameLine();
		ImGui::BeginChild("##PropertiesCanvasEditor", ImVec2(propertiesWidth, region.y), true);
		RenderProperties();
		ImGui::EndChild();

		// Pop styles
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(3);
	}
	ImGui::End();
}

void CanvasEditor::SaveCanvas()
{
	if (!canvas)
		return;

	canvas->SetData(canvasData);
}

void CanvasEditor::LoadCanvas(Canvas* canvas)
{
	if (!canvas->GetData())
	{
		ConsoleLogger::ErrorLog("Canvas Editor: Failed to load. Invalid canvas data.");
		canvasData = nlohmann::json();
		return;
	}

	this->canvas = canvas;
	canvasData = *canvas->GetData();

	// Reset editor state
	selectedObject = nullptr;
	objectInProperties = nullptr;
	objectInHierarchyContextMenu = nullptr;
	hierarchyContextMenuOpen = false;
	dragging = false;

	// Synchronize exposed variables
	bool exposedVariableUpdated = false;
	for (auto& goJson : canvasData["GameObjects"])
	{
		if (!goJson.contains("components") || !goJson["components"].is_array())
			continue;

		for (auto& compJson : goJson["components"])
		{
			const std::string compName = compJson.value("name", "");
			Component* tempComp = nullptr;

			if (compName == "RectTransform")
				tempComp = new RectTransform(nullptr, -1);
			else if (compName == "Label")
				tempComp = new Label(nullptr, -1);
			else if (compName == "Image")
				tempComp = new Image(nullptr, -1);
			else if (compName == "Button")
				tempComp = new Button(nullptr, -1);

			if (!tempComp)
				continue;

			if (compJson.contains("exposed_variables") && compJson["exposed_variables"].is_array())
			{
				nlohmann::json& runtimeEV = tempComp->exposedVariables;

				for (auto jsonIt = compJson["exposed_variables"][1].begin(); jsonIt != compJson["exposed_variables"][1].end(); ++jsonIt)
				{
					const auto& savedVar = *jsonIt;

					for (auto runtimeIt = runtimeEV[1].begin();
						runtimeIt != runtimeEV[1].end(); ++runtimeIt)
					{
						auto& runtimeVar = *runtimeIt;

						if (runtimeVar.size() >= 3 && savedVar.size() >= 3 &&
							runtimeVar[0] == savedVar[0] && runtimeVar[1] == savedVar[1])
						{
							if (runtimeVar[2] != savedVar[2])
							{
								runtimeVar[2] = savedVar[2];
								exposedVariableUpdated = true;
							}
							break;
						}
					}
				}
			}

			compJson["exposed_variables"] = tempComp->exposedVariables;

			delete tempComp;
		}
	}

	// Update the Canvas and file if an exposed variable was updated
	if (exposedVariableUpdated)
		canvas->SetData(canvasData);
}