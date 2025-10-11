#include "Terrain.h"
#include "../RaylibWrapper.h"
#include <algorithm>

void Terrain::Awake()
{
	if (heightmapSprite)
		GenerateFromHeightmap();
	else
		GenerateTerrain();
}

void Terrain::GenerateTerrain()
{
	heightData.resize(terrainDepth, std::vector<float>(terrainWidth, 0.0f));

	// Generate flat mesh
	raylibModel.CreateFromHeightData(heightData, terrainWidth, terrainDepth, terrainHeight, terrainMaterial->GetRaylibMaterial());
	modelGenerated = true;
}

void Terrain::GenerateFromHeightmap()
{
	// Load image data
	RaylibWrapper::Image img = RaylibWrapper::LoadImage(heightmapSprite->GetPath().c_str());
	RaylibWrapper::ImageFormat(&img, RaylibWrapper::PIXELFORMAT_UNCOMPRESSED_R8G8B8);

	if (img.width != terrainWidth || img.height != terrainDepth)
		RaylibWrapper::ImageResize(&img, terrainWidth, terrainDepth);

	heightData.resize(terrainDepth, std::vector<float>(terrainWidth));

	RaylibWrapper::Color* pixels = RaylibWrapper::LoadImageColors(img);
	for (int z = 0; z < terrainDepth; z++)
	{
		for (int x = 0; x < terrainWidth; x++)
		{
			int index = z * terrainWidth + x;
			float gray = (pixels[index].r + pixels[index].g + pixels[index].b) / 3.0f / 255.0f;
			heightData[z][x] = gray * terrainHeight * heightScale;
		}
	}
	UnloadImageColors(pixels);
	UnloadImage(img);

	raylibModel.CreateFromHeightData(heightData, terrainWidth, terrainDepth, terrainHeight, terrainMaterial->GetRaylibMaterial());
	modelGenerated = true;
}

void Terrain::Render(bool renderShadows)
{
	if (!modelGenerated || (renderShadows && !castShadows))
		return;

	Vector3 pos = gameObject->transform.GetPosition();
	Quaternion rot = gameObject->transform.GetRotation();
	//Vector3 scale = gameObject->transform.GetScale(); // Terrain scaling from GameObject Transform is not supported
	Vector3 scale = { 1.0f, 1.0f, 1.0f };

	Vector3 centeredPos = {
		pos.x - (terrainWidth * 0.5f * scale.x),
		pos.y,
		pos.z - (terrainDepth * 0.5f * scale.z)
	};

	raylibModel.DrawModelWrapper(centeredPos.x, centeredPos.y, centeredPos.z,
		scale.x, scale.y, scale.z,
		rot.x, rot.y, rot.z, rot.w,
		255, 255, 255, 255);
}

void Terrain::Update()
{
	if (needsRebuild)
	{
		if (lastRebuild >= 0.05f)
		{
			RebuildMesh(); // Todo: Should this call GenerateFromHeightmap() if a heightmap is being used?

			needsRebuild = false;
			lastRebuild = 0.0f;
		}
		else
			lastRebuild += RaylibWrapper::GetFrameTime();
	}
}

#if defined(EDITOR)
void Terrain::EditorUpdate()
{
	bool shouldRebuild = false;

	// Store old values to detect changes
	int oldWidth = terrainWidth;
	int oldDepth = terrainDepth;
	float oldHeight = terrainHeight;
	float oldScale = heightScale;

	// Update values from exposedVariables
	terrainWidth = exposedVariables[1][0][2];
	terrainDepth = exposedVariables[1][1][2];
	terrainHeight = exposedVariables[1][2][2];
	heightScale = exposedVariables[1][3][2];
	enableLOD = exposedVariables[1][6][2];
	lodLevels = exposedVariables[1][7][2];
	lodDistance = exposedVariables[1][8][2];
	castShadows = exposedVariables[1][9][2];

	if (setupEditor)
	{
		setupEditor = false;
		Awake();
	}

	// Detect if any rebuild-triggering variable changed
	if (terrainWidth != oldWidth || terrainDepth != oldDepth || terrainHeight != oldHeight || heightScale != oldScale)
		shouldRebuild = true;

	// --- Handle heightmap texture changes ---
	std::string spritePath = exposedVariables[1][4][2];
	bool heightmapChanged = false;
	if ((!heightmapSprite && spritePath != "nullptr" && !spritePath.empty()) ||
		(heightmapSprite && spritePath != heightmapSprite->GetRelativePath()))
	{
		if (heightmapSprite != nullptr)
		{
			delete heightmapSprite;
			heightmapSprite = nullptr;
		}

		if (spritePath != "nullptr" && !spritePath.empty())
		{
			heightmapSprite = new Sprite(spritePath);
			heightmapChanged = true;
		}

		shouldRebuild = true; // texture change also triggers rebuild
	}

	// --- Handle material changes ---
	std::string matPath = exposedVariables[1][5][2];

	if (matPath != "nullptr" && (!terrainMaterial || terrainMaterial->GetPath() != matPath))
	{
		terrainMaterial = Material::GetMaterial(matPath);
		if (modelGenerated)
			RebuildMesh(); // Todo: Should this call GenerateFromHeightmap() if a heightmap is being used?
	}

	// --- Perform rebuild if needed ---
	if (shouldRebuild)
	{
		if (heightmapSprite)
			GenerateFromHeightmap(); // Todo: Should RebuildMesh() be called instead?
		else
			RebuildMesh();
	}
}
#endif

