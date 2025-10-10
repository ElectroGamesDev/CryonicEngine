#pragma once

#include "GameObject.h"
#include "RaylibWrapper.h"
#include <json.hpp>
#include <variant>
#include "AsyncPBODisplay.h"

enum class DataFileTypes
{
	Sprite,
	Tilemap,
	Sound,
	Material,
	Other
};

struct DataFile
{
	std::string path;
	nlohmann::json json;
	DataFileTypes type;
};

class Editor
{
public:
	RaylibWrapper::RenderTexture2D* CreateModelPreview(std::filesystem::path modelPath, int textureSize);
	RaylibWrapper::RenderTexture2D* CreateMaterialPreview(std::filesystem::path materialPath, int textureSize);
	AsyncPBODisplay asyncPBODisplay;

	void InitFonts();
	void InitStyle();
	void InitMisc();
	void InitScenes();
	void InitImages();
	void Init();
	void InitMaterialPreview();

	void RenderViewport();
	void RenderGameView();
	void RenderContentBrowserTreeNode(std::filesystem::path path, bool openOnDefault);
	void RenderContentBrowser();
	int RenderColorPicker(std::string name, ImVec2 position, ImVec4& selectedColor, ImVec4& previousColor); // Does this really need to be in here?
	void RenderProperties();
	bool RenderHierarchyNode(GameObject* gameObject, bool normalColor, bool& childDoubleClicked);
	void RenderHierarchy();
	void RenderTopbar();
	void RenderComponentsWin();
	void RenderScriptCreateWin();
	void RenderCameraView();
	void RenderConsole();
	void RenderAnimationGraph();
	void RenderProjectSettings();
	void RenderTerrainToolsWin();
	void Render();

	void OnBuildFinish(int success, bool debug);
	void EnterPlayMode();
	void ExitPlayMode();

	void UpdateViewport();
	void SetupViewport();

	void Cleanup();
	void CleanupCache();

	static RaylibWrapper::Camera camera;
	static RaylibWrapper::Camera2D camera2D;

	static std::variant<std::monostate, GameObject*, DataFile> objectInProperties;

	static Vector4 viewportPosition;
};