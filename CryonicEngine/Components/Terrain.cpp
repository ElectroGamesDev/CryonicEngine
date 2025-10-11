#include "Terrain.h"
#include "../RaylibWrapper.h"
#include <algorithm>

void Terrain::Awake()
{
	InitializeMaterial();
	LoadTerrainShader();

	if (heightmapSprite)
		GenerateFromHeightmap();
	else
		GenerateTerrain();

	InitializeSplatmaps();
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

	// Bind terrain layer textures
	if (!terrainLayers.empty())
	{
		// Bind splatmap to texture unit 5
		RaylibWrapper::rlActiveTextureSlot(5);
		RaylibWrapper::rlEnableTexture(splatmapTexture.id);

		// Bind layer textures to units 0-3
		if (terrainLayers.size() > 0 && terrainLayers[0].material)
		{
			RaylibWrapper::rlActiveTextureSlot(0);
			RaylibWrapper::rlEnableTexture(terrainLayers[0].material->GetAlbedoSprite()->GetTexture()->id);
		}

		if (terrainLayers.size() > 1 && terrainLayers[1].material)
		{
			RaylibWrapper::rlActiveTextureSlot(1);
			RaylibWrapper::rlEnableTexture(terrainLayers[1].material->GetAlbedoSprite()->GetTexture()->id);
		}

		if (terrainLayers.size() > 2 && terrainLayers[2].material)
		{
			RaylibWrapper::rlActiveTextureSlot(2);
			RaylibWrapper::rlEnableTexture(terrainLayers[2].material->GetAlbedoSprite()->GetTexture()->id);
		}

		if (terrainLayers.size() > 3 && terrainLayers[3].material)
		{
			RaylibWrapper::rlActiveTextureSlot(3);
			RaylibWrapper::rlEnableTexture(terrainLayers[3].material->GetAlbedoSprite()->GetTexture()->id);
		}

		// Reset to default texture slot
		RaylibWrapper::rlActiveTextureSlot(0);
	}

	ConsoleLogger::ErrorLog("------------------------------------------------------------------------ ");
	for (int i = 0; i < raylibModel.GetMaterialCount(); i++)
	{
		int matID = raylibModel.GetTextureID(i, RaylibWrapper::MATERIAL_MAP_ALBEDO);
		ConsoleLogger::ErrorLog("Terrain Texture ID " + std::to_string(i) + ": " + std::to_string(matID));
		ConsoleLogger::ErrorLog("Shader ID " + std::to_string(i) + ": " + std::to_string(raylibModel.GetShaderID(i)));
	}

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

			UpdateSplatmapTexture();
			needsSplatmapUpdate = false;

			needsRebuild = false;
			lastRebuild = 0.0f;
		}
		else
			lastRebuild += RaylibWrapper::GetFrameTime();
	}

	if (needsSplatmapUpdate)
	{
		UpdateSplatmapTexture();
		needsSplatmapUpdate = false;
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
	enableLOD = exposedVariables[1][5][2];
	lodLevels = exposedVariables[1][6][2];
	lodDistance = exposedVariables[1][7][2];
	castShadows = exposedVariables[1][8][2];

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

	if (splatmapGenerated)
	{
		RaylibWrapper::UnloadTexture(splatmapTexture);
		splatmapGenerated = false;
	}

	if (terrainMaterial)
	{
		delete terrainMaterial; // Material destructor handles unloading the texture
		terrainMaterial = nullptr;
	}
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
	float centeredX = WorldToHeightmapX(worldX);
	float centeredZ = WorldToHeightmapZ(worldZ);

	for (int z = 0; z < terrainDepth; z++)
	{
		for (int x = 0; x < terrainWidth; x++)
		{
			float dx = x - centeredX;
			float dz = z - centeredZ;
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
	float centeredX = WorldToHeightmapX(worldX);
	float centeredZ = WorldToHeightmapZ(worldZ);

	std::vector<std::vector<float>> newHeights = heightData;
	for (int z = 1; z < terrainDepth - 1; z++)
	{
		for (int x = 1; x < terrainWidth - 1; x++)
		{
			float dx = x - centeredX;
			float dz = z - centeredZ;
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
	float centeredX = WorldToHeightmapX(worldX);
	float centeredZ = WorldToHeightmapZ(worldZ);

	if (targetHeight > terrainHeight)
		targetHeight = terrainHeight;

	for (int z = 0; z < terrainDepth; z++)
	{
		for (int x = 0; x < terrainWidth; x++)
		{
			float dx = x - centeredX;
			float dz = z - centeredZ;
			float dist = sqrtf(dx * dx + dz * dz);
			if (dist < radius)
				heightData[z][x] += (targetHeight - heightData[z][x]) * strength * deltaTime;
		}
	}

	needsRebuild = true;
	//RebuildMesh(); // Let Update() handle the rebuild
}

// ============= Texture Painting =============

void Terrain::InitializeSplatmaps()
{
	// Initialize splatmaps for all layers
	for (auto& layer : terrainLayers)
	{
		if (layer.splatmap.empty())
		{
			layer.splatmap.resize(terrainDepth, std::vector<float>(terrainWidth, 0.0f));
		}
	}

	// If we have at least one layer, set it to full strength by default
	if (!terrainLayers.empty() && terrainLayers[0].splatmap.size() > 0)
	{
		for (int z = 0; z < terrainDepth; z++)
		{
			for (int x = 0; x < terrainWidth; x++)
			{
				terrainLayers[0].splatmap[z][x] = 1.0f;
			}
		}
	}

	needsSplatmapUpdate = true;
}

void Terrain::LoadTerrainShader()
{
	// Load custom terrain shader
	terrainShader = ShaderManager::GetShader(ShaderManager::Shaders::Terrain);

	// Get shader uniform locations
	splatmapLoc = RaylibWrapper::GetShaderLocation({ terrainShader.first, terrainShader.second }, "splatmap");
	texture1Loc = RaylibWrapper::GetShaderLocation({ terrainShader.first, terrainShader.second }, "texture1");
	texture2Loc = RaylibWrapper::GetShaderLocation({ terrainShader.first, terrainShader.second }, "texture2");
	texture3Loc = RaylibWrapper::GetShaderLocation({ terrainShader.first, terrainShader.second }, "texture3");
	textureScaleLoc = RaylibWrapper::GetShaderLocation({ terrainShader.first, terrainShader.second }, "textureScale");

	float defaultScale = 10.0f;
	RaylibWrapper::SetShaderValue({ terrainShader.first, terrainShader.second }, textureScaleLoc, &defaultScale, RaylibWrapper::SHADER_UNIFORM_FLOAT);
}

void Terrain::UpdateShaderTextures()
{
	if (terrainLayers.empty())
		return;

	// Set texture unit indices for each layer in the shader
	// texture0 = unit 0, texture1 = unit 1, etc.
	
	int unit0 = 0;
	RaylibWrapper::SetShaderValue({ terrainShader.first, terrainShader.second }, 
		RaylibWrapper::GetShaderLocation({ terrainShader.first, terrainShader.second }, "texture0"), 
		&unit0, RaylibWrapper::SHADER_UNIFORM_INT);

	if (terrainLayers.size() > 1)
	{
		int unit1 = 1;
		RaylibWrapper::SetShaderValue({ terrainShader.first, terrainShader.second }, texture1Loc, &unit1, RaylibWrapper::SHADER_UNIFORM_INT);
	}

	if (terrainLayers.size() > 2)
	{
		int unit2 = 2;
		RaylibWrapper::SetShaderValue({ terrainShader.first, terrainShader.second }, texture2Loc, &unit2, RaylibWrapper::SHADER_UNIFORM_INT);
	}

	if (terrainLayers.size() > 3)
	{
		int unit3 = 3;
		RaylibWrapper::SetShaderValue({ terrainShader.first, terrainShader.second }, texture3Loc, &unit3, RaylibWrapper::SHADER_UNIFORM_INT);
	}
}

void Terrain::InitializeMaterial()
{
	if (!checkerboardTextureLoaded)
	{
		// Todo: This texture is never being unloaded, but it's not a big concern
		checkerboardTexture = Material::GenerateCheckerboardTexture();
		checkerboardTextureLoaded = true;
	}

	terrainMaterial = new Material("null");
	terrainMaterial->GetRaylibMaterial()->maps[RaylibWrapper::MATERIAL_MAP_ALBEDO].color = { 255, 255, 255, 255 };
	terrainMaterial->GetRaylibMaterial()->maps[RaylibWrapper::MATERIAL_MAP_ALBEDO].texture = checkerboardTexture;
	terrainMaterial->GetRaylibMaterial()->shader.id = terrainShader.first;
	terrainMaterial->GetRaylibMaterial()->shader.locs = terrainShader.second;
}

void Terrain::NormalizeSplatmaps(int x, int z)
{
	// Ensure all weights at this position sum to 1.0
	if (terrainLayers.empty()) return;

	float totalWeight = 0.0f;
	for (auto& layer : terrainLayers)
	{
		if (z < (int)layer.splatmap.size() && x < (int)layer.splatmap[z].size())
			totalWeight += layer.splatmap[z][x];
	}

	if (totalWeight > 0.0f)
	{
		for (auto& layer : terrainLayers)
		{
			if (z < (int)layer.splatmap.size() && x < (int)layer.splatmap[z].size())
				layer.splatmap[z][x] /= totalWeight;
		}
	}
}

void Terrain::PaintTexture(float worldX, float worldZ, float radius, float strength, int layerIndex, float deltaTime)
{
	if (layerIndex < 0 || layerIndex >= GetLayerCount())
		return;

	// Convert world coordinates to heightmap coordinates
	float centeredX = WorldToHeightmapX(worldX);
	float centeredZ = WorldToHeightmapZ(worldZ);

	// Paint in a circular brush pattern
	for (int z = 0; z < terrainDepth; z++)
	{
		for (int x = 0; x < terrainWidth; x++)
		{
			float dx = (float)(x - centeredX);
			float dz = (float)(z - centeredZ);
			float dist = sqrtf(dx * dx + dz * dz);

			if (dist < radius)
			{
				// Smooth falloff using cosine
				float falloff = 0.5f * (cosf(dist / radius * 3.14159f) + 1.0f);
				float paintStrength = strength * falloff * deltaTime;

				// Increase weight of the target layer
				terrainLayers[layerIndex].splatmap[z][x] = std::min(1.0f,
					terrainLayers[layerIndex].splatmap[z][x] + paintStrength);

				// Decrease weight of other layers proportionally
				float remainingWeight = 1.0f - terrainLayers[layerIndex].splatmap[z][x];
				float otherLayersTotal = 0.0f;

				for (int i = 0; i < GetLayerCount(); i++)
				{
					if (i != layerIndex)
						otherLayersTotal += terrainLayers[i].splatmap[z][x];
				}

				if (otherLayersTotal > 0.0f)
				{
					for (int i = 0; i < GetLayerCount(); i++)
					{
						if (i != layerIndex)
						{
							float ratio = terrainLayers[i].splatmap[z][x] / otherLayersTotal;
							terrainLayers[i].splatmap[z][x] = remainingWeight * ratio;
						}
					}
				}

				// Ensure normalization
				NormalizeSplatmaps(x, z);
			}
		}
	}

	needsSplatmapUpdate = true;
}

void Terrain::UpdateSplatmapTexture()
{
	if (terrainLayers.empty())
		return;

	// Create or update splatmap texture
	// For up to 4 layers, we can pack them into RGBA channels
	int layerCount = std::min(4, GetLayerCount());

	if (!splatmapGenerated)
	{
		// Create new texture
		RaylibWrapper::Image splatImage = RaylibWrapper::GenImageColor(terrainWidth, terrainDepth, { 255, 0, 0, 0 });

		RaylibWrapper::Color* pixels = RaylibWrapper::LoadImageColors(splatImage);

		for (int z = 0; z < terrainDepth; z++)
		{
			for (int x = 0; x < terrainWidth; x++)
			{
				int index = z * terrainWidth + x;

				pixels[index].r = (layerCount > 0) ? (unsigned char)(terrainLayers[0].splatmap[z][x] * 255) : 255;
				pixels[index].g = (layerCount > 1) ? (unsigned char)(terrainLayers[1].splatmap[z][x] * 255) : 0;
				pixels[index].b = (layerCount > 2) ? (unsigned char)(terrainLayers[2].splatmap[z][x] * 255) : 0;
				pixels[index].a = (layerCount > 3) ? (unsigned char)(terrainLayers[3].splatmap[z][x] * 255) : 0;
			}
		}

		RaylibWrapper::UnloadImageColors(pixels);
		splatmapTexture = RaylibWrapper::LoadTextureFromImage(splatImage);
		RaylibWrapper::UnloadImage(splatImage);
		splatmapGenerated = true;
	}
	else
	{
		// Update existing texture
		RaylibWrapper::Image splatImage = RaylibWrapper::GenImageColor(terrainWidth, terrainDepth, { 255, 0, 0, 0 });
		RaylibWrapper::Color* pixels = RaylibWrapper::LoadImageColors(splatImage);

		for (int z = 0; z < terrainDepth; z++)
		{
			for (int x = 0; x < terrainWidth; x++)
			{
				int index = z * terrainWidth + x;

				pixels[index].r = (layerCount > 0) ? (unsigned char)(terrainLayers[0].splatmap[z][x] * 255) : 255;
				pixels[index].g = (layerCount > 1) ? (unsigned char)(terrainLayers[1].splatmap[z][x] * 255) : 0;
				pixels[index].b = (layerCount > 2) ? (unsigned char)(terrainLayers[2].splatmap[z][x] * 255) : 0;
				pixels[index].a = (layerCount > 3) ? (unsigned char)(terrainLayers[3].splatmap[z][x] * 255) : 0;
			}
		}

		RaylibWrapper::UpdateTexture(splatmapTexture, pixels);
		RaylibWrapper::UnloadImageColors(pixels);
		RaylibWrapper::UnloadImage(splatImage);
	}

	// Apply the terrain shader to the model
	if (modelGenerated)
	{
		// Set the shader on the terrain model's material
		raylibModel.SetShader(0, ShaderManager::Terrain); // Todo: this shouldn't be needed as the shader is already set in Awake()

		// Assign splatmap texture to MATERIAL_MAP_OCCLUSION
		raylibModel.SetMaterialMap(0, RaylibWrapper::MATERIAL_MAP_OCCLUSION, splatmapTexture, { 255, 255, 255, 255 }, 1);

		// Assign base textures for first 4 layers
		if (terrainLayers.size() > 0 && terrainLayers[0].material)
			raylibModel.SetMaterialMap(0, RaylibWrapper::MATERIAL_MAP_ALBEDO, *terrainLayers[0].material->GetAlbedoSprite()->GetTexture(), { 255, 255, 255, 255 }, 1);

		if (terrainLayers.size() > 1 && terrainLayers[1].material)
			raylibModel.SetMaterialMap(0, RaylibWrapper::MATERIAL_MAP_METALNESS, *terrainLayers[1].material->GetAlbedoSprite()->GetTexture(), { 255, 255, 255, 255 }, 1);

		if (terrainLayers.size() > 2 && terrainLayers[2].material)
			raylibModel.SetMaterialMap(0, RaylibWrapper::MATERIAL_MAP_ROUGHNESS, *terrainLayers[2].material->GetAlbedoSprite()->GetTexture(), { 255, 255, 255, 255 }, 1);

		if (terrainLayers.size() > 3 && terrainLayers[3].material)
			raylibModel.SetMaterialMap(0, RaylibWrapper::MATERIAL_MAP_EMISSION, *terrainLayers[3].material->GetAlbedoSprite()->GetTexture(), { 255, 255, 255, 255 }, 1);

		// Assign splatmap unit (5)
		int splatmapUnit = 5;
		RaylibWrapper::SetShaderValue(
			{ terrainShader.first, terrainShader.second },
			splatmapLoc,
			&splatmapUnit,
			RaylibWrapper::SHADER_UNIFORM_INT
		);

		// Update layer texture uniform locations
		UpdateShaderTextures();
	}
}

void Terrain::AddTerrainLayer(const std::string& materialPath, const std::string& name) {
	TerrainLayer newLayer;
	newLayer.material = Material::GetMaterial(materialPath);
	newLayer.name = name;
	newLayer.splatmap.resize(terrainDepth, std::vector<float>(terrainWidth, 0.0f));
	terrainLayers.push_back(newLayer);

	// Renormalize splatmap weights so no pixel becomes all-zero
	for (int z = 0; z < terrainDepth; z++) {
		for (int x = 0; x < terrainWidth; x++) {
			NormalizeSplatmaps(x, z);
			// If after normalization total is still 0 (all layers were zero), make first layer full
			float total = 0.0f;
			for (auto& layer : terrainLayers) total += layer.splatmap[z][x];
			if (total <= 0.0f && !terrainLayers.empty()) terrainLayers[0].splatmap[z][x] = 1.0f;
		}
	}

	needsSplatmapUpdate = true;
}


void Terrain::RemoveTerrainLayer(int layerIndex)
{
	if (layerIndex < 0 || layerIndex >= GetLayerCount()) return;
	terrainLayers.erase(terrainLayers.begin() + layerIndex);

	// Renormalize all splatmaps after removing a layer
	for (int z = 0; z < terrainDepth; z++)
	{
		for (int x = 0; x < terrainWidth; x++)
		{
			NormalizeSplatmaps(x, z);
		}
	}

	needsSplatmapUpdate = true;
}

TerrainLayer* Terrain::GetLayer(int index)
{
	if (index < 0 || index >= GetLayerCount()) return nullptr;
	return &terrainLayers[index];
}

const TerrainLayer* Terrain::GetLayer(int index) const
{
	if (index < 0 || index >= GetLayerCount()) return nullptr;
	return &terrainLayers[index];
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

nlohmann::json Terrain::SerializeLayerData()
{
	nlohmann::json layersJson;

	for (size_t i = 0; i < terrainLayers.size(); i++)
	{
		nlohmann::json layerJson;
		layerJson["name"] = terrainLayers[i].name;

		// Store material path if available
		if (terrainLayers[i].material)
			layerJson["materialPath"] = terrainLayers[i].material->GetPath();
		else
			layerJson["materialPath"] = "nullptr";

		// Store splatmap data
		layerJson["splatmap"] = terrainLayers[i].splatmap;

		layersJson.push_back(layerJson);
	}

	return layersJson;
}

void Terrain::LoadLayerData(const nlohmann::json& data)
{
	if (!data.is_array())
	{
		ConsoleLogger::ErrorLog(gameObject->GetName() + "'s Terrain failed to load layer data.");
		return;
	}

	terrainLayers.clear();

	for (auto& layerJson : data)
	{
		TerrainLayer layer;

		if (layerJson.contains("name"))
			layer.name = layerJson["name"].get<std::string>();

		if (layerJson.contains("materialPath"))
		{
			std::string matPath = layerJson["materialPath"].get<std::string>();
			if (matPath != "nullptr")
				layer.material = Material::GetMaterial(matPath);
		}

		if (layerJson.contains("splatmap") && layerJson["splatmap"].is_array())
		{
			layer.splatmap.clear();
			for (auto& row : layerJson["splatmap"])
				layer.splatmap.push_back(row.get<std::vector<float>>());
		}
		else
		{
			// Initialize empty splatmap if not provided
			layer.splatmap.resize(terrainDepth, std::vector<float>(terrainWidth, 0.0f));
		}

		terrainLayers.push_back(layer);
	}

	needsSplatmapUpdate = true;
}