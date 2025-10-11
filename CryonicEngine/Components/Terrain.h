#pragma once
#include "Component.h"
#include "../RaylibModelWrapper.h"
#include "../Material.h"
#include "../Sprite.h"
#include "../RaylibWrapper.h"
#include <vector>
#include <string>
#include <cmath>\
#include "json.hpp"

struct TerrainLayer
{
	Material* material = nullptr;
	std::string name = "Layer";
	std::vector<std::vector<float>> splatmap; // Per-vertex weight (0-1)
};

class Terrain : public Component
{
public:
	Terrain(GameObject* obj, int id) : Component(obj, id)
	{
		runInEditor = true;
		name = "Terrain";
		iconUnicode = "\xef\x81\x8b";

#if defined(EDITOR)
		std::string variables = R"(
        [
            0,
            [
                ["int","terrainWidth",256,"Terrain Width"],
                ["int","terrainDepth",256,"Terrain Depth"],
                ["float","terrainHeight",50.0,"Max Height"],
                ["float","heightScale",1.0,"Height Scale"],
                ["Sprite","heightmapSprite","nullptr","Heightmap Texture",{"Extensions":[".png",".jpg",".jpeg"]}],
                ["bool","enableLOD",true,"Enable LOD"],
                ["int","lodLevels",4,"LOD Levels"],
                ["float","lodDistance",100.0,"LOD Distance"],
                ["bool","castShadows",true,"Cast Shadows"]
            ]
        ]
        )";
		exposedVariables = nlohmann::json::parse(variables);
#endif
	}

	Terrain* Clone() override { return new Terrain(gameObject, -1); }

	void Awake() override;
	void Start() override {}
	void Render(bool renderShadows) override;
	void Update() override;
#if defined(EDITOR)
	void EditorUpdate() override;
#endif
	void Destroy() override;

	// Todo: Add setter functions, and make sure to regenerate the terrain if any of these values change
	bool RaycastToTerrain(const RaylibWrapper::Ray& ray, Vector3& hitPos);
	int WorldToHeightmapX(float worldX) const;
	int WorldToHeightmapZ(float worldZ) const;

	int GetWidth() const { return terrainWidth; }
	int GetDepth() const { return terrainDepth; }
	float GetTerrainHeight() const { return terrainHeight; }
	float GetHeightScale() const { return heightScale; }
	Sprite* GetHeightmap() const { return heightmapSprite; }
	Material* GetMaterial() const { return terrainMaterial; }
	bool IsLODEnabled() const { return enableLOD; }
	int GetLODLevels() const { return lodLevels; }
	float GetLODDistance() const { return lodDistance; }
	bool IsCastingShadows() const { return castShadows; }

	// Height manipulation
	void SetHeight(int x, int z, float height);
	float GetHeight(int x, int z) const;
	float GetHeightAtWorldPosition(float worldX, float worldZ) const;
	Vector3 GetNormalAtWorldPosition(float worldX, float worldZ) const;

	// Sculpting
	void RaiseTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime);
	void LowerTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime);
	void SmoothTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime);
	void FlattenTerrain(float worldX, float worldZ, float radius, float strength, float targetHeight, float deltaTime);

	// Texture Painting
	void PaintTexture(float worldX, float worldZ, float radius, float strength, int layerIndex, float deltaTime);
	void AddTerrainLayer(const std::string& materialPath, const std::string& name = "Layer");
	void RemoveTerrainLayer(int layerIndex);
	int GetLayerCount() const { return static_cast<int>(terrainLayers.size()); }
	TerrainLayer* GetLayer(int index);
	const TerrainLayer* GetLayer(int index) const;

	// Terrain serialization, saving, and loading
	nlohmann::json SerializeHeightData();
	void LoadHeightData(const nlohmann::json& data);
	nlohmann::json SerializeLayerData();
	void LoadLayerData(const nlohmann::json& data);

private:
	// internal generation
	void GenerateTerrain();
	void GenerateFromHeightmap();
	void RebuildMesh();
	void UpdateSplatmapTexture();
	void InitializeSplatmaps();
	void NormalizeSplatmaps(int x, int z);
	void LoadTerrainShader();
	void UpdateShaderTextures();
	void InitializeMaterial();

private:
	bool setupEditor = true;

	bool checkerboardTextureLoaded = false;
	RaylibWrapper::Texture2D checkerboardTexture;

	int terrainWidth = 256;
	int terrainDepth = 256;
	float terrainHeight = 100.0f;
	float heightScale = 1.0f;
	Sprite* heightmapSprite = nullptr;
	Material* terrainMaterial = nullptr;
	bool enableLOD = true;
	int lodLevels = 4;
	float lodDistance = 100.0f;
	bool castShadows = true;

	int splatmapLoc = -1;
	int texture1Loc = -1;
	int texture2Loc = -1;
	int texture3Loc = -1;
	int textureScaleLoc = -1;
	std::pair<unsigned int, int*> terrainShader;

	bool needsRebuild = false;
	float lastRebuild = 0.0f; // Seconds since the last rebuild
	bool needsSplatmapUpdate = false;

	std::vector<std::vector<float>> heightData;
	std::vector<TerrainLayer> terrainLayers;

	RaylibModel raylibModel;
	bool modelGenerated = false;

	// Splatmap texture for GPU
	RaylibWrapper::Texture2D splatmapTexture = { 0 };
	bool splatmapGenerated = false;
};