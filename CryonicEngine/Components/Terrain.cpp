#include "Terrain.h"
#include "../RaylibWrapper.h"
#include "rlgl.h"
#include <cmath>
#include <algorithm>
#include "CameraComponent.h"
#if defined (EDITOR)
#include "../ProjectManager.h"
#include "../Editor.h"
#else
#include "../Game.h"
#endif

void Terrain::Awake()
{
	// Initialize splat maps for texture painting
	splatMaps.resize(1); // Start with 1 base layer
	splatMaps[0].resize(terrainWidth * terrainDepth, 1.0f); // Base layer fully visible
}

void Terrain::Start()
{
	if (!terrainGenerated)
		GenerateTerrain();
}

void Terrain::Update()
{
	// Todo: Optimize by only updating when camera moves a certain distance
	
	// Update LOD based on camera position if enabled
	if (enableLOD && terrainGenerated)
	{
		CameraComponent* mainCamera = CameraComponent::main;
		if (mainCamera)
		{
			Vector3 cameraPos = mainCamera->gameObject->transform.GetPosition();
			UpdateLOD(cameraPos);
		}
	}
}

void Terrain::Render(bool renderShadows)
{
	if (!terrainGenerated || (renderShadows && !castShadows))
		return;

	Vector3 position = gameObject->transform.GetPosition();
	Quaternion rotation = gameObject->transform.GetRotation();
	Vector3 scale = gameObject->transform.GetScale();

	// Debug scale values
	if (scale.x < 0.01f || scale.y < 0.01f || scale.z < 0.01f) {
		ConsoleLogger::ErrorLog("Terrain scale too small: (" +
			std::to_string(scale.x) + ", " + std::to_string(scale.y) + ", " + std::to_string(scale.z) + ")");
		scale = { 1.0f, 1.0f, 1.0f }; // Default to unit scale if too small
	}

	RaylibModel* modelToRender = &terrainModel;
	if (enableLOD && currentLODLevel > 0 && currentLODLevel <= lodMeshes.size()) {
		modelToRender = &lodMeshes[currentLODLevel - 1].model;
	}

	if ((!terrainMaterial && modelToRender->GetMaterialID(0) != 0) ||
		(terrainMaterial && !modelToRender->CompareMaterials({ terrainMaterial->GetID() }))) {
		if (!terrainMaterial)
			modelToRender->SetMaterials({ &Material::defaultMaterial });
		else
			modelToRender->SetMaterials({ terrainMaterial->GetRaylibMaterial() });
	}

	ConsoleLogger::InfoLog("Rendering LOD level: " + std::to_string(currentLODLevel));
	ConsoleLogger::InfoLog("Model mesh count: " + std::to_string(modelToRender->GetMeshCount()));
	if (modelToRender->GetMeshCount() > 0) {
		ConsoleLogger::InfoLog("Triangle count: " + std::to_string(modelToRender->GetTriangleCount(0)));
	}

	modelToRender->DrawModelWrapper(position.x, position.y, position.z,
		scale.x, scale.y, scale.z,
		rotation.x, rotation.y, rotation.z, rotation.w,
		255, 255, 255, 255);
}

