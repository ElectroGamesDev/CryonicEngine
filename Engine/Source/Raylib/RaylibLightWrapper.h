#pragma once
#include "Systems/Rendering/ShaderManager.h"

class RaylibLight
{
public:
	// 0 = Directional, 1 = Point
	void Create(int type, float posX, float posY, float posZ, float targetX, float targetY, float targetZ, unsigned char colorR, unsigned char colorG, unsigned char colorB, unsigned char colorA, ShaderManager::Shaders shader);
	void SetPosition(float x, float y, float z);
	void Update();
};