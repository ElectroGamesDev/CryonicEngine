#pragma once

#include "Component.h"
#include "../RaylibModelWrapper.h"
#include "../Material.h"
#include "../Sprite.h"
#include <vector>
#include <array>

// Forward declarations
struct TerrainLayer;
struct TerrainLOD;

class Terrain : public Component
{
public:
	Terrain(GameObject* obj, int id) : Component(obj, id)
	{
		runInEditor = true;
		name = "Terrain";
		iconUnicode = "\xef\x84\xb8";
#if defined(EDITOR)
		std::string variables = R"(
        [
            0,
            [
                [
                    "int",
                    "terrainWidth",
                    256,
                    "Terrain Width"
                ],
                [
                    "int",
                    "terrainDepth",
                    256,
                    "Terrain Depth"
                ],
                [
                    "float",
                    "terrainHeight",
                    50.0,
                    "Max Height"
                ],
                [
                    "float",
                    "heightScale",
                    1.0,
                    "Height Scale"
                ],
                [
                    "Sprite",
                    "heightmapSprite",
                    "nullptr",
                    "Heightmap Texture",
                    {
                        "Extensions": [".png", ".jpg", ".jpeg"]
                    }
                ],
                [
                    "Material",
                    "terrainMaterial",
                    "nullptr",
                    "Terrain Material",
                    {
                        "Extensions": [".mat"]
                    }
                ],
                [
                    "bool",
                    "enableLOD",
                    true,
                    "Enable LOD"
                ],
                [
                    "int",
                    "lodLevels",
                    4,
                    "LOD Levels"
                ],
                [
                    "float",
                    "lodDistance",
                    100.0,
                    "LOD Distance"
                ],
                [
                    "bool",
                    "castShadows",
                    true,
                    "Cast Shadows"
                ]
            ]
        ]
    )";
		exposedVariables = nlohmann::json::parse(variables);
#endif
	}

	Terrain* Clone() override
	{
		return new Terrain(gameObject, -1);
	}

	// Component lifecycle
	void Awake() override;
	void Start() override;
	void Update() override;
	void Render(bool renderShadows) override;
#if defined(EDITOR)
	void EditorUpdate() override;
#endif
	void Destroy() override;

	// Terrain generation
	void GenerateTerrain();
	void LoadHeightmap(const std::filesystem::path& path);
	void GenerateFlatTerrain();

	// Height manipulation
	void SetHeight(int x, int z, float height);
	float GetHeight(int x, int z) const;
	float GetHeightAtWorldPosition(float worldX, float worldZ) const;
	Vector3 GetNormalAtWorldPosition(float worldX, float worldZ) const;

	// Sculpting functions
	void RaiseTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime);
	void LowerTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime);
	void SmoothTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime);
	void FlattenTerrain(float worldX, float worldZ, float radius, float targetHeight, float strength, float deltaTime);

	// Texture painting
	void PaintTexture(float worldX, float worldZ, float radius, int layerIndex, float strength, float deltaTime);
	void AddTerrainLayer(Material* material, const std::string& name = "Layer");
	void RemoveTerrainLayer(int layerIndex);
	int GetLayerCount() const { return static_cast<int>(terrainLayers.size()); }

	// Material and rendering
	void SetMaterial(Material* mat);
	Material* GetMaterial() { return terrainMaterial; }
	void UpdateTerrainMesh();

	// LOD system
	void UpdateLOD(const Vector3& cameraPosition);
	void GenerateLODMeshes();

	// Utility
	void RegenerateTerrain();
	Vector3 GetTerrainSize() const;
	bool IsValidTerrainPosition(int x, int z) const;

private:
	// Terrain data
	std::vector<float> heightData;
	std::vector<Vector3> normals;
	std::vector<std::vector<float>> splatMaps; // Texture blend weights for each layer
	std::vector<TerrainLayer> terrainLayers;

	// Terrain properties
	int terrainWidth = 256;
	int terrainDepth = 256;
	float terrainHeight = 50.0f;
	float heightScale = 1.0f;
	Sprite* heightmapSprite;

	// Rendering
	RaylibModel terrainModel;
	Material* terrainMaterial = nullptr;
	bool terrainGenerated = false;
	bool castShadows = true;
	bool defaultMaterial = false;

	// LOD system
	bool enableLOD = true;
	int lodLevels = 4;
	float lodDistance = 100.0f;
	std::vector<TerrainLOD> lodMeshes;
	int currentLODLevel = 0;

	// Mesh generation helpers
	void GenerateMeshData(std::vector<float>& vertices, std::vector<unsigned short>& indices,
		std::vector<float>& texcoords, std::vector<float>& normals, int lodLevel = 0);
	void CalculateNormals();
	Vector3 CalculateNormalAt(int x, int z) const;
	void ApplyHeightmapToTerrain(const std::filesystem::path& path);

	// Sculpting helpers
	float GetBrushWeight(float distance, float radius) const;
	void WorldToTerrainCoords(float worldX, float worldZ, int& terrainX, int& terrainZ) const;

	// Texture painting helpers
	void NormalizeSplatWeights(int x, int z);
	void UpdateSplatMap();

	// LOD helpers
	int CalculateLODLevel(float distance) const;
	void SwitchLODMesh(int lodLevel);
};

// Terrain layer structure for texture painting
struct TerrainLayer
{
	Material* material = nullptr;
	std::string name;
	float tileSize = 1.0f;
	Vector2 offset = { 0.0f, 0.0f };
};

// LOD mesh data
struct TerrainLOD
{
	RaylibModel model;
	int resolution; // Vertices per side (reduced from original)
	float maxDistance;
};