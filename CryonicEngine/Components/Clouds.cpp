#include "Clouds.h"
#if defined(EDITOR)
#include "../ProjectManager.h"
#include "../Editor.h"
#else
#include "../Game.h"
#endif
#include <cmath>
#include "rlgl.h"

std::vector<Clouds*> Clouds::clouds;

void Clouds::Awake()
{
	if (!cloudMaterial)
	{
		cloudMaterial = new Material("CloudDefault");
		ownMaterial = true;

		// Set base color to white/translucent for clouds
		cloudMaterial->SetAlbedoColor({ 255, 255, 255, 128 });

		std::pair<unsigned int, int*> cloudShaderPair = ShaderManager::GetShader(ShaderManager::Shaders::Clouds);
		if (cloudShaderPair.first == 0)
		{
			delete cloudMaterial;
			cloudMaterial = nullptr;
			return;
		}
		cloudMaterial->SetShader({ cloudShaderPair.first, cloudShaderPair.second });
	}
	startTime = RaylibWrapper::GetTime();

	if (doOnce) // This should be removed once the code stops calling Awake() to create new materials
	{
		clouds.push_back(this);
		doOnce = false;
	}
}

void Clouds::Start()
{
	// Create a full-screen quad for post-processing the clouds
	modelSet = raylibModel.Create(ModelType::Plane, "Clouds", ShaderManager::Shaders::Clouds, "", { 2.0f, 2.0f });  // Todo: Test with the last parameter empty. This sets the length and width to 2
	raylibModel.SetMaterials({ cloudMaterial->GetRaylibMaterial() });
}

void Clouds::Update()
{
	// Animate wind/time
	// Time is handled in shader, but can offset here if needed
}

void Clouds::RenderClouds()
{
	for (Clouds* cloud : clouds)
		cloud->RenderCloud();
}