#if defined(EDITOR)
void Terrain::EditorUpdate()
{
	bool needsRegeneration = false;

	if (editorSetup)
	{
		needsRegeneration = true;
		editorSetup = false;
	}

	// Update LOD based on camera position if enabled
	// Todo: Optimize by only updating when camera moves a certain distance and only run this code if the viewport or main camera is visible
	if (enableLOD && terrainGenerated)
	{
		RaylibWrapper::Camera& mainCamera = Editor::camera;
		UpdateLOD({ mainCamera.position.x, mainCamera.position.y, mainCamera.position.z });
	}

	// Check if terrain dimensions changed
	if (exposedVariables[1][0][2].get<int>() != terrainWidth)
	{
		terrainWidth = exposedVariables[1][0][2].get<int>();
		needsRegeneration = true;
	}
	if (exposedVariables[1][1][2].get<int>() != terrainDepth)
	{
		terrainDepth = exposedVariables[1][1][2].get<int>();
		needsRegeneration = true;
	}

	// Update other properties
	terrainHeight = exposedVariables[1][2][2].get<float>();
	heightScale = exposedVariables[1][3][2].get<float>();

	// Check heightmap changes
	std::string newHeightmapPath = exposedVariables[1][4][2].get<std::string>();
	if ((!heightmapSprite && newHeightmapPath != "nullptr" && !newHeightmapPath.empty()) || (heightmapSprite && newHeightmapPath != heightmapSprite->GetRelativePath()))
	{
		if (heightmapSprite != nullptr)
			delete heightmapSprite;

		if (newHeightmapPath == "nullptr" || newHeightmapPath.empty())
			heightmapSprite = nullptr;
		else
			heightmapSprite = new Sprite(exposedVariables[1][4][2].get<std::string>());

		ApplyHeightmapToTerrain(heightmapSprite);
		UpdateTerrainMesh();
	}

	// Update material
	if (exposedVariables[1][5][2] == "nullptr")
		exposedVariables[1][5][2] = "Default";

	if ((!terrainMaterial && !(exposedVariables[1][5][2] == "Default" && defaultMaterial)) ||
		(terrainMaterial && terrainMaterial->GetPath() != exposedVariables[1][5][2]))
	{
		if (exposedVariables[1][5][2] == "Default")
		{
			defaultMaterial = true;
			SetMaterial(nullptr);
		}
		else
			SetMaterial(Material::GetMaterial(exposedVariables[1][5][2]));
	}

	// Update LOD settings
	enableLOD = exposedVariables[1][6][2].get<bool>();
	int newLodLevels = exposedVariables[1][7][2].get<int>();
	float newLodDistance = exposedVariables[1][8][2].get<float>();

	if (newLodLevels != lodLevels || newLodDistance != lodDistance)
	{
		lodLevels = newLodLevels;
		lodDistance = newLodDistance;
		if (enableLOD && terrainGenerated)
			GenerateLODMeshes();
	}

	castShadows = exposedVariables[1][9][2].get<bool>();

	if (needsRegeneration)
		RegenerateTerrain();
}
#endif

void Terrain::Destroy()
{
	if (!terrainGenerated)
		return;

	terrainModel.DeleteInstance();

	for (auto& lod : lodMeshes)
		lod.model.DeleteInstance();

	lodMeshes.clear();
}

// ============================================================================
// TERRAIN GENERATION
// ============================================================================

void Terrain::GenerateTerrain()
{
	// Initialize height data
	heightData.resize(terrainWidth * terrainDepth, 0.0f);
	normals.resize(terrainWidth * terrainDepth);

	// Load heightmap if specified
	if (heightmapSprite != nullptr)
		ApplyHeightmapToTerrain(heightmapSprite);
	else
		GenerateFlatTerrain();

	CalculateNormals();
	UpdateTerrainMesh();

	if (enableLOD)
		GenerateLODMeshes();

	terrainGenerated = true;
}

void Terrain::GenerateFlatTerrain()
{
	std::fill(heightData.begin(), heightData.end(), 1.0f);
}

void Terrain::LoadHeightmap(Sprite* sprite)
{
	heightmapSprite = sprite;
	ApplyHeightmapToTerrain(heightmapSprite);
	UpdateTerrainMesh();
}

void Terrain::ApplyHeightmapToTerrain(Sprite* sprite)
{
	// Todo: I'm not sure if this works properly
	if (!sprite)
	{
		std::fill(heightData.begin(), heightData.end(), 0.0f);
		return;
	}

	// Load heightmap using Raylib
	RaylibWrapper::Image heightmap = RaylibWrapper::LoadImageFromTexture(*sprite->GetTexture());

	if (!heightmap.data)
		return;

	// Resize heightmap to match terrain dimensions if needed
	if (heightmap.width != terrainWidth || heightmap.height != terrainDepth)
		RaylibWrapper::ImageResize(&heightmap, terrainWidth, terrainDepth);

	// Extract height data from image
	RaylibWrapper::Color* pixels = RaylibWrapper::LoadImageColors(heightmap);

	for (int z = 0; z < terrainDepth; z++)
	{
		for (int x = 0; x < terrainWidth; x++)
		{
			int idx = z * terrainWidth + x;
			RaylibWrapper::Color pixel = pixels[idx];

			// Use grayscale value as height
			float height = (pixel.r + pixel.g + pixel.b) / (3.0f * 255.0f);
			heightData[idx] = height * terrainHeight * heightScale;
		}
	}

	RaylibWrapper::UnloadImageColors(pixels);
	RaylibWrapper::UnloadImage(heightmap);

	CalculateNormals();
}

