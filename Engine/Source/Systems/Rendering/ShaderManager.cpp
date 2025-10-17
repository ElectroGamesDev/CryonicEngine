#include "ShaderManager.h"
//#include "Components/CameraComponent.h"
#include <filesystem>
#include "Raylib/RaylibShaderWrapper.h"
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
    {
        it->second.Unload();
    }
    RaylibShader::shaders.clear();
}

void ShaderManager::Init()
{
    //shaders[ShaderManager::LitStandard] = LoadShader(("resources/shaders/glsl" + std::to_string(GLSL_VERSION) + "/lighting.vs").c_str(), ("resources/shaders/glsl" + std::to_string(GLSL_VERSION) + "/lighting.fs").c_str());

    // Todo: Replace glsl330 to GLSL_VERSION. I will need to define the platform though
#if defined (EDITOR)
    RaylibShader::shaders[ShaderManager::LitStandard].Load("resources/Shaders/glsl330/lighting.vs", "resources/Shaders/glsl330/lighting.fs");
#else
    if (exeParent.empty())
    {
        RaylibShader::shaders[ShaderManager::LitStandard].Load("resources/Shaders/glsl330/lighting.vs", "resources/Shaders/glsl330/lighting.fs");
    }
    else
    {
        RaylibShader::shaders[ShaderManager::LitStandard].Load((std::filesystem::path(exeParent) / "resources/Shaders/glsl330/lighting.vs").string().c_str(), (std::filesystem::path(exeParent) / "resources/Shaders/glsl330/lighting.fs").string().c_str());
    }
#endif

    // Terrain
#if defined (EDITOR)
	RaylibShader::shaders[ShaderManager::Terrain].Load("Resources/shaders/glsl330/terrain.vs", "resources/shaders/glsl330/terrain.fs");
#else
	if (exeParent.empty())
	{
		RaylibShader::shaders[ShaderManager::Terrain].Load("Resources/shaders/glsl330/terrain.vs", "Resources/shaders/glsl330/terrain.fs");
	}
	else
	{
		RaylibShader::shaders[ShaderManager::Terrain].Load((std::filesystem::path(exeParent) / "Resources/shaders/glsl330/terrain.vs").string().c_str(), (std::filesystem::path(exeParent) / "Resources/shaders/glsl330/terrain.fs").string().c_str());
	}
#endif

	// Cubemap
#if defined (EDITOR)
	RaylibShader::shaders[ShaderManager::Cubemap].Load("Resources/shaders/glsl330/cubemap.vs", "resources/shaders/glsl330/cubemap.fs");
#else
	if (exeParent.empty())
	{
		RaylibShader::shaders[ShaderManager::Cubemap].Load("Resources/shaders/glsl330/cubemap.vs", "Resources/shaders/glsl330/cubemap.fs");
	}
	else
	{
		RaylibShader::shaders[ShaderManager::Cubemap].Load((std::filesystem::path(exeParent) / "Resources/shaders/glsl330/cubemap.vs").string().c_str(), (std::filesystem::path(exeParent) / "Resources/shaders/glsl330/cubemap.fs").string().c_str());
	}
#endif

	// Skybox
#if defined (EDITOR)
	RaylibShader::shaders[ShaderManager::Skybox].Load("Resources/shaders/glsl330/skybox.vs", "resources/shaders/glsl330/skybox.fs");
#else
	if (exeParent.empty())
	{
		RaylibShader::shaders[ShaderManager::Skybox].Load("Resources/shaders/glsl330/skybox.vs", "Resources/shaders/glsl330/skybox.fs");
	}
	else
	{
		RaylibShader::shaders[ShaderManager::Skybox].Load((std::filesystem::path(exeParent) / "Resources/shaders/glsl330/skybox.vs").string().c_str(), (std::filesystem::path(exeParent) / "Resources/shaders/glsl330/skybox.fs").string().c_str());
	}
#endif

	// Water
#if defined (EDITOR)
	RaylibShader::shaders[ShaderManager::Water].Load("Resources/shaders/glsl330/water.vs", "resources/shaders/glsl330/water.fs");
#else
	if (exeParent.empty())
	{
		RaylibShader::shaders[ShaderManager::Water].Load("Resources/shaders/glsl330/water.vs", "Resources/shaders/glsl330/water.fs");
	}
	else
	{
		RaylibShader::shaders[ShaderManager::Water].Load((std::filesystem::path(exeParent) / "Resources/shaders/glsl330/water.vs").string().c_str(), (std::filesystem::path(exeParent) / "Resources/shaders/glsl330/water.fs").string().c_str());
	}
#endif

	// Clouds
#if defined (EDITOR)
	RaylibShader::shaders[ShaderManager::Clouds].Load("Resources/shaders/glsl330/clouds.vs", "resources/shaders/glsl330/clouds.fs");
#else
	if (exeParent.empty())
		RaylibShader::shaders[ShaderManager::Clouds].Load("Resources/shaders/glsl330/clouds.vs", "Resources/shaders/glsl330/clouds.fs");
	else
		RaylibShader::shaders[ShaderManager::Clouds].Load((std::filesystem::path(exeParent) / "Resources/shaders/glsl330/clouds.vs").string().c_str(), (std::filesystem::path(exeParent) / "Resources/shaders/glsl330/clouds.fs").string().c_str());
#endif


    //std::string currentDirectory = GetWorkingDirectory();
    //std::string relativePath = "resources/shaders/glsl330/lighting.vs";

    //// Combine the current directory and relative path
    //std::string fullPath = currentDirectory + "/" + relativePath;

    //// Output the full path
    //ConsoleLogger::InfoLog("Full path: " + fullPath);
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