void Clouds::RenderCloud()
{
	if (!modelSet || !gameObject->IsActive() || !gameObject->IsGlobalActive() || !IsActive())
		return;

	// Todo: We will likely need to render for each active camera

	RaylibWrapper::rlSetBlendMode(RaylibWrapper::RL_BLEND_ADDITIVE);  // Additive for cloud glow/scattering

	// Get camera data
	Vector3 cameraPos;
	RaylibWrapper::Matrix proj;
	RaylibWrapper::Matrix view;
	float aspect = (float)RaylibWrapper::GetScreenWidth() / (float)RaylibWrapper::GetScreenHeight();

#if defined(EDITOR)
	cameraPos = { Editor::camera.position.x, Editor::camera.position.y, Editor::camera.position.z };
	view = RaylibWrapper::GetCameraMatrix(Editor::camera);

	// Todo: These need to get the near and far plane from the camera
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	if (Editor::camera.projection == RaylibWrapper::CAMERA_PERSPECTIVE)
		proj = RaylibWrapper::MatrixPerspective(Editor::camera.fovy * DEG2RAD, aspect, nearPlane, farPlane);
	else
	{
		float orthoWidth = Editor::camera.fovy;
		float orthoHeight = orthoWidth / aspect;
		proj = RaylibWrapper::MatrixOrtho(-orthoWidth / 2, orthoWidth / 2, -orthoHeight / 2, orthoHeight / 2, nearPlane, farPlane);
	}

#else
	// Todo: This may not be correct if the main camera changes. Use Events to know when the main camera changes
	if (!mainCamera)
		mainCamera = CameraComponent::main;

	if (mainCamera)
	{
		cameraPos = mainCamera->GetGameObject()->transform.GetPosition();
		view = mainCamera->raylibCamera.GetCameraMatrix();

		// Todo: These need to get the near and far plane from the camera
		float nearPlane = 0.1f;
		float farPlane = 1000.0f;

		if (mainCamera->raylibCamera.GetProjection() == RaylibWrapper::CAMERA_PERSPECTIVE)
			proj = RaylibWrapper::MatrixPerspective(mainCamera->raylibCamera.GetFOVY() * DEG2RAD, aspect, nearPlane, farPlane);
		else
		{
			float orthoWidth = mainCamera->raylibCamera.GetFOVY();
			float orthoHeight = orthoWidth / aspect;
			proj = RaylibWrapper::MatrixOrtho(-orthoWidth / 2, orthoWidth / 2, -orthoHeight / 2, orthoHeight / 2, nearPlane, farPlane);
		}
	}
	else
		return;
#endif

	RaylibWrapper::Shader shader = cloudMaterial->GetRaylibMaterial()->shader;
	float timeVal = RaylibWrapper::GetTime() - startTime;

	// Time uniform
	int locTime = RaylibWrapper::GetShaderLocation(shader, "time");
	RaylibWrapper::SetShaderValue(shader, locTime, &timeVal, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	// Cloud params
	int locCoverage = RaylibWrapper::GetShaderLocation(shader, "coverage");
	RaylibWrapper::SetShaderValue(shader, locCoverage, &coverage, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locDensity = RaylibWrapper::GetShaderLocation(shader, "density");
	RaylibWrapper::SetShaderValue(shader, locDensity, &density, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	float windArr[2] = { windDir.x * windSpeed, windDir.y * windSpeed };
	int locWind = RaylibWrapper::GetShaderLocation(shader, "wind");
	RaylibWrapper::SetShaderValue(shader, locWind, windArr, RaylibWrapper::SHADER_UNIFORM_VEC2);

	int locHeight = RaylibWrapper::GetShaderLocation(shader, "cloudHeight");
	RaylibWrapper::SetShaderValue(shader, locHeight, &cloudHeight, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locThickness = RaylibWrapper::GetShaderLocation(shader, "cloudThickness");
	RaylibWrapper::SetShaderValue(shader, locThickness, &cloudThickness, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locSteps = RaylibWrapper::GetShaderLocation(shader, "raymarchSteps");
	int stepsInt = highQuality ? raymarchSteps * 2 : raymarchSteps;  // Double for HQ
	RaylibWrapper::SetShaderValue(shader, locSteps, &stepsInt, RaylibWrapper::SHADER_UNIFORM_INT);

	int locAbsorption = RaylibWrapper::GetShaderLocation(shader, "absorption");
	RaylibWrapper::SetShaderValue(shader, locAbsorption, &absorption, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locScattering = RaylibWrapper::GetShaderLocation(shader, "scattering");
	RaylibWrapper::SetShaderValue(shader, locScattering, &scattering, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locPhaseG = RaylibWrapper::GetShaderLocation(shader, "phaseG");
	RaylibWrapper::SetShaderValue(shader, locPhaseG, &phaseG, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	// Sun
	float sunDirArr[3] = { sunDir.x, sunDir.y, sunDir.z };
	int locSunDir = RaylibWrapper::GetShaderLocation(shader, "sunDir");
	RaylibWrapper::SetShaderValue(shader, locSunDir, sunDirArr, RaylibWrapper::SHADER_UNIFORM_VEC3);

	float sunColArr[3] = { sunColor.x, sunColor.y, sunColor.z };
	int locSunCol = RaylibWrapper::GetShaderLocation(shader, "sunColor");
	RaylibWrapper::SetShaderValue(shader, locSunCol, sunColArr, RaylibWrapper::SHADER_UNIFORM_VEC3);

	int locSunInt = RaylibWrapper::GetShaderLocation(shader, "sunIntensity");
	RaylibWrapper::SetShaderValue(shader, locSunInt, &sunIntensity, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locAmbient = RaylibWrapper::GetShaderLocation(shader, "ambientLight");
	RaylibWrapper::SetShaderValue(shader, locAmbient, &ambientLight, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	// Camera
	float camPosArr[3] = { cameraPos.x, cameraPos.y, cameraPos.z };
	int locCamPos = RaylibWrapper::GetShaderLocation(shader, "cameraPos");
	RaylibWrapper::SetShaderValue(shader, locCamPos, camPosArr, RaylibWrapper::SHADER_UNIFORM_VEC3);

	// Inv view proj for ray dir computation
	RaylibWrapper::Matrix invViewProj = RaylibWrapper::MatrixMultiply(RaylibWrapper::MatrixInvert(proj), RaylibWrapper::MatrixInvert(view));
	int locInvVP = RaylibWrapper::GetShaderLocation(shader, "invViewProj");
	RaylibWrapper::SetShaderValueMatrix(shader, locInvVP, invViewProj);

	// HQ toggle
	int locHQ = RaylibWrapper::GetShaderLocation(shader, "highQuality");
	int highQualityInt = highQuality ? 1 : 0;  // Convert bool to int
	RaylibWrapper::SetShaderValue(shader, locHQ, &highQualityInt, RaylibWrapper::SHADER_UNIFORM_INT);

	// Draw full-screen quad in NDC (ortho projection for screen-space rendering)
	raylibModel.DrawModelWrapper(0, 0, 0, 1, 1, -1, 0.707f, 0.0f, 0.0f, 0.707f, 255, 255, 255, 255, true, true, true);

	// Reset blend... Is this needed?
	RaylibWrapper::rlSetBlendMode(RaylibWrapper::RL_BLEND_ALPHA);
}

#if defined(EDITOR)
void Clouds::EditorUpdate()
{
	if (!editorSetup)
	{
		Awake();
		Start();
		editorSetup = true;
	}

	// Update from exposed variables
	coverage = exposedVariables[1][0][2].get<float>();
	density = exposedVariables[1][1][2].get<float>();
	windSpeed = exposedVariables[1][2][2].get<float>();
	windDir = { exposedVariables[1][3][2][0].get<float>(), exposedVariables[1][3][2][1].get<float>() };
	cloudHeight = exposedVariables[1][4][2].get<float>();
	cloudThickness = exposedVariables[1][5][2].get<float>();
	raymarchSteps = exposedVariables[1][6][2].get<int>();
	absorption = exposedVariables[1][7][2].get<float>();
	scattering = exposedVariables[1][8][2].get<float>();
	phaseG = exposedVariables[1][9][2].get<float>();
	sunDir = { exposedVariables[1][10][2][0].get<float>(), exposedVariables[1][10][2][1].get<float>(), exposedVariables[1][10][2][2].get<float>() };
	sunColor = { exposedVariables[1][11][2][0].get<float>(), exposedVariables[1][11][2][1].get<float>(), exposedVariables[1][11][2][2].get<float>() };
	sunIntensity = exposedVariables[1][12][2].get<float>();
	ambientLight = exposedVariables[1][13][2].get<float>();
	highQuality = exposedVariables[1][14][2].get<bool>();

	// Regenerate mode
	if (modelSet && exposedVariables[1][6][2] != raymarchSteps)
	{
		raymarchSteps = exposedVariables[1][6][2].get<int>();
		Start(); // Todo: Move the model regeneration to a separate function
	}
}
#endif

void Clouds::Destroy()
{
	if (modelSet)
		raylibModel.DeleteInstance();

	if (ownMaterial)
		delete cloudMaterial;

	auto it = std::find(clouds.begin(), clouds.end(), this);
	if (it != clouds.end())
		clouds.erase(it);
}