// ============================================================================
// HEIGHT MANIPULATION
// ============================================================================

void Terrain::SetHeight(int x, int z, float height)
{
	if (!IsValidTerrainPosition(x, z))
		return;

	int idx = z * terrainWidth + x;
	heightData[idx] = std::clamp(height, 0.0f, terrainHeight);
}

float Terrain::GetHeight(int x, int z) const
{
	if (!IsValidTerrainPosition(x, z))
		return 0.0f;

	int idx = z * terrainWidth + x;
	return heightData[idx];
}

float Terrain::GetHeightAtWorldPosition(float worldX, float worldZ) const
{
	Vector3 terrainPos = gameObject->transform.GetPosition();
	Vector3 scale = gameObject->transform.GetScale();

	// Convert world position to terrain local position
	float localX = (worldX - terrainPos.x) / scale.x;
	float localZ = (worldZ - terrainPos.z) / scale.z;

	// Convert to terrain grid coordinates
	float terrainX = localX;
	float terrainZ = localZ;

	if (terrainX < 0 || terrainX >= terrainWidth - 1 ||
		terrainZ < 0 || terrainZ >= terrainDepth - 1)
		return 0.0f;

	// Bilinear interpolation
	int x0 = static_cast<int>(terrainX);
	int z0 = static_cast<int>(terrainZ);
	int x1 = x0 + 1;
	int z1 = z0 + 1;

	float fx = terrainX - x0;
	float fz = terrainZ - z0;

	float h00 = GetHeight(x0, z0);
	float h10 = GetHeight(x1, z0);
	float h01 = GetHeight(x0, z1);
	float h11 = GetHeight(x1, z1);

	float h0 = h00 * (1 - fx) + h10 * fx;
	float h1 = h01 * (1 - fx) + h11 * fx;

	return (h0 * (1 - fz) + h1 * fz) * scale.y + terrainPos.y;
}

Vector3 Terrain::GetNormalAtWorldPosition(float worldX, float worldZ) const
{
	int terrainX, terrainZ;
	WorldToTerrainCoords(worldX, worldZ, terrainX, terrainZ);

	if (!IsValidTerrainPosition(terrainX, terrainZ))
		return { 0.0f, 1.0f, 0.0f };

	int idx = terrainZ * terrainWidth + terrainX;
	return normals[idx];
}

// ============================================================================
// SCULPTING FUNCTIONS
// ============================================================================

void Terrain::RaiseTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime)
{
	int centerX, centerZ;
	WorldToTerrainCoords(worldX, worldZ, centerX, centerZ);

	int radiusInt = static_cast<int>(radius);

	for (int z = centerZ - radiusInt; z <= centerZ + radiusInt; z++)
	{
		for (int x = centerX - radiusInt; x <= centerX + radiusInt; x++)
		{
			if (!IsValidTerrainPosition(x, z))
				continue;

			float dx = static_cast<float>(x - centerX);
			float dz = static_cast<float>(z - centerZ);
			float distance = std::sqrt(dx * dx + dz * dz);

			if (distance <= radius)
			{
				float weight = GetBrushWeight(distance, radius);
				int idx = z * terrainWidth + x;
				heightData[idx] += strength * weight * deltaTime;
				heightData[idx] = std::clamp(heightData[idx], 0.0f, terrainHeight);
			}
		}
	}

	CalculateNormals();
	UpdateTerrainMesh();
}

void Terrain::LowerTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime)
{
	RaiseTerrain(worldX, worldZ, radius, -strength, deltaTime);
}

