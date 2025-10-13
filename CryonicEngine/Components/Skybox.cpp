#include "Skybox.h"
#include "rlgl.h"
#include "../RaylibWrapper.h"
#include <algorithm>
#if defined(EDITOR)
#include "../ProjectManager.h"
#include "../Editor.h"
#else
#include "../Game.h"
#endif

std::vector<CameraComponent*> Skybox::cameras;
std::vector<Skybox*> Skybox::skyboxes;

void Skybox::Awake()
{
	// Todo: Use Events instead. Make sure to have an event for when they are destroyed too
	for (GameObject* go : SceneManager::GetActiveScene()->GetGameObjects())
	{
		CameraComponent* camera = go->GetComponent<CameraComponent>();
		if (camera)
			cameras.push_back(camera);
	}

	// Make sure the skybox is only rendered in 3D projects
#if defined(EDITOR)
	if (!ProjectManager::projectData.is3D)
		return;
#elif defined(IS2D)
	return;
#endif

	// Create model (assuming this loads/inits the skyShader via ShaderManager)
	skyboxModel.Create(ModelType::Skybox, "Skybox", skyShader, "");

	// Set cubemap shader values (unchanged, but move equirectangularMap here if needed)
	std::pair<unsigned int, int*> cubemapShaderPair = ShaderManager::GetShader(cubemapShader);
	RaylibWrapper::Shader rlCubeShader = { cubemapShaderPair.first, cubemapShaderPair.second };
	int equirectangularMap[1] = { 0 };
	RaylibWrapper::SetShaderValue(rlCubeShader, RaylibWrapper::GetShaderLocation(rlCubeShader, "equirectangularMap"), equirectangularMap, RaylibWrapper::SHADER_UNIFORM_INT);

	// Initial load of texture
	LoadSkyTexture();

	// Set skybox shader values
	std::pair<unsigned int, int*> skyShaderPair = ShaderManager::GetShader(skyShader);
	RaylibWrapper::Shader rlSkyShader = { skyShaderPair.first, skyShaderPair.second };

	int materialMap[1] = { RaylibWrapper::MATERIAL_MAP_CUBEMAP };
	RaylibWrapper::SetShaderValue(rlSkyShader, RaylibWrapper::GetShaderLocation(rlSkyShader, "environmentMap"), materialMap, RaylibWrapper::SHADER_UNIFORM_INT);

	int doGamma[1] = { useHDRI ? 1 : 0 };
	RaylibWrapper::SetShaderValue(rlSkyShader, RaylibWrapper::GetShaderLocation(rlSkyShader, "doGamma"), doGamma, RaylibWrapper::SHADER_UNIFORM_INT);

	int vflipped[1] = { useHDRI ? 1 : 0 };
	RaylibWrapper::SetShaderValue(rlSkyShader, RaylibWrapper::GetShaderLocation(rlSkyShader, "vflipped"), vflipped, RaylibWrapper::SHADER_UNIFORM_INT);

	// Set previous paths for change detection
	previousHdriPath = hdriTexturePath;

	needsReload = false;
	skyboxes.push_back(this);
}

void Skybox::Update()
{
	// Reload texture if needed
	if (needsReload)
	{
		LoadSkyTexture();
		needsReload = false;
	}
}

void Skybox::RenderSkyboxes()
{
	for (Skybox* skybox : skyboxes)
		skybox->RenderSkybox();
}

// This must have its own function because skyboxes need to be rendered before everything else
void Skybox::RenderSkybox()
{
	if (!gameObject->IsActive() || !gameObject->IsGlobalActive() || !IsActive())
		return;

	rlDisableBackfaceCulling();
	rlDisableDepthMask();
#if defined(EDITOR)
	skyboxModel.DrawModelWrapper(Editor::camera.position.x, Editor::camera.position.y, Editor::camera.position.z, skyboxScale, skyboxScale, skyboxScale, 0, 0, 0, 1, color.r, color.g, color.b, color.a);
#else
	for (CameraComponent* camera : cameras)
	{
		if (!camera)
		{
			RemoveCamera(camera);
			continue;
		}

		if (camera && camera->IsActive() && camera->GetGameObject()->IsGlobalActive())
		{
			Vector3 camPos = camera->GetGameObject()->transform.GetPosition();
			skyboxModel.DrawModelWrapper(camPos.x, camPos.y, camPos.z, skyboxScale, skyboxScale, skyboxScale, 0, 0, 0, 1, color.r, color.g, color.b, color.a);
		}
	}
#endif
	rlEnableBackfaceCulling();
	rlEnableDepthMask();
}

