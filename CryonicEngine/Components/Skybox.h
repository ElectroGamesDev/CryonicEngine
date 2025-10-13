#pragma once

#include "Component.h"
#include "../CryonicCore.h"
#include "../Sprite.h"
#include "../RaylibWrapper.h"
#include "../RaylibModelWrapper.h"
#include "CameraComponent.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

class Skybox : public Component
{
public:
	Skybox(GameObject* obj, int id) : Component(obj, id)
	{
		runInEditor = true;
		name = "Skybox";
		iconUnicode = "\xef\x87\xbe";

#if defined(EDITOR)
		std::string variables = R"(
        [
            0,
            [
                [
                    "Color",
                    "color",
                    [ 255, 255, 255, 255 ],
                    "Color"
                ],
                [
                    "Sprite",
                    "sprite",
                    "nullptr",
                    "Texture",
                    {
                        "Extensions": [".png", ".jpg", ".jpeg"]
                    }
                ],
                [
                    "string",
                    "hdriTexturePath",
                    "nullptr",
                    "HDRI Texture",
                    {
                        "Extensions": [".hdr"]
                    }
                ]
            ]
        ]
    )";
		exposedVariables = nlohmann::json::parse(variables);
#endif

		sprite = nullptr;
		hdriTexturePath = "";
		previousHdriPath = "";
		needsReload = true;
		useHDRI = false;
	}
	Skybox* Clone() override
	{
		return new Skybox(gameObject, -1);
	}

	// Hide everything from API
	void Awake() override;
	void Update() override;
#if defined(EDITOR)
	void EditorUpdate() override;
#endif
	void Destroy() override;

	void SetColor(Color color);
	Color GetColor() const { return color; }

	void SetSprite(Sprite* newSprite);
	void SetHDRITexturePath(const std::string& path);

	// Hide in API
	static void AddCamera(CameraComponent* camera);
	static void RemoveCamera(CameraComponent* camera);
	
	static void RenderSkyboxes();
	void RenderSkybox();

	Color color = { 255, 255, 255, 255 };

private:
	Sprite* sprite = nullptr;
	std::string hdriTexturePath;
	std::string previousHdriPath;
	bool needsReload;
	bool useHDRI;
	bool setupInEditor = false;

	RaylibModel skyboxModel;
	ShaderManager::Shaders skyShader = ShaderManager::Skybox;
	ShaderManager::Shaders cubemapShader = ShaderManager::Cubemap;
	RaylibWrapper::TextureCubemap environmentMap = { 0 };
	RaylibWrapper::Texture2D panorama = { 0 };
	float skyboxScale = 1000.0f;
	CameraComponent* mainCamera = nullptr;

	void LoadSkyTexture();
	RaylibWrapper::TextureCubemap GenTextureCubemap(ShaderManager::Shaders shader, RaylibWrapper::Texture2D panorama, int size, int format);

	static std::vector<CameraComponent*> cameras;
	static std::vector<Skybox*> skyboxes;
};