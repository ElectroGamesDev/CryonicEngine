#include "Clouds.h"
#if defined(EDITOR)
#include "Core/ProjectManager.h"
#include "Core/Editor.h"
#else
#include "Game.h"
#endif
#include <cmath>
#include "Thirdparty/raylib/include/rlgl.h"

#include "Thirdparty/raylib/include/external/glad.h"

std::vector<Clouds*> Clouds::clouds;

static void GetTexture2DSize(unsigned int texId, int* width, int* height)
{
	glBindTexture(GL_TEXTURE_2D, texId);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, height);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Clouds::Awake()
{
	startTime = RaylibWrapper::GetTime();

	if (doOnce) // This should be removed once the code stops calling Awake() to create new materials
	{
		doOnce = false;
		clouds.push_back(this);

		// Load shaders
		shader = ShaderManager::GetShader(ShaderManager::Clouds);
		raymarchShader = ShaderManager::GetComputeShader(ShaderManager::ComputeShaders::CloudsRaymarch);

		noiseShader = ShaderManager::GetComputeShader(ShaderManager::ComputeShaders::CloudsNoise);
		int width = RaylibWrapper::GetScreenWidth();
		int height = RaylibWrapper::GetScreenHeight();
		// Half-res textures
		halfCloudTexture = RaylibWrapper::rlLoadTexture(NULL, width / 2, height / 2, RaylibWrapper::PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, 1);
		previousCloudTexture = RaylibWrapper::rlLoadTexture(NULL, width / 2, height / 2, RaylibWrapper::PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, 1);

		std::vector<float> clearData((width / 2) * (height / 2) * 4, 0.0f);

		glBindTexture(GL_TEXTURE_2D, halfCloudTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RGBA, GL_FLOAT, clearData.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, previousCloudTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RGBA, GL_FLOAT, clearData.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, 0);

		// Noise 3D texture (use direct GL since Raylib lacks 3D support)
		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_3D, noiseTexture);
		glTexStorage3D(GL_TEXTURE_3D, 1, GL_RG32F, 64, 64, 64);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glBindTexture(GL_TEXTURE_3D, 0);
	}
}

void Clouds::Start()
{
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

void CheckGLError(const char* location) {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		printf("OpenGL Error at %s: 0x%x\n", location, err);
	}
}