#if defined(EDITOR)
void Skybox::EditorUpdate()
{
	if (!setupInEditor)
	{
		Awake();
		setupInEditor = true;
	}

	color.r = exposedVariables[1][0][2][0].get<int>();
	color.g = exposedVariables[1][0][2][1].get<int>();
	color.b = exposedVariables[1][0][2][2].get<int>();
	color.a = exposedVariables[1][0][2][3].get<int>();

	std::string spritePath = exposedVariables[1][1][2].get<std::string>();
	bool spriteChanged = false;
	if ((!sprite && spritePath != "nullptr") || (sprite && sprite->GetRelativePath() != spritePath))
	{
		if (sprite)
			delete sprite;

		sprite = new Sprite(spritePath);
		spriteChanged = true;
	}

	std::string newHdrPath = exposedVariables[1][2][2].get<std::string>();
	bool hdrChanged = (hdriTexturePath != newHdrPath);
	hdriTexturePath = newHdrPath;

	previousHdriPath = hdriTexturePath;

	if (spriteChanged || hdrChanged)
		needsReload = true;
}
#endif

void Skybox::Destroy()
{
	if (sprite)
		delete sprite;

	if (panorama.id != 0)
		RaylibWrapper::UnloadTexture(panorama);

	RaylibWrapper::UnloadTexture(environmentMap);
	skyboxModel.DeleteInstance();

	auto it = std::find(skyboxes.begin(), skyboxes.end(), this);
	if (it != skyboxes.end())
		skyboxes.erase(it);
}

