#include "RaylibCameraWrapper.h"
#include "ThirdParty/raylib/include/raylib.h"

RaylibCamera::RaylibCamera()
{
	camera = new Camera3D{ 0 };
}

RaylibCamera::~RaylibCamera()
{
	delete camera;
}

void RaylibCamera::SetFOVY(int fov)
{
	camera->fovy = fov;
}

float RaylibCamera::GetFOVY()
{
	return camera->fovy;
}

void RaylibCamera::SetUpY(int amount)
{
	camera->up = {0, (float)amount, 0};
}

void RaylibCamera::SetUp(float x, float y, float z)
{
	camera->up = {x, y, z};
}

void RaylibCamera::SetPosition(int x, int y, int z)
{
	camera->position.x = x;
	camera->position.y = y;
	camera->position.z = z;
}

void RaylibCamera::SetPositionX(int x)
{
	camera->position.x = x;
}

void RaylibCamera::SetPositionY(int y)
{
	camera->position.y = y;
}

void RaylibCamera::SetPositionZ(int z)
{
	camera->position.z = z;
}

std::array<float, 3> RaylibCamera::GetPosition()
{
	return { camera->position.x, camera->position.y, camera->position.z };
}

void RaylibCamera::SetTarget(float x, float y, float z)
{
	camera->target.x = x;
	camera->target.y = y;
	camera->target.z = z;
}

void RaylibCamera::SetProjection(int projection)
{
	camera->projection = projection;
}

int RaylibCamera::GetProjection()
{
	return camera->projection;
}

void RaylibCamera::BeginMode3D()
{
	RaylibWrapper::BeginMode3D({ { camera->position.x, camera->position.y, camera->position.z }, { camera->target.x, camera->target.y, camera->target.z }, { camera->up.x, camera->up.y, camera->up.z }, camera->fovy, camera->projection });
}

RaylibWrapper::Matrix RaylibCamera::GetCameraMatrix()
{
	return RaylibWrapper::GetCameraMatrix({ { camera->position.x, camera->position.y, camera->position.z }, { camera->target.x, camera->target.y, camera->target.z }, { camera->up.x, camera->up.y, camera->up.z }, camera->fovy, camera->projection });
}

std::array<float, 2> RaylibCamera::GetWorldToScreen(float x, float y, float z)
{
	Vector2 pos = ::GetWorldToScreen({x, y, z}, *camera);
	return {pos.x, pos.y};
}