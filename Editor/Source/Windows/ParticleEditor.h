//#pragma once
//#include "EditorWindow.h"
//#include "Core/CryonicCore.h"
//#include "ThirdParty/Misc/json.hpp"
//#include "Core/GameObject.h"
//#include "Raylib/RaylibWrapper.h"
//#include "Resources/Particles.h"
//#include "Components/ParticleRenderer.h"
//#include <ctime>
//class ParticleEditor : public EditorWindow
//{
//public:
//	void Render();
//	void SaveParticles();
//	void LoadParticles(Particles* particles);
//private:
//	void RenderPreview();
//	void RenderHierarchy();
//	void RenderProperties();
//	void DrawGizmos(nlohmann::json& emitter);
//	bool RenderHierarchyNode(nlohmann::json* emitter, bool normalColor, bool& childDoubleClicked, nlohmann::json* objectHovered);
//	bool EditCurve(const char* label, nlohmann::json& curve);
//	bool EditGradient(const char* label, nlohmann::json& gradient);
//	Particles* particles = nullptr;
//	nlohmann::json particlesData;
//	nlohmann::json* selectedObject = nullptr;
//	bool hierarchyObjectClicked = false;
//	nlohmann::json* objectInProperties = nullptr;
//	nlohmann::json* objectInHierarchyContextMenu = nullptr;
//	bool hierarchyContextMenuOpen = false;
//	GameObject* previewObject = nullptr;
//	ParticleRenderer* previewRenderer = nullptr;
//	RenderTexture2D previewTexture;
//	Camera3D previewCamera;
//	float currentTime = 0.0f;
//	bool playing = false;
//	float playbackSpeed = 1.0f;
//	bool scrubbing = false;
//	Vector2 orbitAngles = { 0,0 };
//	float cameraDistance = 10.0f;
//	ImVec2 previewSize = { 0,0 };
//};