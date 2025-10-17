#include "RaylibShaderWrapper.h"
#include "ThirdParty/Raylib/include/raylib.h"
#include "ThirdParty/Raylib/include/raymath.h"
#include "ThirdParty/Raylib/include/rlgl.h"

std::unordered_map<ShaderManager::Shaders, RaylibShader> RaylibShader::shaders;

void RaylibShader::Load(const char* vertexPath, const char* fragmentPath)
{
	shader = LoadShader(vertexPath, fragmentPath);

    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    int ambientLoc = GetShaderLocation(shader, "ambient");
    float ambientValues[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    SetShaderValue(shader, ambientLoc, ambientValues, SHADER_UNIFORM_VEC4);
}

void RaylibShader::Update(float cameraPosX, float cameraPosY, float cameraPosZ)
{
    float cameraPos[3] = {
        cameraPosX,
        cameraPosY,
        cameraPosZ
    };
    SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
}

void RaylibShader::Unload()
{
	UnloadShader(shader);
}

std::pair<unsigned int, int*> RaylibShader::GetShader()
{
    return {shader.id, shader.locs};
}