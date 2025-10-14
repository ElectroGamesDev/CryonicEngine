#pragma once

#include <unordered_map>
#include <string>

class ShaderManager
{
public:
	enum Shaders
	{
		None,
		LitStandard,
		Terrain,
		Cubemap,
		Skybox,
		Water,
		Clouds
	};

	static void Init();
	static void Cleanup();
	static void UpdateShaders(float cameraPosX, float cameraPosY, float cameraPosZ);
	static std::pair<unsigned int, int*> GetShader(Shaders shader); // id, locs
};