void Terrain::SmoothTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime)
{
	int centerX, centerZ;
	WorldToTerrainCoords(worldX, worldZ, centerX, centerZ);

	int radiusInt = static_cast<int>(radius);
	std::vector<float> smoothedHeights = heightData; // Copy for reading

	for (int z = centerZ - radiusInt; z <= centerZ + radiusInt; z++)
	{
		for (int x = centerX - radiusInt; x <= centerX + radiusInt; x++)
		{
			if (!IsValidTerrainPosition(x, z))
				continue;

			float dx = static_cast<float>(x - centerX);
			float dz = static_cast<float>(z - centerZ);
			float distance = std::sqrt(dx * dx + dz * dz);

			if (distance <= radius)
			{
				// Calculate average height of neighbors
				float avgHeight = 0.0f;
				int count = 0;

				for (int nz = z - 1; nz <= z + 1; nz++)
				{
					for (int nx = x - 1; nx <= x + 1; nx++)
					{
						if (IsValidTerrainPosition(nx, nz))
						{
							avgHeight += heightData[nz * terrainWidth + nx];
							count++;
						}
					}
				}

				if (count > 0)
				{
					avgHeight /= count;
					float weight = GetBrushWeight(distance, radius);
					int idx = z * terrainWidth + x;
					float currentHeight = heightData[idx];
					smoothedHeights[idx] = currentHeight + (avgHeight - currentHeight) * weight * strength * deltaTime;
				}
			}
		}
	}

	heightData = smoothedHeights;
	CalculateNormals();
	UpdateTerrainMesh();
}

void Terrain::FlattenTerrain(float worldX, float worldZ, float radius, float targetHeight, float strength, float deltaTime)
{
	int centerX, centerZ;
	WorldToTerrainCoords(worldX, worldZ, centerX, centerZ);

	int radiusInt = static_cast<int>(radius);

	for (int z = centerZ - radiusInt; z <= centerZ + radiusInt; z++)
	{
		for (int x = centerX - radiusInt; x <= centerX + radiusInt; x++)
		{
			if (!IsValidTerrainPosition(x, z))
				continue;

			float dx = static_cast<float>(x - centerX);
			float dz = static_cast<float>(z - centerZ);
			float distance = std::sqrt(dx * dx + dz * dz);

			if (distance <= radius)
			{
				float weight = GetBrushWeight(distance, radius);
				int idx = z * terrainWidth + x;
				float currentHeight = heightData[idx];
				heightData[idx] = currentHeight + (targetHeight - currentHeight) * weight * strength * deltaTime;
			}
		}
	}

	CalculateNormals();
	UpdateTerrainMesh();
}

float Terrain::GetBrushWeight(float distance, float radius) const
{
	if (distance >= radius)
		return 0.0f;

	// Smooth falloff using cosine interpolation
	float t = distance / radius;
	return (std::cos(t * 3.14159f) + 1.0f) * 0.5f;
}

void Terrain::WorldToTerrainCoords(float worldX, float worldZ, int& terrainX, int& terrainZ) const
{
	Vector3 terrainPos = gameObject->transform.GetPosition();
	Vector3 scale = gameObject->transform.GetScale();

	float localX = (worldX - terrainPos.x) / scale.x;
	float localZ = (worldZ - terrainPos.z) / scale.z;

	terrainX = static_cast<int>(localX);
	terrainZ = static_cast<int>(localZ);
}

// ============================================================================
// TEXTURE PAINTING
// ============================================================================

void Terrain::PaintTexture(float worldX, float worldZ, float radius, int layerIndex, float strength, float deltaTime)
{
	if (layerIndex < 0 || layerIndex >= splatMaps.size())
		return;

	int centerX, centerZ;
	WorldToTerrainCoords(worldX, worldZ, centerX, centerZ);

	int radiusInt = static_cast<int>(radius);

	for (int z = centerZ - radiusInt; z <= centerZ + radiusInt; z++)
	{
		for (int x = centerX - radiusInt; x <= centerX + radiusInt; x++)
		{
			if (!IsValidTerrainPosition(x, z))
				continue;

			float dx = static_cast<float>(x - centerX);
			float dz = static_cast<float>(z - centerZ);
			float distance = std::sqrt(dx * dx + dz * dz);

			if (distance <= radius)
			{
				float weight = GetBrushWeight(distance, radius);
				int idx = z * terrainWidth + x;

				// Increase weight for target layer
				splatMaps[layerIndex][idx] += strength * weight * deltaTime;
				splatMaps[layerIndex][idx] = std::clamp(splatMaps[layerIndex][idx], 0.0f, 1.0f);

				// Normalize all layer weights at this position
				NormalizeSplatWeights(x, z);
			}
		}
	}

	UpdateSplatMap();
}

void Terrain::AddTerrainLayer(Material* material, const std::string& name)
{
	TerrainLayer layer;
	layer.material = material;
	layer.name = name.empty() ? "Layer " + std::to_string(terrainLayers.size()) : name;
	terrainLayers.push_back(layer);

	// Add new splat map initialized to 0
	splatMaps.push_back(std::vector<float>(terrainWidth * terrainDepth, 0.0f));
}

