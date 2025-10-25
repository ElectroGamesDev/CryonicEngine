#pragma once
#include "Systems/Rendering/ShaderManager.h"
#include "ThirdParty/Raylib/include/raylib.h"
#include "ThirdParty/Raylib/include/raymath.h"
#include "ThirdParty/Raylib/include/rlgl.h"

class RaylibComputeShader
{
public:
	void Load(const char* path);
	//void Update(float cameraPosX, float cameraPosY, float cameraPosZ);
	void Unload();
	unsigned int GetShader();

	unsigned int shader;
	static std::unordered_map<ShaderManager::ComputeShaders, RaylibComputeShader> shaders;
};