void Skybox::LoadSkyTexture()
{
	// Unload previous cubemap
	if (environmentMap.id != 0)
	{
		RaylibWrapper::UnloadTexture(environmentMap);
		environmentMap = { 0 };
	}

	// Determine mode
	useHDRI = (!hdriTexturePath.empty() && hdriTexturePath != "nullptr");

	std::string absoluteHdriPath;

	if (useHDRI)
	{
		for (char& c : hdriTexturePath) // Reformatted the path for unix.
		{
			if (c == '\\')
				c = '/';
		}

#if defined(EDITOR)
		absoluteHdriPath = ProjectManager::projectData.path.string() + "/Assets/" + hdriTexturePath;
#else
		if (exeParent.empty())
			absoluteHdriPath = "Resources/Assets/" + hdriTexturePath;
		else
			absoluteHdriPath = exeParent.string() + "/Resources/Assets/" + hdriTexturePath;
#endif

		if (!std::filesystem::exists(absoluteHdriPath))
			useHDRI = false;
	}

	if (useHDRI)
	{
		// Load HDR panorama
		if (panorama.id != 0)
			RaylibWrapper::UnloadTexture(panorama);

		panorama = RaylibWrapper::LoadTexture(absoluteHdriPath.c_str());

		// Generate cubemap from panorama
		environmentMap = GenTextureCubemap(cubemapShader, panorama, 1024, RaylibWrapper::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

		// Unload panorama
		RaylibWrapper::UnloadTexture(panorama);
		panorama = { 0 };
	}
	else {
		// Use sprite (non-HDR cubemap layout)
		if (sprite)
		{
			std::string path = sprite->GetPath();
			if (!path.empty() && path != "nullptr")
			{
				RaylibWrapper::Image img = RaylibWrapper::LoadImage(path.c_str());
				environmentMap = LoadTextureCubemap(img, RaylibWrapper::CUBEMAP_LAYOUT_AUTO_DETECT);
				RaylibWrapper::UnloadImage(img);
			}
		}
	}

	// Assign cubemap to material map
	if (environmentMap.id != 0)
		skyboxModel.SetMaterialMap(0, RaylibWrapper::MATERIAL_MAP_CUBEMAP, environmentMap, { 255, 255, 255, 255 }, 1);

	std::pair<unsigned int, int*> skyShaderPair = ShaderManager::GetShader(skyShader);
	RaylibWrapper::Shader rlSkyShader = { skyShaderPair.first, skyShaderPair.second };
	int doGamma[1] = { useHDRI ? 1 : 0 };
	RaylibWrapper::SetShaderValue(rlSkyShader, RaylibWrapper::GetShaderLocation(rlSkyShader, "doGamma"), doGamma, RaylibWrapper::SHADER_UNIFORM_INT);
	int vflipped[1] = { useHDRI ? 1 : 0 };
	RaylibWrapper::SetShaderValue(rlSkyShader, RaylibWrapper::GetShaderLocation(rlSkyShader, "vflipped"), vflipped, RaylibWrapper::SHADER_UNIFORM_INT);
}

RaylibWrapper::TextureCubemap Skybox::GenTextureCubemap(ShaderManager::Shaders shader, RaylibWrapper::Texture2D panorama, int size, int format)
{
	RaylibWrapper::TextureCubemap cubemap = { 0 };
	std::pair<unsigned int, int*> shaderPair = ShaderManager::GetShader(shader); // first = id, second = locs

	rlDisableBackfaceCulling(); // Disable backface culling to render inside the cube

	// Setup framebuffer
	unsigned int rbo = rlLoadTextureDepth(size, size, true);
	cubemap.id = rlLoadTextureCubemap(0, size, format, 1);

	unsigned int fbo = rlLoadFramebuffer();
	rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
	rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);

	//if (!rlFramebufferComplete(fbo)) {}

	// Draw to framebuffer
	// NOTE: Shader is used to convert HDR equirectangular environment map to cubemap equivalent (6 faces)
	rlEnableShader(shaderPair.first);

	// Define projection matrix and send it to shader
	RaylibWrapper::Matrix matFboProjection = RaylibWrapper::MatrixPerspective(90.0f * DEG2RAD, 1.0f, rlGetCullDistanceNear(), rlGetCullDistanceFar());
	rlSetUniformMatrix(shaderPair.second[RaylibWrapper::SHADER_LOC_MATRIX_PROJECTION], matFboProjection);

	// Define view matrix for every side of the cubemap
	RaylibWrapper::Matrix fboViews[6] = {
		RaylibWrapper::MatrixLookAt({ 0.0f, 0.0f, 0.0f }, { 1.0f,  0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f }),
		RaylibWrapper::MatrixLookAt({ 0.0f, 0.0f, 0.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f }),
		RaylibWrapper::MatrixLookAt({ 0.0f, 0.0f, 0.0f }, { 0.0f,  1.0f,  0.0f }, { 0.0f,  0.0f,  1.0f }),
		RaylibWrapper::MatrixLookAt({ 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f,  0.0f }, { 0.0f,  0.0f, -1.0f }),
		RaylibWrapper::MatrixLookAt({ 0.0f, 0.0f, 0.0f }, { 0.0f,  0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f }),
		RaylibWrapper::MatrixLookAt({ 0.0f, 0.0f, 0.0f }, { 0.0f,  0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f })
	};

	rlViewport(0, 0, size, size);   // Set viewport to current fbo dimensions

	// Activate and enable texture for drawing to cubemap faces
	rlActiveTextureSlot(0);
	rlEnableTexture(panorama.id);

	for (int i = 0; i < 6; i++)
	{
		// Set the view matrix for the current cube face
		rlSetUniformMatrix(shaderPair.second[RaylibWrapper::SHADER_LOC_MATRIX_VIEW], fboViews[i]);

		// Select the current cubemap face attachment for the fbo
		// This function by default enables->attach->disables fbo
		rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X + i, 0);
		rlEnableFramebuffer(fbo);

		// Load and draw a cube, it uses the current enabled texture
		rlClearScreenBuffers();
		rlLoadDrawCube();
	}

	// Unload framebuffer and reset state
	rlDisableShader();          // Unbind shader
	rlDisableTexture();         // Unbind texture
	rlDisableFramebuffer();     // Unbind framebuffer
	rlUnloadFramebuffer(fbo);   // Unload framebuffer (and automatically attached depth texture/renderbuffer)

	// Reset viewport dimensions to default
	rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
	rlEnableBackfaceCulling();

	cubemap.width = size;
	cubemap.height = size;
	cubemap.mipmaps = 1;
	cubemap.format = format;

	return cubemap;
}

void Skybox::SetColor(Color color)
{
	this->color = color;
}

void Skybox::SetSprite(Sprite* newSprite)
{
	if (sprite != newSprite) {
		sprite = newSprite;
		needsReload = true;
	}
}

void Skybox::SetHDRITexturePath(const std::string& path)
{
	if (hdriTexturePath != path)
	{
		hdriTexturePath = path;
		needsReload = true;
	}
}

void Skybox::AddCamera(CameraComponent* camera)
{
	cameras.push_back(camera);
}

void Skybox::RemoveCamera(CameraComponent* camera)
{
	auto it = std::find(cameras.begin(), cameras.end(), camera);
	if (it != cameras.end())
		cameras.erase(it);
}