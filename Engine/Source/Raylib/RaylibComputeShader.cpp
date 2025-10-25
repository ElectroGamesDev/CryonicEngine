#include "RaylibComputeShader.h"
#include "ThirdParty/Raylib/include/raylib.h"
#include "ThirdParty/Raylib/include/raymath.h"
#include "ThirdParty/Raylib/include/rlgl.h"

std::unordered_map<ShaderManager::ComputeShaders, RaylibComputeShader> RaylibComputeShader::shaders;

void RaylibComputeShader::Load(const char* path)
{
	char* shaderFile = LoadFileText(path);
    unsigned int compiledShader = rlCompileShader(shaderFile, RL_COMPUTE_SHADER);
	shader = rlLoadComputeShaderProgram(compiledShader);
	UnloadFileText(shaderFile);
}

//void RaylibComputeShader::Update(float cameraPosX, float cameraPosY, float cameraPosZ)
//{
//	float cameraPos[3] = {
//		cameraPosX,
//		cameraPosY,
//		cameraPosZ
//	};
//	SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
//}

void RaylibComputeShader::Unload()
{
    rlUnloadShaderProgram(shader);
}

unsigned int RaylibComputeShader::GetShader()
{
    return shader;
}