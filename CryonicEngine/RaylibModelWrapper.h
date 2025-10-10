#pragma once
#include "ShaderManager.h"
#include <filesystem>
#include "RaylibWrapper.h"

class Model;

enum ModelType
{
	Custom,
	Cube,
	Sphere,
	Plane,
	Cylinder,
	Cone
};

class RaylibModel
{
public:
	bool Create(ModelType type, std::filesystem::path path, ShaderManager::Shaders shader, std::filesystem::path projectPath);
	void CreateTerrain(int vertexCount, int triangleCount, unsigned int vaoId, std::vector<unsigned int> vboIds);
	void Unload();
	void DeleteInstance();
	void DrawModelWrapper(float posX, float posY, float posZ, float sizeX, float sizeY, float sizeZ, float rotationX, float rotationY, float rotationZ, float rotationW, unsigned char colorR, unsigned char colorG, unsigned char colorB, unsigned char colorA);
	void SetMaterialMap(int materialIndex, int mapIndex, RaylibWrapper::Texture2D texture, RaylibWrapper::Color color, float intesity);
	void SetMaterials(std::vector<RaylibWrapper::Material*> mats);
	void SetEmbeddedMaterials();
	void SetMaterialsToEmbedded();
	std::vector<int> GetMaterialIDs();
	bool CompareMaterials(std::vector<int> matIDs);
	int GetMaterialCount();
	int GetMaterialID(int index);
	//RaylibWrapper::Material GetMaterial(int index);
	void SetShaderValue(int materialIndex, int locIndex, const void* value, int uniformType);
	int GetShaderLocation(int materialIndex, std::string uniformName);
	void SetShader(int materialIndex, ShaderManager::Shaders shader);
	static void SetShadowShader(unsigned int id, int* locs);
	static void SetMaterialPreviewShader(unsigned int id, int* locs);
	bool IsPrimitive();
	int GetMeshCount();
	int GetTriangleCount(int meshIndex);

private:
	std::pair<Model, int>* model = nullptr;
	bool primitiveModel = false;
	bool terrainModel = false;
	std::filesystem::path path;
	// This was removed because it added overhead for something that has an easier solution. It will only need to be re-added if GetMaterial() is needed
	//static std::unordered_map<Model, std::vector<RaylibWrapper::Material>> rWrapperMaterials; // Model can not be used as a key. A pointer could be used though.
	ShaderManager::Shaders modelShader;
	static std::pair<unsigned int, int*> shadowShader;
	static std::pair<unsigned int, int*> materialPreviewShadowShader;
};
