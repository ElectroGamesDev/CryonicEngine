#include "ShaderManager.h"
//#include "Components/CameraComponent.h"
#include <filesystem>
#include "Raylib/RaylibShaderWrapper.h"
#include "Raylib/RaylibComputeShader.h"
#include "Raylib/RaylibWrapper.h"
#ifndef EDITOR
#include "Game.h"
#endif

#if defined(PLATFORM_DESKTOP)  // I had to remove PLATFORM_DESKTOP from predefines to stop crash so this code is useless and GLSL_VERSION is always 100 right now.
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

void ShaderManager::Cleanup()
{
    for (auto it = RaylibShader::shaders.begin(); it != RaylibShader::shaders.end(); ++it)
        it->second.Unload();

    RaylibShader::shaders.clear();

	for (auto it = RaylibComputeShader::shaders.begin(); it != RaylibComputeShader::shaders.end(); ++it)
		it->second.Unload();

	RaylibComputeShader::shaders.clear();
}

void ShaderManager::Init()
{
	auto loadShader = [&](ShaderManager::Shaders type, const std::string& vs, const std::string& fs)
		{
#if defined(EDITOR)
			RaylibShader::shaders[type].Load(vs.c_str(), fs.c_str());
#else
			std::filesystem::path basePath = exeParent.empty() ? "" : std::filesystem::path(exeParent);
			RaylibShader::shaders[type].Load(
				(basePath / vs).string().c_str(),
				(basePath / fs).string().c_str()
			);
#endif
		};

	auto loadComputeShader = [&](ShaderManager::ComputeShaders type, const std::string& comp)
		{
#if defined(EDITOR)
			RaylibComputeShader::shaders[type].Load(comp.c_str());
#else
			std::filesystem::path basePath = exeParent.empty() ? "" : std::filesystem::path(exeParent);
			RaylibComputeShader::shaders[type].Load((basePath / comp).string().c_str());
#endif
		};

	// Shader Loading

	loadShader(ShaderManager::LitStandard,
		"resources/Shaders/glsl330/lighting.vs",
		"resources/Shaders/glsl330/lighting.fs");

	loadShader(ShaderManager::Terrain,
		"Resources/shaders/glsl330/terrain.vs",
		"Resources/shaders/glsl330/terrain.fs");

	loadShader(ShaderManager::Cubemap,
		"Resources/shaders/glsl330/cubemap.vs",
		"Resources/shaders/glsl330/cubemap.fs");

	loadShader(ShaderManager::Skybox,
		"Resources/shaders/glsl330/skybox.vs",
		"Resources/shaders/glsl330/skybox.fs");

	loadShader(ShaderManager::Water,
		"Resources/shaders/glsl330/water.vs",
		"Resources/shaders/glsl330/water.fs");

	loadShader(ShaderManager::Clouds,
		"Resources/shaders/glsl440/clouds.vs",
		"Resources/shaders/glsl440/clouds.fs");

	// Compute Shaders

	loadComputeShader(ShaderManager::CloudsRaymarch, "Resources/shaders/glsl440/clouds_raymarch.comp");
	loadComputeShader(ShaderManager::CloudsNoise, "Resources/shaders/glsl440/clouds_noise.comp");
}

void ShaderManager::UpdateShaders(float cameraPosX, float cameraPosY, float cameraPosZ)
{
    for (auto it = RaylibShader::shaders.begin(); it != RaylibShader::shaders.end(); ++it)
        it->second.Update(cameraPosX, cameraPosY, cameraPosZ);
}

std::pair<unsigned int, int*> ShaderManager::GetShader(Shaders shader)
{
    return RaylibShader::shaders[shader].GetShader();
}

unsigned int ShaderManager::GetComputeShader(ComputeShaders shader)
{
	return RaylibComputeShader::shaders[shader].GetShader();
}