void Terrain::RemoveTerrainLayer(int layerIndex)
{
	if (layerIndex <= 0 || layerIndex >= terrainLayers.size())
		return; // Can't remove base layer

	terrainLayers.erase(terrainLayers.begin() + layerIndex);
	splatMaps.erase(splatMaps.begin() + layerIndex);

	// Renormalize all weights
	for (int z = 0; z < terrainDepth; z++)
	{
		for (int x = 0; x < terrainWidth; x++)
		{
			NormalizeSplatWeights(x, z);
		}
	}

	UpdateSplatMap();
}

void Terrain::NormalizeSplatWeights(int x, int z)
{
	int idx = z * terrainWidth + x;
	float total = 0.0f;

	for (const auto& splatMap : splatMaps)
	{
		total += splatMap[idx];
	}

	if (total > 0.0f)
	{
		for (auto& splatMap : splatMaps)
		{
			splatMap[idx] /= total;
		}
	}
}

void Terrain::UpdateSplatMap()
{
	// This would update shader uniforms or textures with splat map data
	// Implementation depends on your material/shader system
	// For now, this is a placeholder for future shader integration
}

// ============================================================================
// MESH GENERATION AND UPDATE
// ============================================================================

void Terrain::UpdateTerrainMesh()
{
	std::vector<float> vertices;
	std::vector<unsigned short> indices;
	std::vector<float> texcoords;
	std::vector<float> normalsData;

	GenerateMeshData(vertices, indices, texcoords, normalsData, 0);

	ConsoleLogger::InfoLog("Generating terrain mesh: " +
		std::to_string(vertices.size() / 3) + " vertices, " +
		std::to_string(indices.size() / 3) + " triangles");

	// Debug output
	if (vertices.size() >= 9 && indices.size() >= 6) {
		ConsoleLogger::InfoLog("First 3 vertices:");
		for (int i = 0; i < 9; i += 3) {
			ConsoleLogger::InfoLog("  V" + std::to_string(i / 3) + ": (" +
				std::to_string(vertices[i]) + ", " +
				std::to_string(vertices[i + 1]) + ", " +
				std::to_string(vertices[i + 2]) + ")");
		}
		ConsoleLogger::InfoLog("First 2 triangles: [" +
			std::to_string(indices[0]) + "," + std::to_string(indices[1]) + "," + std::to_string(indices[2]) + "] [" +
			std::to_string(indices[3]) + "," + std::to_string(indices[4]) + "," + std::to_string(indices[5]) + "]");
	}

	// Cleanup old mesh if it exists
	if (terrainGenerated)
		terrainModel.DeleteInstance();

	// Create Raylib mesh with CPU-side data
	RaylibWrapper::Mesh mesh = { 0 };
	mesh.vertexCount = static_cast<int>(vertices.size() / 3);
	mesh.triangleCount = static_cast<int>(indices.size() / 3);

	// Allocate and copy vertex data
	mesh.vertices = (float*)RL_MALLOC(vertices.size() * sizeof(float));
	memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));

	// Allocate and copy texcoord data
	mesh.texcoords = (float*)RL_MALLOC(texcoords.size() * sizeof(float));
	memcpy(mesh.texcoords, texcoords.data(), texcoords.size() * sizeof(float));

	// Allocate and copy normal data
	mesh.normals = (float*)RL_MALLOC(normalsData.size() * sizeof(float));
	memcpy(mesh.normals, normalsData.data(), normalsData.size() * sizeof(float));

	// Allocate and copy index data
	mesh.indices = (unsigned short*)RL_MALLOC(indices.size() * sizeof(unsigned short));
	memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned short));

	// Let UploadMesh handle GPU upload
	// This properly initializes vaoId and vboId
	RaylibWrapper::UploadMesh(&mesh, false);

	// Now pass to model wrapper
	std::vector<unsigned int> vboIds = {
		mesh.vboId[0],  // vertices
		mesh.vboId[1],  // texcoords  
		mesh.vboId[2],  // normals
		mesh.vboId[6]   // indices
	};

	terrainModel.CreateTerrain(mesh.vertexCount, mesh.triangleCount, mesh.vaoId, vboIds);
}