void Terrain::RebuildMesh()
{
	// Resize the heightData if needed
	if ((int)heightData.size() != terrainDepth || (int)heightData[0].size() != terrainWidth)
		heightData.assign(terrainDepth, std::vector<float>(terrainWidth, 0.0f));


	raylibModel.CreateFromHeightData(heightData, terrainWidth, terrainDepth, terrainHeight, terrainMaterial->GetRaylibMaterial());
}

void Terrain::Destroy()
{
	if (modelGenerated)
		raylibModel.DeleteInstance();
}

bool Terrain::RaycastToTerrain(const RaylibWrapper::Ray& ray, Vector3& hitPos)
{
	float maxDistance = 1000.0f;
	float step = 0.5f; // Acciracuy of the raycast. Smaller = more accurate.
	for (float t = 0; t < maxDistance; t += step)
	{
		RaylibWrapper::Vector3 point = RaylibWrapper::Vector3Add(ray.position, RaylibWrapper::Vector3Scale(ray.direction, t));

		// Convert world position to local terrain coordinates. This actually isnt needed
		//RaylibWrapper::Vector3 local = WorldToLocal(point);
		RaylibWrapper::Vector3 local = point;

		// Center it since the terrain is rendered from the center
		local.x = point.x - (gameObject->transform.GetPosition().x - terrainWidth / 2.0f);
		local.z = point.z - (gameObject->transform.GetPosition().z - terrainDepth / 2.0f);

		// Check bounds
		if (local.x < 0 || local.x >= terrainWidth || local.z < 0 || local.z >= terrainDepth)
			continue;

		float terrainY = heightData[(int)local.z][(int)local.x];
		//if (point.y <= terrainY)
		//{
		//	hitPos = { point.x, point.y, point.z };
		//	return true;
		//}
		float prevY = heightData[(int)local.z][(int)local.x];
		if (point.y <= prevY) {
			hitPos = { point.x, prevY, point.z };
			return true;
		}
	}
	return false;
}

int Terrain::WorldToHeightmapX(float worldX) const
{
	// If I add support for game object scaling in the future, this function will need to be updated
	float halfWidth = terrainWidth * 0.5f;
	float terrainWorldX = gameObject->transform.GetPosition().x;
	return (int)((worldX - (terrainWorldX - halfWidth)));
}

int Terrain::WorldToHeightmapZ(float worldZ) const
{
	// If I add support for game object scaling in the future, this function will need to be updated
	float halfDepth = terrainDepth * 0.5f;
	float terrainWorldZ = gameObject->transform.GetPosition().z;
	return (int)((worldZ - (terrainWorldZ - halfDepth)));
}

// ============= Height Manipulation =============

void Terrain::SetHeight(int x, int z, float height)
{
	if (x < 0 || z < 0 || x >= terrainWidth || z >= terrainDepth) return;
	heightData[z][x] = height;
}

float Terrain::GetHeight(int x, int z) const
{
	// Clamps the values
	if (x < 0) x = 0;
	else if (x >= terrainWidth) x = terrainWidth - 1;

	if (z < 0) z = 0;
	else if (z >= terrainDepth) z = terrainDepth - 1;

	return heightData[z][x];
}

float Terrain::GetHeightAtWorldPosition(float worldX, float worldZ) const
{
	float localX = worldX - gameObject->transform.GetPosition().x;
	float localZ = worldZ - gameObject->transform.GetPosition().z;

	int x = (int)(localX + terrainWidth / 2);
	int z = (int)(localZ + terrainDepth / 2);

	return GetHeight(x, z);
}

Vector3 Terrain::GetNormalAtWorldPosition(float worldX, float worldZ) const
{
	int x = (int)(worldX);
	int z = (int)(worldZ);

	float hL = GetHeight(x - 1, z);
	float hR = GetHeight(x + 1, z);
	float hD = GetHeight(x, z - 1);
	float hU = GetHeight(x, z + 1);

	RaylibWrapper::Vector3 normalized = RaylibWrapper::Vector3Normalize({ hL - hR, 2.0f, hD - hU });
	return { normalized.x, normalized.y, normalized.z };
}

