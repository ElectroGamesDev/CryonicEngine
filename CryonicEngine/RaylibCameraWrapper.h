#pragma once

#include <array>
#include "RaylibWrapper.h"

class RaylibCamera
{
public:
	void SetFOVY(int y);
	float GetFOVY();
	void SetUpY(int amount);
	void SetPosition(int x, int y, int z);
	void SetPositionX(int x);
	void SetPositionY(int y);
	void SetPositionZ(int z);
	std::array<float, 3> GetPosition();
	void SetTarget(float x, float y, float z);
	// 0 = Perspective, 1 = Orthographic
	void SetProjection(int projection);
	int GetProjection();
	void BeginMode3D();
	RaylibWrapper::Matrix GetCameraMatrix();
	std::array<float, 2> GetWorldToScreen(float x, float y, float z);
};