void Terrain::GenerateMeshData(std::vector<float>& vertices, std::vector<unsigned short>& indices,
	std::vector<float>& texcoords, std::vector<float>& normalsData, int lodLevel)
{
	int step = 1 << lodLevel; // 1, 2, 4, 8... based on LOD level
	int resolutionX = (terrainWidth - 1) / step + 1;
	int resolutionZ = (terrainDepth - 1) / step + 1;

	vertices.reserve(resolutionX * resolutionZ * 3);
	texcoords.reserve(resolutionX * resolutionZ * 2);
	normalsData.reserve(resolutionX * resolutionZ * 3);
	indices.reserve((resolutionX - 1) * (resolutionZ - 1) * 6);

	// Generate vertices
	for (int z = 0; z < resolutionZ; z++)
	{
		for (int x = 0; x < resolutionX; x++)
		{
			int actualX = std::min(x * step, terrainWidth - 1);
			int actualZ = std::min(z * step, terrainDepth - 1);

			float height = GetHeight(actualX, actualZ);

			// Vertex position
			vertices.push_back(static_cast<float>(actualX));
			vertices.push_back(height);
			vertices.push_back(static_cast<float>(actualZ));

			// Texture coordinates
			texcoords.push_back(static_cast<float>(actualX) / (terrainWidth - 1));
			texcoords.push_back(static_cast<float>(actualZ) / (terrainDepth - 1));

			// Normals
			int idx = actualZ * terrainWidth + actualX;
			normalsData.push_back(normals[idx].x);
			normalsData.push_back(normals[idx].y);
			normalsData.push_back(normals[idx].z);
		}
	}

	// Generate indices
	for (int z = 0; z < resolutionZ - 1; z++)
	{
		for (int x = 0; x < resolutionX - 1; x++)
		{
			unsigned short topLeft = z * resolutionX + x;
			unsigned short topRight = topLeft + 1;
			unsigned short bottomLeft = (z + 1) * resolutionX + x;
			unsigned short bottomRight = bottomLeft + 1;

			// First triangle
			indices.push_back(topLeft);
			indices.push_back(bottomLeft);
			indices.push_back(topRight);

			// Second triangle
			indices.push_back(topRight);
			indices.push_back(bottomLeft);
			indices.push_back(bottomRight);
		}
	}
}

void Terrain::CalculateNormals()
{
	// Initialize normals to zero
	for (auto& normal : normals)
	{
		normal = { 0.0f, 0.0f, 0.0f };
	}

	// Calculate normals for each vertex
	for (int z = 0; z < terrainDepth; z++)
	{
		for (int x = 0; x < terrainWidth; x++)
		{
			normals[z * terrainWidth + x] = CalculateNormalAt(x, z);
		}
	}
}

Vector3 Terrain::CalculateNormalAt(int x, int z) const
{
	// Use central differences to calculate normal
	float heightL = GetHeight(x - 1, z);
	float heightR = GetHeight(x + 1, z);
	float heightD = GetHeight(x, z - 1);
	float heightU = GetHeight(x, z + 1);

	Vector3 normal;
	normal.x = heightL - heightR;
	normal.y = 2.0f;
	normal.z = heightD - heightU;

	// Normalize
	float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
	if (length > 0.0f)
	{
		normal.x /= length;
		normal.y /= length;
		normal.z /= length;
	}

	return normal;
}

// ============================================================================
// LOD SYSTEM
// ============================================================================

