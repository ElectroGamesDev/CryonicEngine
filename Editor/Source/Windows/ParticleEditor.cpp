//#include "ParticleEditor.h"
//#include "ThirdParty/imgui/imgui.h"
//#include "ThirdParty/imnodes/imnodes.h"
//#include "ThirdParty/imgui/IconsFontAwesome6.h"
//#include "Utilities/FontManager.h"
//#include <fstream>
//#include "Utilities/ConsoleLogger.h"
//#include "Raylib/RaylibDrawWrapper.h"
//void ParticleEditor::Render()
//{
//	if (!windowOpen) return;
//	if (ImGui::Begin((ICON_FA_SPARKLES + std::string(" Particle Editor")).c_str(), &windowOpen))
//	{
//		if (!particles)
//		{
//			ImGui::PushFont(FontManager::GetFont("Familiar-Pro-Bold", 25, false));
//			ImVec2 textSize = ImGui::CalcTextSize("No Particles selected. Select or create one in the Content Browser.");
//			ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - textSize.x) * 0.5f, (ImGui::GetWindowHeight() - textSize.y) * 0.5f));
//			ImGui::Text("No Particles selected. Select or create one in the Content Browser.");
//			ImGui::PopFont();
//			ImGui::End();
//			return;
//		}
//		// Toolbar
//		ImGui::BeginChild("##Toolbar", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() + 10));
//		if (ImGui::Button(ICON_FA_FLOPPY_DISK " Save"))
//			SaveParticles();
//		ImGui::EndChild();
//		ImGui::Separator();
//		ImVec2 region = ImGui::GetContentRegionAvail();
//		static float hierarchyWidth = 250.0f;
//		static float propertiesWidth = 300.0f;
//		const float minWidth = 150.0f;
//		hierarchyWidth = ImClamp(hierarchyWidth, minWidth, ImMax(minWidth, region.x - minWidth - propertiesWidth));
//		propertiesWidth = ImClamp(propertiesWidth, minWidth, ImMax(minWidth, region.x - minWidth - hierarchyWidth));
//		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
//		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
//		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
//		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
//		ImGui::BeginChild("##HierarchyParticleEditor", ImVec2(hierarchyWidth, region.y * 0.8f), true);
//		RenderHierarchy();
//		ImGui::EndChild();
//		ImGui::SameLine();
//		ImGui::InvisibleButton("##hsplitter1", ImVec2(8.0f, -1));
//		if (ImGui::IsItemActive())
//			hierarchyWidth += ImGui::GetIO().MouseDelta.x;
//		ImGui::SameLine();
//		ImGui::BeginChild("##Preview", ImVec2(region.x - hierarchyWidth - propertiesWidth - 16.0f, region.y * 0.8f), true);
//		RenderPreview();
//		ImGui::EndChild();
//		ImGui::SameLine();
//		ImGui::InvisibleButton("##hsplitter2", ImVec2(8.0f, -1));
//		if (ImGui::IsItemActive())
//			propertiesWidth -= ImGui::GetIO().MouseDelta.x;
//		ImGui::SameLine();
//		ImGui::BeginChild("##PropertiesParticleEditor", ImVec2(propertiesWidth, region.y * 0.8f), true);
//		RenderProperties();
//		ImGui::EndChild();
//		// Timeline
//		ImGui::BeginChild("##Timeline", ImVec2(region.x, region.y * 0.2f), true);
//		ImGui::SliderFloat("Time", &currentTime, 0.0f, particlesData["duration"].get<float>());
//		if (ImGui::IsItemEdited())
//			scrubbing = true;
//		if (ImGui::Button(playing ? ICON_FA_PAUSE : ICON_FA_PLAY))
//			playing = !playing;
//		if (ImGui::Button(ICON_FA_STEP_FORWARD))
//			previewRenderer->Simulate(0.016f);
//		ImGui::DragFloat("Speed", &playbackSpeed, 0.1f, 0.1f, 10.0f);
//		ImGui::EndChild();
//		ImGui::PopStyleColor();
//		ImGui::PopStyleVar(3);
//	}
//	ImGui::End();
//	// Simulation
//	if (playing)
//		previewRenderer->Simulate(GetFrameTime() * playbackSpeed);
//	if (scrubbing)
//	{
//		previewRenderer->ClearParticles();
//		float t = 0.0f;
//		float step = 0.016f;
//		while (t < currentTime)
//		{
//			previewRenderer->Simulate(step);
//			t += step;
//		}
//		scrubbing = false;
//	}
//}
//void ParticleEditor::RenderPreview()
//{
//	ImVec2 avail = ImGui::GetContentRegionAvail();
//	if (avail.x != previewSize.x || avail.y != previewSize.y)
//	{
//		if (previewTexture.id > 0) UnloadRenderTexture(previewTexture);
//		previewTexture = LoadRenderTexture(avail.x, avail.y);
//		previewSize = avail;
//	}
//	// Update camera
//	if (ImGui::IsWindowHovered())
//	{
//		if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
//		{
//			orbitAngles.x += ImGui::GetIO().MouseDelta.y * 0.01f;
//			orbitAngles.y += ImGui::GetIO().MouseDelta.x * 0.01f;
//		}
//		cameraDistance -= GetMouseWheelMove() * 1.0f;
//		cameraDistance = ImClamp(cameraDistance, 1.0f, 100.0f);
//	}
//	previewCamera.position.x = cameraDistance * sin(orbitAngles.y) * cos(orbitAngles.x);
//	previewCamera.position.y = cameraDistance * sin(orbitAngles.x);
//	previewCamera.position.z = cameraDistance * cos(orbitAngles.y) * cos(orbitAngles.x);
//	previewCamera.target = { 0,0,0 };
//	BeginTextureMode(previewTexture);
//	ClearBackground(GRAY);
//	BeginMode3D(previewCamera);
//	previewRenderer->Render();
//	if (selectedObject)
//		DrawGizmos(*selectedObject);
//	EndMode3D();
//	EndTextureMode();
//	rlImGuiImageRenderTexture(&previewTexture);
//}
//void ParticleEditor::RenderHierarchy()
//{
//	ImGui::Text(ICON_FA_SITEMAP " Emitters");
//	ImGui::Separator();
//	nlohmann::json* objectHovered = nullptr;
//	if (ImGui::BeginTable("HierarchyTable##ParticleEditor", 1, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
//	{
//		bool hierarchyRowColor = true;
//		for (auto& emitterJson : particlesData["emitters"])
//		{
//			bool childDoubleClicked = false;
//			hierarchyRowColor = RenderHierarchyNode(&emitterJson, hierarchyRowColor, childDoubleClicked, objectHovered);
//		}
//		ImGui::EndTable();
//	}
//	// Context menu
//	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
//	{
//		ImGui::OpenPopup("HierarchyContextMenuParticleEditor");
//		objectInHierarchyContextMenu = objectHovered;
//	}
//	if (ImGui::BeginPopup("HierarchyContextMenuParticleEditor"))
//	{
//		if (ImGui::MenuItem("Create Emitter"))
//		{
//			nlohmann::json newEmitter;
//			newEmitter["name"] = "New Emitter";
//			newEmitter["id"] = 100000 + rand() % 900000;
//			newEmitter["modules"] = nlohmann::json::object(); // Add default modules
//			// Todo: Default values for modules
//			if (objectInHierarchyContextMenu)
//				(*objectInHierarchyContextMenu)["children"].push_back(newEmitter);
//			else
//				particlesData["emitters"].push_back(newEmitter);
//			selectedObject = &particlesData["emitters"].back(); // or child
//			objectInProperties = selectedObject;
//		}
//		if (objectInHierarchyContextMenu && ImGui::MenuItem("Delete"))
//		{
//			// Todo: Remove from array
//			DeleteEmitter((*objectInHierarchyContextMenu)["id"].get<int>());
//		}
//		ImGui::EndPopup();
//	}
//}
//bool ParticleEditor::RenderHierarchyNode(nlohmann::json* emitter, bool normalColor, bool& childDoubleClicked, nlohmann::json* objectHovered)
//{
//	childDoubleClicked = false;
//	ImGui::TableNextRow();
//	ImGui::TableSetColumnIndex(0);
//	ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, normalColor ? IM_COL32(36, 40, 43, 255) : IM_COL32(45, 48, 51, 255));
//	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
//	if (selectedObject && (*selectedObject)["id"].get<int>() == (*emitter)["id"].get<int>())
//		flags |= ImGuiTreeNodeFlags_Selected;
//	nlohmann::json& children = (*emitter)["children"];
//	flags |= children.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnArrow;
//	bool nodeOpen = ImGui::TreeNodeEx((ICON_FA_SPARKLES + std::string(" ") + (*emitter)["name"].get<std::string>() + "##" + std::to_string((*emitter)["id"].get<int>())).c_str(), flags);
//	if (ImGui::IsItemHovered())
//		objectHovered = emitter;
//	if (ImGui::IsItemClicked())
//	{
//		hierarchyObjectClicked = true;
//		objectInProperties = selectedObject = emitter;
//	}
//	if (nodeOpen)
//	{
//		bool nColor = !normalColor;
//		for (nlohmann::json& child : children)
//		{
//			bool childClicked = false;
//			nColor = RenderHierarchyNode(&child, nColor, childClicked, objectHovered);
//			childDoubleClicked |= childClicked;
//		}
//		ImGui::TreePop();
//	}
//	return !normalColor;
//}
//void ParticleEditor::RenderProperties()
//{
//	ImGui::Text(ICON_FA_WRENCH " Properties");
//	ImGui::Separator();
//	if (!objectInProperties)
//		return;
//	// Name
//	std::string& name = (*objectInProperties)["name"].get_ref<std::string&>();
//	char buffer[128];
//	strncpy_s(buffer, name.c_str(), _TRUNCATE);
//	if (ImGui::InputText("Name", buffer, sizeof(buffer)))
//		name = buffer;
//	// Modules
//	nlohmann::json& modules = (*objectInProperties)["modules"];
//	if (ImGui::CollapsingHeader("Emission"))
//	{
//		ImGui::DragFloat("Rate", &modules["emission"]["rate"]);
//		// Todo: Bursts table, lifetime curve EditCurve, delay, looping, duration
//		EditCurve("Lifetime", modules["emission"]["lifetime_curve"]);
//	}
//	if (ImGui::CollapsingHeader("Shape"))
//	{
//		const char* types[] = { "Point", "Sphere", "Box", "Cone", "Cylinder" };
//		int typeIdx = 0; // Todo: map to index
//		if (ImGui::Combo("Type", &typeIdx, types, 5))
//			modules["shape"]["type"] = types[typeIdx];
//		// Todo: params based on type
//	}
//	if (ImGui::CollapsingHeader("Velocity & Motion"))
//	{
//		float initial[3] = { modules["velocity"]["initial"][0],[1],[2] };
//		if (ImGui::DragFloat3("Initial", initial))
//			modules["velocity"]["initial"] = { initial[0], initial[1], initial[2] };
//		// Todo: random, inherit, gravity, drag, etc
//		EditCurve("Over Lifetime", modules["velocity"]["over_lifetime"]);
//	}
//	if (ImGui::CollapsingHeader("Appearance"))
//	{
//		// Texture, atlas, blend mode, etc
//	}
//	// Todo: other modules, forces, collision, rendering
//	EditGradient("Color Over Lifetime", modules["color"]["gradient"]);
//	// For advanced node-based, use ImNodes if enabled
//	if (modules["advanced_mode"].get<bool>())
//	{
//		ImNodes::BeginNodeEditor();
//		// Todo: Niagara-style graph
//		ImNodes::EndNodeEditor();
//	}
//}
//bool ParticleEditor::EditCurve(const char* label, nlohmann::json& curve)
//{
//	ImGui::Text(label);
//	ImGui::BeginChild(label, ImVec2(0, 100));
//	ImDrawList* dl = ImGui::GetWindowDrawList();
//	ImVec2 pos = ImGui::GetCursorScreenPos();
//	ImVec2 size = ImGui::GetContentRegionAvail();
//	dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(50, 50, 50, 255));
//	dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(255, 255, 255, 255));
//	bool changed = false;
//	for (size_t i = 0; i < curve.size() - 1; i++)
//	{
//		ImVec2 p1 = { pos.x + curve[i][0].get<float>() * size.x, pos.y + (1 - curve[i][1].get<float>()) * size.y };
//		ImVec2 p2 = { pos.x + curve[i + 1][0].get<float>() * size.x, pos.y + (1 - curve[i + 1][1].get<float>()) * size.y };
//		dl->AddLine(p1, p2, IM_COL32(0, 255, 0, 255), 2.0f);
//	}
//	for (size_t i = 0; i < curve.size(); i++)
//	{
//		ImVec2 p = { pos.x + curve[i][0].get<float>() * size.x, pos.y + (1 - curve[i][1].get<float>()) * size.y };
//		dl->AddCircleFilled(p, 4.0f, IM_COL32(255, 0, 0, 255));
//		ImGui::SetCursorScreenPos(p - ImVec2(4, 4));
//		ImGui::InvisibleButton(("point##" + std::to_string(i) + label).c_str(), ImVec2(8, 8));
//		if (ImGui::IsItemActive())
//		{
//			ImVec2 delta = ImGui::GetIO().MouseDelta;
//			curve[i][0] = ImClamp(curve[i][0].get<float>() + delta.x / size.x, 0.0f, 1.0f);
//			curve[i][1] = ImClamp(curve[i][1].get<float>() - delta.y / size.y, 0.0f, 1.0f);
//			changed = true;
//		}
//	}
//	if (ImGui::IsWindowHovered() && ImGui::IsMouseDoubleClicked(0))
//	{
//		float t = (ImGui::GetIO().MousePos.x - pos.x) / size.x;
//		float v = 1 - (ImGui::GetIO().MousePos.y - pos.y) / size.y;
//		size_t idx = 0;
//		for (; idx < curve.size(); idx++)
//			if (curve[idx][0] > t) break;
//		curve.insert(curve.begin() + idx, { t, v });
//		changed = true;
//	}
//	ImGui::EndChild();
//	return changed;
//}
//bool ParticleEditor::EditGradient(const char* label, nlohmann::json& gradient)
//{
//	// Similar to curve, but draw color bar
//	ImGui::Text(label);
//	ImGui::BeginChild(label, ImVec2(0, 30));
//	ImDrawList* dl = ImGui::GetWindowDrawList();
//	ImVec2 pos = ImGui::GetCursorScreenPos();
//	ImVec2 size = ImGui::GetContentRegionAvail();
//	for (size_t i = 0; i < gradient.size() - 1; i++)
//	{
//		float t1 = gradient[i][0].get<float>();
//		float t2 = gradient[i + 1][0].get<float>();
//		Color c1 = { gradient[i][1][0], gradient[i][1][1], gradient[i][1][2], 255 };
//		Color c2 = { gradient[i + 1][1][0], gradient[i + 1][1][1], gradient[i + 1][1][2], 255 };
//		dl->AddRectFilledMultiColor(ImVec2(pos.x + t1 * size.x, pos.y), ImVec2(pos.x + t2 * size.x, pos.y + size.y), c1, c2, c2, c1);
//	}
//	// Todo: points, drag, color picker
//	ImGui::EndChild();
//	return false; // Todo: implement change
//}
//void ParticleEditor::DrawGizmos(nlohmann::json& emitter)
//{
//	// Todo: Draw shape gizmo, forces
//	std::string type = emitter["modules"]["shape"]["type"].get<std::string>();
//	Vector3 pos = { 0,0,0 }; // Relative
//	if (type == "sphere")
//		DrawSphereWires(pos, emitter["modules"]["shape"]["radius"].get<float>(), 10, 10, BLUE);
//	// Todo: other shapes
//}
//void ParticleEditor::SaveParticles()
//{
//	if (!particles) return;
//	particles->SetData(particlesData);
//}
//void ParticleEditor::LoadParticles(Particles* particles)
//{
//	if (!particles->GetData())
//	{
//		ConsoleLogger::ErrorLog("Particle Editor: Failed to load. Invalid particles data.");
//		particlesData = nlohmann::json();
//		return;
//	}
//	this->particles = particles;
//	particlesData = *particles->GetData();
//	// Setup preview
//	if (previewObject) delete previewObject;
//	previewObject = new GameObject();
//	previewRenderer = previewObject->AddComponent<ParticleRenderer>();
//	previewRenderer->SetParticles(particles);
//	previewCamera = { {0,10,10}, {0,0,0}, {0,1,0}, 45, CAMERA_PERSPECTIVE };
//	previewTexture = LoadRenderTexture(800, 450); // Initial
//	// Todo: Sync exposed, similar to CanvasEditor
//}