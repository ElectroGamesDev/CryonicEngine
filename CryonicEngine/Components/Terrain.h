#pragma once
#include "Component.h"
#include "../RaylibModelWrapper.h"
#include "../Material.h"
#include "../Sprite.h"
#include "../RaylibWrapper.h"
#include <vector>
#include <string>
#include <cmath>

class Terrain : public Component
{
public:
	Terrain(GameObject* obj, int id) : Component(obj, id)
	{
		runInEditor = true;
		name = "Terrain";
		iconUnicode = "\xef\x81\x8b"; // example icon

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
                ["Material","terrainMaterial","nullptr","Terrain Material",{"Extensions":[".mat"]}],
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
#if defined(EDITOR)
	void EditorUpdate() override;
#endif
	void Destroy() override;

	// ----- Height manipulation -----
	void SetHeight(int x, int z, float height);
	float GetHeight(int x, int z) const;
	float GetHeightAtWorldPosition(float worldX, float worldZ) const;
	Vector3 GetNormalAtWorldPosition(float worldX, float worldZ) const;

	// ----- Sculpting -----
	void RaiseTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime);
	void LowerTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime);
	void SmoothTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime);
	void FlattenTerrain(float worldX, float worldZ, float radius, float targetHeight, float strength, float deltaTime);

	// ----- Texture Painting -----
	void PaintTexture(float worldX, float worldZ, float radius, int layerIndex, float strength, float deltaTime);
	void AddTerrainLayer(Material* material, const std::string& name = "Layer");
	void RemoveTerrainLayer(int layerIndex);
	int GetLayerCount() const { return static_cast<int>(terrainLayers.size()); }

private:
	// internal generation
	void GenerateTerrain();
	void GenerateFromHeightmap();
	void RebuildMesh();

private:
	bool setupEditor = true;

	int terrainWidth = 256;
	int terrainDepth = 256;
	float terrainHeight = 50.0f;
	float heightScale = 1.0f;
	Sprite* heightmapSprite = nullptr;
	Material* terrainMaterial = nullptr;
	bool enableLOD = true;
	int lodLevels = 4;
	float lodDistance = 100.0f;
	bool castShadows = true;

	std::vector<std::vector<float>> heightData;
	std::vector<Material*> terrainLayers;

	RaylibModel raylibModel;
	bool modelGenerated = false;
};