//Vector3 Terrain::GetNormalAtWorldPosition(float worldX, float worldZ) const // Old method. This method might be faster, but may also have issues. Compare the two functions
//{
//	int x = (int)worldX;
//	int z = (int)worldZ;
//
//	// Clamp to terrain range
//	//x = Clamp(x, 1, terrainWidth - 2);
//	//z = Clamp(z, 1, terrainDepth - 2);
//
//	if (x < 1) x = 1;
//	if (x > terrainWidth - 2) x = terrainWidth - 2;
//
//	if (z < 1) z = 1;
//	if (z > terrainDepth - 2) z = terrainDepth - 2;
//
//	float hL = heightData[z][x - 1];
//	float hR = heightData[z][x + 1];
//	float hD = heightData[z - 1][x];
//	float hU = heightData[z + 1][x];
//
//	Vector3 normal = { hL - hR, 2.0f, hD - hU };
//	return normal.Normalize();
//}

// ============= Sculpting =============

void Terrain::RaiseTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime)
{
	for (int z = 0; z < terrainDepth; z++)
	{
		for (int x = 0; x < terrainWidth; x++)
		{
			float dx = x - worldX;
			float dz = z - worldZ;
			float dist = sqrtf(dx * dx + dz * dz);
			if (dist < radius)
			{
				float falloff = 0.5f * (cosf(dist / radius * 3.14159f) + 1.0f);
				heightData[z][x] = std::min(heightData[z][x] + strength * falloff * deltaTime, terrainHeight);
			}
		}
	}

	needsRebuild = true;
	//RebuildMesh(); // Let Update() handle the rebuild
}

void Terrain::LowerTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime)
{
	RaiseTerrain(worldX, worldZ, radius, -strength, deltaTime);
}

void Terrain::SmoothTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime)
{
	std::vector<std::vector<float>> newHeights = heightData;
	for (int z = 1; z < terrainDepth - 1; z++)
	{
		for (int x = 1; x < terrainWidth - 1; x++)
		{
			float dx = x - worldX;
			float dz = z - worldZ;
			float dist = sqrtf(dx * dx + dz * dz);
			if (dist < radius)
			{
				float avg = (
					GetHeight(x - 1, z) +
					GetHeight(x + 1, z) +
					GetHeight(x, z - 1) +
					GetHeight(x, z + 1) +
					GetHeight(x, z)) / 5.0f;
				newHeights[z][x] = heightData[z][x] + (avg - heightData[z][x]) * strength * deltaTime;
			}
		}
	}
	heightData = newHeights;

	needsRebuild = true;
	//RebuildMesh(); // Let Update() handle the rebuild
}

void Terrain::FlattenTerrain(float worldX, float worldZ, float radius, float strength, float targetHeight, float deltaTime)
{
	if (targetHeight > terrainHeight)
		targetHeight = terrainHeight;

	for (int z = 0; z < terrainDepth; z++)
	{
		for (int x = 0; x < terrainWidth; x++)
		{
			float dx = x - worldX;
			float dz = z - worldZ;
			float dist = sqrtf(dx * dx + dz * dz);
			if (dist < radius)
			{
				heightData[z][x] += (targetHeight - heightData[z][x]) * strength * deltaTime;
			}
		}
	}

	needsRebuild = true;
	//RebuildMesh(); // Let Update() handle the rebuild
}

// ============= Texture Painting =============

void Terrain::PaintTexture(float worldX, float worldZ, float radius, int layerIndex, float strength, float deltaTime)
{
	// Placeholder: implement actual splatmap blending
	if (layerIndex < 0 || layerIndex >= GetLayerCount()) return;
	// Example: blend texture layer based on brush radius
}

void Terrain::AddTerrainLayer(Material* material, const std::string& name)
{
	terrainLayers.push_back(material);
}

void Terrain::RemoveTerrainLayer(int layerIndex)
{
	if (layerIndex < 0 || layerIndex >= GetLayerCount()) return;
	terrainLayers.erase(terrainLayers.begin() + layerIndex);
}

nlohmann::json Terrain::SerializeHeightData()
{
	nlohmann::json dataJson;

	dataJson["heightData"] = heightData;

	return dataJson;
}

void Terrain::LoadHeightData(const nlohmann::json& data)
{
	if (!data.contains("heightData") || !data["heightData"].is_array())
	{
		ConsoleLogger::ErrorLog(gameObject->GetName() + "'s Terrain failed to load terrain data.");
		return;
	}

	heightData.clear();

	for (auto& row : data["heightData"])
		heightData.push_back(row.get<std::vector<float>>());
}