void Terrain::GenerateLODMeshes()
{
	// Clear existing LOD meshes
	for (auto& lod : lodMeshes)
	{
		lod.model.DeleteInstance();
	}
	lodMeshes.clear();

	if (!enableLOD)
		return;

	// Generate LOD levels
	for (int i = 0; i < lodLevels; i++)
	{
		TerrainLOD lod;
		int step = 1 << (i + 1);
		lod.resolution = (terrainWidth - 1) / step + 1;
		lod.maxDistance = lodDistance * (i + 1);

		std::vector<float> vertices;
		std::vector<unsigned short> indices;
		std::vector<float> texcoords;
		std::vector<float> normalsData;

		GenerateMeshData(vertices, indices, texcoords, normalsData, i + 1);

		// Create Raylib mesh
		RaylibWrapper::Mesh mesh = { 0 };
		mesh.vertexCount = static_cast<int>(vertices.size() / 3);
		mesh.triangleCount = static_cast<int>(indices.size() / 3);

		mesh.vertices = (float*)RL_MALLOC(vertices.size() * sizeof(float));
		memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));

		mesh.texcoords = (float*)RL_MALLOC(texcoords.size() * sizeof(float));
		memcpy(mesh.texcoords, texcoords.data(), texcoords.size() * sizeof(float));

		mesh.normals = (float*)RL_MALLOC(normalsData.size() * sizeof(float));
		memcpy(mesh.normals, normalsData.data(), normalsData.size() * sizeof(float));

		mesh.indices = (unsigned short*)RL_MALLOC(indices.size() * sizeof(unsigned short));
		memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned short));

		RaylibWrapper::UploadMesh(&mesh, false);

		std::vector<unsigned int> vboIds = {
			mesh.vboId[0], mesh.vboId[1], mesh.vboId[2], mesh.vboId[6]
		};

		lod.model.CreateTerrain(mesh.vertexCount, mesh.triangleCount, mesh.vaoId, vboIds);

		lodMeshes.push_back(lod);

		ConsoleLogger::InfoLog("Generated LOD " + std::to_string(i + 1) +
			" with " + std::to_string(mesh.vertexCount) + " vertices, " +
			std::to_string(mesh.triangleCount) + " triangles");
	}
}

void Terrain::UpdateLOD(const Vector3& cameraPosition)
{
	if (!enableLOD || lodMeshes.empty())
	{
		currentLODLevel = 0;
		return;
	}

	// Calculate distance from camera to terrain center
	Vector3 terrainPos = gameObject->transform.GetPosition();
	Vector3 scale = gameObject->transform.GetScale();
	Vector3 terrainCenter = {
		terrainPos.x + (terrainWidth * scale.x) * 0.5f,
		terrainPos.y,
		terrainPos.z + (terrainDepth * scale.z) * 0.5f
	};

	float dx = cameraPosition.x - terrainCenter.x;
	float dy = cameraPosition.y - terrainCenter.y;
	float dz = cameraPosition.z - terrainCenter.z;
	float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

	// Select appropriate LOD level
	// LOD 0 = base mesh (highest detail)
	// LOD 1+ = simplified meshes
	currentLODLevel = 0;

	for (int i = 0; i < lodMeshes.size(); i++)
	{
		// Switch to this LOD if distance exceeds its threshold
		if (distance > lodMeshes[i].maxDistance)
		{
			currentLODLevel = i + 1;
		}
	}

	// Clamp to valid range
	currentLODLevel = std::min(currentLODLevel, static_cast<int>(lodMeshes.size()));

	// Debug output
	static int lastLOD = -1;
	if (currentLODLevel != lastLOD) {
		ConsoleLogger::InfoLog("Switched to LOD level " + std::to_string(currentLODLevel) +
			" (distance: " + std::to_string(distance) + ")");
		lastLOD = currentLODLevel;
	}
}

void Terrain::SwitchLODMesh(int lodLevel)
{
	if (lodLevel < 0 || lodLevel >= lodMeshes.size())
		return;

	currentLODLevel = lodLevel;
}

// ============================================================================
// MATERIAL AND UTILITY
// ============================================================================

void Terrain::SetMaterial(Material* mat)
{
	terrainMaterial = mat;
}

void Terrain::RegenerateTerrain()
{
	if (terrainGenerated)
	{
		terrainModel.DeleteInstance();
		for (auto& lod : lodMeshes)
		{
			lod.model.DeleteInstance();
		}
		lodMeshes.clear();
		terrainGenerated = false;
	}

	// Resize data structures
	heightData.resize(terrainWidth * terrainDepth, 0.0f);
	normals.resize(terrainWidth * terrainDepth);

	for (auto& splatMap : splatMaps)
	{
		splatMap.resize(terrainWidth * terrainDepth, 0.0f);
	}

	// Fill base layer
	if (!splatMaps.empty())
	{
		std::fill(splatMaps[0].begin(), splatMaps[0].end(), 1.0f);
	}

	GenerateTerrain();
}

Vector3 Terrain::GetTerrainSize() const
{
	Vector3 scale = gameObject->transform.GetScale();
	return {
		terrainWidth * scale.x,
		terrainHeight * scale.y,
		terrainDepth * scale.z
	};
}

bool Terrain::IsValidTerrainPosition(int x, int z) const
{
	return x >= 0 && x < terrainWidth && z >= 0 && z < terrainDepth;
}