void Clouds::RenderCloud()
{
	if (!gameObject->IsActive() || !gameObject->IsGlobalActive() || !IsActive())
		return;

	// Get camera data
	Vector3 cameraPos;
	RaylibWrapper::Matrix proj;
	RaylibWrapper::Matrix view;
	float aspect = (float)RaylibWrapper::GetScreenWidth() / (float)RaylibWrapper::GetScreenHeight();
#if defined(EDITOR)
	cameraPos = { Editor::camera.position.x, Editor::camera.position.y, Editor::camera.position.z };
	view = RaylibWrapper::GetCameraMatrix(Editor::camera);
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
	if (!mainCamera)
		mainCamera = CameraComponent::main;
	if (mainCamera)
	{
		cameraPos = mainCamera->GetGameObject()->transform.GetPosition();
		view = mainCamera->raylibCamera.GetCameraMatrix();
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
	RaylibWrapper::rlSetBlendMode(RaylibWrapper::RL_BLEND_ALPHA);

	RaylibWrapper::Matrix viewProj = RaylibWrapper::MatrixMultiply(view, proj);
	RaylibWrapper::Matrix invViewProj = RaylibWrapper::MatrixInvert(viewProj);
	RaylibWrapper::Matrix prevViewProj = firstFrame ? viewProj : RaylibWrapper::MatrixMultiply(prevView, prevProj);
	float timeVal = RaylibWrapper::GetTime() - startTime;

	// Bind textures (swap read/write each frame)
	unsigned int readTexture = (currentCloudTexture == 0) ? previousCloudTexture : halfCloudTexture;
	unsigned int writeTexture = (currentCloudTexture == 0) ? halfCloudTexture : previousCloudTexture;

	// Noise Generation Pass

	rlEnableShader(noiseShader);
	glBindImageTexture(0, noiseTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG32F);

	// Set uniforms for noise
	int locTimeNoise = rlGetLocationUniform(noiseShader, "time");
	rlSetUniform(locTimeNoise, &timeVal, (int)RaylibWrapper::SHADER_UNIFORM_FLOAT, 1);
	float windArr[2] = { windDir.x * windSpeed, windDir.y * windSpeed };
	int locWindNoise = rlGetLocationUniform(noiseShader, "wind");
	int locNoiseScale = rlGetLocationUniform(noiseShader, "NOISE_SCALE");
	rlSetUniform(locNoiseScale, &baseNoiseFrequency, (int)RaylibWrapper::SHADER_UNIFORM_FLOAT, 1);
	float worleyScale = baseNoiseFrequency * detailNoiseMultiplier;
	int locWorleyScale = rlGetLocationUniform(noiseShader, "WORLEY_SCALE");
	rlSetUniform(locWorleyScale, &worleyScale, (int)RaylibWrapper::SHADER_UNIFORM_FLOAT, 1);
	rlSetUniform(locWindNoise, windArr, (int)RaylibWrapper::SHADER_UNIFORM_VEC2, 1);
	rlComputeShaderDispatch(16, 16, 16);  // local_size=4,4,4 -> 4*16=64
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	rlDisableShader();

	// Raymarch Pass

	rlEnableShader(raymarchShader);
	CheckGLError("After enable shader");
	glBindImageTexture(0, writeTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	CheckGLError("After bind image 0");
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, readTexture);

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_3D, noiseTexture);
	CheckGLError("After all bindings");

	// Set uniforms
	int locFirstFrame = rlGetLocationUniform(raymarchShader, "firstFrame");
	rlSetUniform(locFirstFrame, &firstFrame, (int)RaylibWrapper::SHADER_UNIFORM_INT, 1);
	int locTime = rlGetLocationUniform(raymarchShader, "time");
	rlSetUniform(locTime, &timeVal, (int)RaylibWrapper::SHADER_UNIFORM_FLOAT, 1);
	int locCoverage = rlGetLocationUniform(raymarchShader, "coverage");
	rlSetUniform(locCoverage, &coverage, (int)RaylibWrapper::SHADER_UNIFORM_FLOAT, 1);
	int locDensity = rlGetLocationUniform(raymarchShader, "density");
	rlSetUniform(locDensity, &density, (int)RaylibWrapper::SHADER_UNIFORM_FLOAT, 1);
	int locWind = rlGetLocationUniform(raymarchShader, "wind");
	rlSetUniform(locWind, windArr, (int)RaylibWrapper::SHADER_UNIFORM_VEC2, 1);
	int locCloudHeight = rlGetLocationUniform(raymarchShader, "cloudHeight");
	rlSetUniform(locCloudHeight, &cloudHeight, (int)RaylibWrapper::SHADER_UNIFORM_FLOAT, 1);
	int locCloudThickness = rlGetLocationUniform(raymarchShader, "cloudThickness");
	rlSetUniform(locCloudThickness, &cloudThickness, (int)RaylibWrapper::SHADER_UNIFORM_FLOAT, 1);
	int locRaymarchSteps = rlGetLocationUniform(raymarchShader, "raymarchSteps");
	rlSetUniform(locRaymarchSteps, &raymarchSteps, (int)RaylibWrapper::SHADER_UNIFORM_INT, 1);
	int locLightSteps = rlGetLocationUniform(raymarchShader, "lightSteps");
	rlSetUniform(locLightSteps, &lightSteps, (int)RaylibWrapper::SHADER_UNIFORM_INT, 1);
	int locLightMarchSize = rlGetLocationUniform(raymarchShader, "lightMarchSize");
	rlSetUniform(locLightMarchSize, &lightMarchSize, (int)RaylibWrapper::SHADER_UNIFORM_FLOAT, 1);
	int locAbsorption = rlGetLocationUniform(raymarchShader, "absorption");
	rlSetUniform(locAbsorption, &absorption, (int)RaylibWrapper::SHADER_UNIFORM_FLOAT, 1);
	int locScattering = rlGetLocationUniform(raymarchShader, "scattering");
	rlSetUniform(locScattering, &scattering, (int)RaylibWrapper::SHADER_UNIFORM_FLOAT, 1);
	int locPhaseG = rlGetLocationUniform(raymarchShader, "phaseG");
	rlSetUniform(locPhaseG, &phaseG, (int)RaylibWrapper::SHADER_UNIFORM_FLOAT, 1);
	float sunDirArr[3] = { sunDir.x, sunDir.y, sunDir.z };
	int locSunDir = rlGetLocationUniform(raymarchShader, "sunDir");
	rlSetUniform(locSunDir, sunDirArr, (int)RaylibWrapper::SHADER_UNIFORM_VEC3, 1);
	float sunColArr[3] = { sunColor.x, sunColor.y, sunColor.z };
	int locSunColor = rlGetLocationUniform(raymarchShader, "sunColor");
	rlSetUniform(locSunColor, sunColArr, (int)RaylibWrapper::SHADER_UNIFORM_VEC3, 1);
	int locSunIntensity = rlGetLocationUniform(raymarchShader, "sunIntensity");
	rlSetUniform(locSunIntensity, &sunIntensity, (int)RaylibWrapper::SHADER_UNIFORM_FLOAT, 1);
	int locAmbientLight = rlGetLocationUniform(raymarchShader, "ambientLight");
	rlSetUniform(locAmbientLight, &ambientLight, (int)RaylibWrapper::SHADER_UNIFORM_FLOAT, 1);
	bool highQualityBool = highQuality;
	int locHighQuality = rlGetLocationUniform(raymarchShader, "highQuality");
	rlSetUniform(locHighQuality, &highQualityBool, (int)RaylibWrapper::SHADER_UNIFORM_INT, 1);
	float camPosArr[3] = { cameraPos.x, cameraPos.y, cameraPos.z };
	int locCameraPos = rlGetLocationUniform(raymarchShader, "cameraPos");
	rlSetUniform(locCameraPos, camPosArr, (int)RaylibWrapper::SHADER_UNIFORM_VEC3, 1);
	int locInvViewProj = rlGetLocationUniform(raymarchShader, "invViewProj");
	glUniformMatrix4fv(locInvViewProj, 1, GL_FALSE, &invViewProj.m0);
	int locPrevViewProj = rlGetLocationUniform(raymarchShader, "prevViewProj");
	glUniformMatrix4fv(locPrevViewProj, 1, GL_FALSE, &prevViewProj.m0);
	float renderRes[2] = { (float)RaylibWrapper::GetScreenWidth(), (float)RaylibWrapper::GetScreenHeight() };
	int locRenderRes = rlGetLocationUniform(raymarchShader, "renderRes");
	rlSetUniform(locRenderRes, renderRes, (int)RaylibWrapper::SHADER_UNIFORM_VEC2, 1);
	int halfW, halfH;
	GetTexture2DSize(halfCloudTexture, &halfW, &halfH);
	float halfRes[2] = { (float)halfW, (float)halfH };
	int locHalfRes = rlGetLocationUniform(raymarchShader, "halfRes");
	rlSetUniform(locHalfRes, halfRes, (int)RaylibWrapper::SHADER_UNIFORM_VEC2, 1);

	// Dispatch
	int groupX = (halfW + 7) / 8;
	int groupY = (halfH + 7) / 8;
	rlComputeShaderDispatch(groupX, groupY, 1);
	CheckGLError("After dispatch 1");
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	rlDisableShader();
	CheckGLError("After dispatch 2");

	// Swap for next frame
	currentCloudTexture = 1 - currentCloudTexture;

	// Unbind all image textures
	glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(2, 0, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);

	//// Debug
	//float* pixels = new float[halfW * halfH * 4];
	//glBindTexture(GL_TEXTURE_2D, halfCloudTexture);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels);

	//// Check if any non - zero values
	//bool hasData = false;
	//for (int i = 0; i < halfW * halfH * 4; i++) {
	//	if (pixels[i] != 0.0f) {
	//		hasData = true;
	//		printf("Found data: pixels[%d] = %f\n", i, pixels[i]);
	//		break;
	//	}
	//}
	//printf("Texture has data: %s\n", hasData ? "YES" : "NO");
	//delete[] pixels;

	RaylibWrapper::rlSetBlendMode(RaylibWrapper::RL_BLEND_ALPHA);

	RaylibWrapper::Texture2D texture;
	texture.id = writeTexture;
	texture.width = halfW;
	texture.height = halfH;
	texture.mipmaps = 1;
	texture.format = RaylibWrapper::PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;

	int locCloudTex = RaylibWrapper::GetShaderLocation({ shader.first, shader.second }, "cloudTexture");
	RaylibWrapper::SetShaderValueTexture({ shader.first, shader.second }, locCloudTex, texture);

	RaylibWrapper::BeginShaderMode({ shader.first, shader.second });

	rlPushMatrix();
	rlLoadIdentity();

	rlBegin(RL_QUADS);
	rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-1.0f, 1.0f, 0.0f);
	rlTexCoord2f(0.0f, 1.0f); rlVertex3f(-1.0f, -1.0f, 0.0f);
	rlTexCoord2f(1.0f, 1.0f); rlVertex3f(1.0f, -1.0f, 0.0f);
	rlTexCoord2f(1.0f, 0.0f); rlVertex3f(1.0f, 1.0f, 0.0f);
	rlEnd();

	rlPopMatrix();
	//rlSetTexture(0);

	RaylibWrapper::EndShaderMode();

	RaylibWrapper::rlSetBlendMode(RaylibWrapper::RL_BLEND_ALPHA);

	prevView = view;
	prevProj = proj;
	firstFrame = false;
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
	baseNoiseFrequency = exposedVariables[1][2][2].get<float>();
	detailNoiseMultiplier = exposedVariables[1][3][2].get<float>();
	lightSteps = exposedVariables[1][4][2].get<int>();
	lightMarchSize = exposedVariables[1][5][2].get<float>();
	windSpeed = exposedVariables[1][6][2].get<float>();
	windDir = { exposedVariables[1][7][2][0].get<float>(), exposedVariables[1][7][2][1].get<float>() };
	cloudHeight = exposedVariables[1][8][2].get<float>();
	cloudThickness = exposedVariables[1][9][2].get<float>();
	raymarchSteps = exposedVariables[1][10][2].get<int>();
	absorption = exposedVariables[1][11][2].get<float>();
	scattering = exposedVariables[1][12][2].get<float>();
	phaseG = exposedVariables[1][13][2].get<float>();
	sunDir = { exposedVariables[1][14][2][0].get<float>(), exposedVariables[1][14][2][1].get<float>(), exposedVariables[1][14][2][2].get<float>() };
	sunColor = { exposedVariables[1][15][2][0].get<float>(), exposedVariables[1][15][2][1].get<float>(), exposedVariables[1][15][2][2].get<float>() };
	sunIntensity = exposedVariables[1][16][2].get<float>();
	ambientLight = exposedVariables[1][17][2].get<float>();
	highQuality = exposedVariables[1][18][2].get<bool>();
}
#endif

void Clouds::Destroy()
{
	auto it = std::find(clouds.begin(), clouds.end(), this);
	if (it != clouds.end())
		clouds.erase(it);
}