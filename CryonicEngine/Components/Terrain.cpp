#include "Terrain.h"
#include "../RaylibWrapper.h"
#include <algorithm>
#include <random>

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
	if (!modelGenerated)
		return;

	if (renderShadows && !castShadows)
	{
		RenderTerrainMeshs(true); // Renders the meshs' shadows even if the Terrain doesn't cast them
		return;
	}

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

	raylibModel.DrawModelWrapper(centeredPos.x, centeredPos.y, centeredPos.z,
		scale.x, scale.y, scale.z,
		rot.x, rot.y, rot.z, rot.w,
		255, 255, 255, 255);

	// Render Meshs (painted meshes)
	RenderTerrainMeshs(renderShadows);
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

	// Clean up meshs
	for (TerrainMesh& mesh : terrainMeshs)
	{
		if (mesh.cachedModel)
		{
			// Check if this is the last reference to this model
			bool isLastReference = true;
			for (TerrainMesh& otherMesh : terrainMeshs)
			{
				if (&otherMesh != &mesh &&
					otherMesh.cachedModel == mesh.cachedModel &&
					otherMesh.modelPath == mesh.modelPath)
				{
					isLastReference = false;
					break;
				}
			}

			if (isLastReference)
			{
				mesh.cachedModel->DeleteInstance();
				delete mesh.cachedModel;
			}

			mesh.cachedModel = nullptr;
		}

		mesh.cachedMaterial = nullptr; // Don't delete, managed by Material system
	}

	terrainMeshs.clear();

	// Clean up auto mesh rules
	for (AutoMeshRule& rule : autoMeshRules)
	{
		for (auto& cached : rule.cachedVariants)
		{
			if (cached.model)
			{
				// Check if this is the last reference
				bool isLastReference = true;
				for (AutoMeshRule& otherRule : autoMeshRules)
				{
					if (&otherRule == &rule) continue;

					for (const auto& otherCached : otherRule.cachedVariants)
					{
						if (otherCached.model == cached.model)
						{
							isLastReference = false;
							break;
						}
					}
					if (!isLastReference) break;
				}

				if (isLastReference)
				{
					cached.model->DeleteInstance();
					delete cached.model;
				}

				cached.model = nullptr;
			}
			cached.material = nullptr;
		}
		rule.cachedVariants.clear();
	}

	autoMeshRules.clear();
	autoGeneratedMeshIndices.clear();
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

// ============= Mesh Painting Implementation =============

void Terrain::PaintMeshes(float worldX, float worldZ, float radius, float density, int terrainMeshIndex,
	float scaleVariation, float rotationRandomness, bool alignToNormal, float deltaTime)
{
	if (terrainMeshIndex < 0 || terrainMeshIndex >= GetTerrainMeshCount())
		return;

	TerrainMesh& terrainMesh = terrainMeshs[terrainMeshIndex];

	// Random number generation
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	// Calculate number of meshes to attempt to place based on density and deltaTime
	static float paintAccumulator = 0.0f;
	paintAccumulator += density * deltaTime * 10.0f;
	int meshesToPlace = static_cast<int>(paintAccumulator);
	paintAccumulator -= meshesToPlace;

	// Convert world coordinates to heightmap coordinates
	float centeredX = WorldToHeightmapX(worldX);
	float centeredZ = WorldToHeightmapZ(worldZ);

	for (int i = 0; i < meshesToPlace; i++)
	{
		// Random position within brush radius
		float angle = dist(gen) * 2.0f * 3.14159f;
		float distance = sqrtf(dist(gen)) * radius; // Square root for uniform distribution

		float offsetX = cosf(angle) * distance;
		float offsetZ = sinf(angle) * distance;

		float placeX = centeredX + offsetX;
		float placeZ = centeredZ + offsetZ;

		// Check if within terrain bounds
		if (placeX < 0 || placeX >= terrainWidth || placeZ < 0 || placeZ >= terrainDepth)
			continue;

		// Check if too close to existing instances (density control)
		bool tooClose = false;
		float minDistance = 2.0f / density; // Closer spacing for higher density

		for (const MeshInstance& existing : terrainMesh.instances)
		{
			float dx = existing.position.x - (worldX + offsetX);
			float dz = existing.position.z - (worldZ + offsetZ);
			float distToExisting = sqrtf(dx * dx + dz * dz);

			if (distToExisting < minDistance)
			{
				tooClose = true;
				break;
			}
		}

		if (tooClose)
			continue;

		// Create new mesh instance
		MeshInstance instance;

		// Position on terrain surface
		float terrainY = GetHeight((int)placeX, (int)placeZ);
		instance.position = { worldX + offsetX, terrainY, worldZ + offsetZ };

		// Scale with variation
		float baseScale = 1.0f;
		float scaleRandom = 1.0f + (dist(gen) - 0.5f) * 2.0f * scaleVariation;
		instance.scale = { baseScale * scaleRandom, baseScale * scaleRandom, baseScale * scaleRandom };

		// Rotation
		if (alignToNormal)
		{
			// Align to terrain normal
			Vector3 normal = GetNormalAtWorldPosition(placeX, placeZ);

			// Calculate rotation from up vector to normal
			Vector3 up = { 0.0f, 1.0f, 0.0f };
			Vector3 axis = {
				up.y * normal.z - up.z * normal.y,
				up.z * normal.x - up.x * normal.z,
				up.x * normal.y - up.y * normal.x
			};

			float axisLength = sqrtf(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
			if (axisLength > 0.001f)
			{
				axis.x /= axisLength;
				axis.y /= axisLength;
				axis.z /= axisLength;

				float angle = acosf(up.x * normal.x + up.y * normal.y + up.z * normal.z);
				instance.rotation.x = axis.x * angle;
				instance.rotation.y = axis.y * angle;
				instance.rotation.z = axis.z * angle;
			}
			else
				instance.rotation = { 0.0f, 0.0f, 0.0f };

			// Add random Y rotation
			instance.rotation.y += dist(gen) * 2.0f * 3.14159f * rotationRandomness;
		}
		else
		{
			// Random rotation around Y axis
			instance.rotation.x = 0.0f;
			instance.rotation.y = dist(gen) * 2.0f * 3.14159f;
			instance.rotation.z = 0.0f;
		}

		// Add instance
		terrainMesh.instances.push_back(instance);
	}
}

void Terrain::EraseMeshes(float worldX, float worldZ, float radius, int terrainMeshIndex)
{
	if (terrainMeshIndex < 0 || terrainMeshIndex >= GetTerrainMeshCount())
		return;

	TerrainMesh& terrainMesh = terrainMeshs[terrainMeshIndex];

	// Remove instances within radius
	terrainMesh.instances.erase(
		std::remove_if(terrainMesh.instances.begin(), terrainMesh.instances.end(),
			[worldX, worldZ, radius](const MeshInstance& instance) {
				float dx = instance.position.x - worldX;
				float dz = instance.position.z - worldZ;
				float dist = sqrtf(dx * dx + dz * dz);
				return dist < radius;
			}),
		terrainMesh.instances.end()
	);
}

void Terrain::AddTerrainMesh(const std::string& modelPath, const std::string& materialPath, const std::string& name)
{
	TerrainMesh newMesh;
	newMesh.name = name;
	newMesh.modelPath = modelPath;
	newMesh.materialPath = materialPath;

	// Determine model type from extension
	std::string ext = std::filesystem::path(modelPath).extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext == ".obj" || ext == ".gltf" || ext == ".glb")
		newMesh.modelType = Custom;
	else
		newMesh.modelType = Custom;

	terrainMeshs.push_back(newMesh);

	// Load the model and cache it
	LoadTerrainMeshModels();
}

void Terrain::RemoveTerrainMesh(int layerIndex)
{
	if (layerIndex < 0 || layerIndex >= GetTerrainMeshCount())
		return;

	// Clean up cached model if it's the only layer using it
	TerrainMesh& layer = terrainMeshs[layerIndex];
	if (layer.cachedModel)
	{
		bool isUsedByOthers = false;
		for (int i = 0; i < GetTerrainMeshCount(); i++)
		{
			if (i != layerIndex && terrainMeshs[i].modelPath == layer.modelPath)
			{
				isUsedByOthers = true;
				break;
			}
		}

		if (!isUsedByOthers)
		{
			layer.cachedModel->DeleteInstance();
			delete layer.cachedModel;
		}
	}

	terrainMeshs.erase(terrainMeshs.begin() + layerIndex);
}

TerrainMesh* Terrain::GetTerrainMesh(int index)
{
	if (index < 0 || index >= GetTerrainMeshCount())
		return nullptr;
	return &terrainMeshs[index];
}

const TerrainMesh* Terrain::GetTerrainMesh(int index) const
{
	if (index < 0 || index >= GetTerrainMeshCount())
		return nullptr;
	return &terrainMeshs[index];
}

void Terrain::LoadTerrainMeshModels()
{
	for (TerrainMesh& layer : terrainMeshs)
	{
		// Check if model is already cached
		if (layer.cachedModel)
			continue;

		// Check if another layer already loaded this model
		bool foundShared = false;
		for (TerrainMesh& otherLayer : terrainMeshs)
		{
			if (&otherLayer != &layer && otherLayer.modelPath == layer.modelPath && otherLayer.cachedModel)
			{
				layer.cachedModel = otherLayer.cachedModel;
				foundShared = true;
				break;
			}
		}

		if (foundShared)
			continue;

		// Load new model
		layer.cachedModel = new RaylibModel();

#if defined(EDITOR)
		layer.cachedModel->Create(layer.modelType, layer.modelPath, ShaderManager::Shaders::LitStandard,
			ProjectManager::projectData.path / "Assets");
#else
		std::filesystem::path assetsPath = exeParent.empty() ? "Resources/Assets" :
			std::filesystem::path(exeParent) / "Resources" / "Assets";
		layer.cachedModel->Create(layer.modelType, layer.modelPath, ShaderManager::Shaders::LitStandard, assetsPath);
#endif

		// Load material if specified
		if (!layer.materialPath.empty())
		{
			layer.cachedMaterial = Material::GetMaterial(layer.materialPath);
			if (layer.cachedMaterial)
				layer.cachedModel->SetMaterials({ layer.cachedMaterial->GetRaylibMaterial() });
		}
		else
		{
			// Use embedded materials
			layer.cachedModel->SetEmbeddedMaterials();
		}
	}
}

void Terrain::RenderTerrainMeshs(bool renderShadows)
{
	for (TerrainMesh& layer : terrainMeshs)
	{
		if (!layer.cachedModel || layer.instances.empty())
			continue;

		// Render all instances of this Mesh
		for (const MeshInstance& instance : layer.instances)
		{
			// Convert euler angles to quaternion for rendering
			Quaternion quat = EulerToQuaternion(instance.rotation.x * DEG2RAD, instance.rotation.y * DEG2RAD, instance.rotation.z * DEG2RAD);

			// Set material if needed
			if (layer.cachedMaterial && !layer.cachedModel->CompareMaterials({ layer.cachedMaterial->GetID() }))
				layer.cachedModel->SetMaterials({ layer.cachedMaterial->GetRaylibMaterial() });
			else if (!layer.cachedMaterial && layer.cachedModel->GetMaterialID(0) != 1)
				layer.cachedModel->SetEmbeddedMaterials();

			// Draw the instance
			layer.cachedModel->DrawModelWrapper(
				instance.position.x, instance.position.y, instance.position.z,
				instance.scale.x, instance.scale.y, instance.scale.z,
				quat.x, quat.y, quat.z, quat.w,
				255, 255, 255, 255
			);
		}
	}
}

nlohmann::json Terrain::SerializeTerrainMeshData()
{
	nlohmann::json terrainMeshsJson;

	for (const TerrainMesh& terrainMesh : terrainMeshs)
	{
		nlohmann::json layerJson;
		layerJson["name"] = terrainMesh.name;
		layerJson["modelPath"] = terrainMesh.modelPath;
		layerJson["materialPath"] = terrainMesh.materialPath;
		layerJson["modelType"] = static_cast<int>(terrainMesh.modelType);

		nlohmann::json instancesJson;
		for (const MeshInstance& instance : terrainMesh.instances)
		{
			nlohmann::json instanceJson;
			instanceJson["position"] = { instance.position.x, instance.position.y, instance.position.z };
			instanceJson["rotation"] = { instance.rotation.x, instance.rotation.y, instance.rotation.z };
			instanceJson["scale"] = { instance.scale.x, instance.scale.y, instance.scale.z };
			instancesJson.push_back(instanceJson);
		}
		layerJson["instances"] = instancesJson;

		terrainMeshsJson.push_back(layerJson);
	}

	return terrainMeshsJson;
}

void Terrain::LoadTerrainMeshData(const nlohmann::json& data)
{
	if (!data.is_array())
	{
		ConsoleLogger::ErrorLog(gameObject->GetName() + "'s Terrain failed to load Mesh data.");
		return;
	}

	terrainMeshs.clear();

	for (const nlohmann::json& layerJson : data)
	{
		TerrainMesh terrainMesh;

		if (layerJson.contains("name"))
			terrainMesh.name = layerJson["name"].get<std::string>();

		if (layerJson.contains("modelPath"))
			terrainMesh.modelPath = layerJson["modelPath"].get<std::string>();

		if (layerJson.contains("materialPath"))
			terrainMesh.materialPath = layerJson["materialPath"].get<std::string>();

		if (layerJson.contains("modelType"))
			terrainMesh.modelType = static_cast<ModelType>(layerJson["modelType"].get<int>());

		if (layerJson.contains("instances") && layerJson["instances"].is_array())
		{
			for (const nlohmann::json& instanceJson : layerJson["instances"])
			{
				MeshInstance instance;

				if (instanceJson.contains("position") && instanceJson["position"].is_array())
				{
					auto pos = instanceJson["position"];
					instance.position = { pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };
				}

				if (instanceJson.contains("rotation") && instanceJson["rotation"].is_array())
				{
					auto rot = instanceJson["rotation"];
					instance.rotation = { rot[0].get<float>(), rot[1].get<float>(), rot[2].get<float>() };
				}

				if (instanceJson.contains("scale") && instanceJson["scale"].is_array())
				{
					auto scl = instanceJson["scale"];
					instance.scale = { scl[0].get<float>(), scl[1].get<float>(), scl[2].get<float>() };
				}

				terrainMesh.instances.push_back(instance);
			}
		}

		terrainMeshs.push_back(terrainMesh);
	}

	// Load all Mesh models
	LoadTerrainMeshModels();
}

// ============= Auto Mesh Placement Implementation =============

void Terrain::AddAutoMeshRule(const std::string& name, int targetLayerIndex)
{
	if (targetLayerIndex < 0 || targetLayerIndex >= GetLayerCount())
		return;

	AutoMeshRule rule;
	rule.name = name;
	rule.targetLayerIndex = targetLayerIndex;
	rule.maxHeight = terrainHeight; // Set to current terrain max height

	autoMeshRules.push_back(rule);
}

void Terrain::RemoveAutoMeshRule(int ruleIndex)
{
	if (ruleIndex < 0 || ruleIndex >= GetAutoMeshRuleCount())
		return;

	// Clean up cached models for this rule
	AutoMeshRule& rule = autoMeshRules[ruleIndex];
	for (auto& cached : rule.cachedVariants)
	{
		if (cached.model)
		{
			// Check if this model is used by other rules
			bool usedByOthers = false;
			for (int i = 0; i < GetAutoMeshRuleCount(); i++)
			{
				if (i == ruleIndex) continue;

				for (const auto& otherCached : autoMeshRules[i].cachedVariants)
				{
					if (otherCached.model == cached.model)
					{
						usedByOthers = true;
						break;
					}
				}
				if (usedByOthers) break;
			}

			if (!usedByOthers)
			{
				cached.model->DeleteInstance();
				delete cached.model;
			}
		}
	}

	// Clear auto-generated meshes for this rule
	if (autoGeneratedMeshIndices.count(ruleIndex) > 0)
	{
		// Note: We don't actually remove the instances here, just clear the tracking
		// Call RegenerateAutoMeshes() if you want to physically remove them
		autoGeneratedMeshIndices.erase(ruleIndex);
	}

	autoMeshRules.erase(autoMeshRules.begin() + ruleIndex);

	// Reindex the autoGeneratedMeshIndices map
	std::map<int, std::vector<int>> newMap;
	for (auto& pair : autoGeneratedMeshIndices)
	{
		int idx = pair.first;
		if (idx > ruleIndex)
			newMap[idx - 1] = pair.second;
		else if (idx < ruleIndex)
			newMap[idx] = pair.second;
	}
	autoGeneratedMeshIndices = newMap;
}

AutoMeshRule* Terrain::GetAutoMeshRule(int index)
{
	if (index < 0 || index >= GetAutoMeshRuleCount())
		return nullptr;
	return &autoMeshRules[index];
}

const AutoMeshRule* Terrain::GetAutoMeshRule(int index) const
{
	if (index < 0 || index >= GetAutoMeshRuleCount())
		return nullptr;
	return &autoMeshRules[index];
}

void Terrain::GenerateAutoMeshes()
{
	for (int i = 0; i < GetAutoMeshRuleCount(); i++)
	{
		AutoMeshRule& rule = autoMeshRules[i];
		if (rule.targetLayerIndex < 0 || rule.targetLayerIndex >= GetLayerCount())
			continue;

		if (rule.meshVariants.empty())
			continue;

		// Load models if not already cached
		LoadAutoMeshRuleModels(rule);

		// Generate meshes for this rule
		GenerateAutoMeshesForRule(rule);
	}
}

void Terrain::RegenerateAutoMeshes()
{
	ClearAutoGeneratedMeshes();
	GenerateAutoMeshes();
}

void Terrain::ClearAutoGeneratedMeshes()
{
	// For each rule, find the corresponding terrain mesh and clear instances
	for (auto& pair : autoGeneratedMeshIndices)
	{
		int ruleIndex = pair.first;
		if (ruleIndex < 0 || ruleIndex >= GetAutoMeshRuleCount())
			continue;

		AutoMeshRule& rule = autoMeshRules[ruleIndex];

		// Find all terrain meshes that were auto-generated
		// We'll clear all instances from meshes that match our variants
		for (const auto& variant : rule.meshVariants)
		{
			for (int i = 0; i < GetTerrainMeshCount(); i++)
			{
				TerrainMesh* mesh = GetTerrainMesh(i);
				if (mesh && mesh->modelPath == variant.modelPath)
				{
					mesh->instances.clear();
				}
			}
		}
	}

	autoGeneratedMeshIndices.clear();
}

void Terrain::LoadAutoMeshRuleModels(AutoMeshRule& rule)
{
	if (!rule.cachedVariants.empty())
		return; // Already loaded

	for (const MeshVariant& variant : rule.meshVariants)
	{
		AutoMeshRule::CachedVariant cached;
		cached.weight = variant.weight;

		// Check if we already have this model cached in another rule
		bool foundShared = false;
		for (AutoMeshRule& otherRule : autoMeshRules)
		{
			if (&otherRule == &rule) continue;

			for (const auto& otherCached : otherRule.cachedVariants)
			{
				if (otherCached.model &&
					!rule.meshVariants.empty() &&
					variant.modelPath == rule.meshVariants[0].modelPath)
				{
					cached.model = otherCached.model;
					cached.material = otherCached.material;
					foundShared = true;
					break;
				}
			}
			if (foundShared) break;
		}

		if (!foundShared)
		{
			// Load new model
			cached.model = new RaylibModel();

			ModelType modelType = Custom;
			std::string ext = std::filesystem::path(variant.modelPath).extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

#if defined(EDITOR)
			cached.model->Create(modelType, variant.modelPath, ShaderManager::Shaders::LitStandard,
				ProjectManager::projectData.path / "Assets");
#else
			std::filesystem::path assetsPath = exeParent.empty() ? "Resources/Assets" :
				std::filesystem::path(exeParent) / "Resources" / "Assets";
			cached.model->Create(modelType, variant.modelPath, ShaderManager::Shaders::LitStandard, assetsPath);
#endif

			// Load material if specified
			if (!variant.materialPath.empty())
			{
				cached.material = Material::GetMaterial(variant.materialPath);
				if (cached.material)
					cached.model->SetMaterials({ cached.material->GetRaylibMaterial() });
			}
			else
			{
				cached.model->SetEmbeddedMaterials();
			}
		}

		rule.cachedVariants.push_back(cached);
	}
}

MeshVariant* Terrain::SelectWeightedVariant(AutoMeshRule& rule)
{
	if (rule.meshVariants.empty())
		return nullptr;

	// Calculate total weight
	float totalWeight = 0.0f;
	for (const MeshVariant& variant : rule.meshVariants)
		totalWeight += variant.weight;

	if (totalWeight <= 0.0f)
		return &rule.meshVariants[0];

	// Random selection based on weight
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0.0f, totalWeight);

	float randomValue = dist(gen);
	float cumulativeWeight = 0.0f;

	for (MeshVariant& variant : rule.meshVariants)
	{
		cumulativeWeight += variant.weight;
		if (randomValue <= cumulativeWeight)
			return &variant;
	}

	return &rule.meshVariants[rule.meshVariants.size() - 1];
}

bool Terrain::CheckPlacementConstraints(const AutoMeshRule& rule, int x, int z, float terrainHeight, const Vector3& normal) const
{
	// Check height constraints
	if (terrainHeight < rule.minHeight || terrainHeight > rule.maxHeight)
		return false;

	// Check slope constraints
	float slope = acosf(normal.y) * RAD2DEG;
	if (slope < rule.minSlope || slope > rule.maxSlope)
		return false;

	// Check texture weight
	if (rule.targetLayerIndex >= 0 && rule.targetLayerIndex < GetLayerCount())
	{
		const TerrainLayer* layer = GetLayer(rule.targetLayerIndex);
		if (layer && z < (int)layer->splatmap.size() && x < (int)layer->splatmap[z].size())
		{
			float weight = layer->splatmap[z][x];
			if (weight < rule.textureWeightThreshold)
				return false;
		}
		else
			return false;
	}

	return true;
}

void Terrain::GenerateAutoMeshesForRule(AutoMeshRule& rule)
{
	if (rule.meshVariants.empty() || rule.cachedVariants.empty())
		return;

	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	// Calculate cell size based on density
	float cellSize = 1.0f / sqrtf(rule.density);
	int cellsX = (int)(terrainWidth / cellSize);
	int cellsZ = (int)(terrainDepth / cellSize);

	// Track which terrain mesh indices we're adding to
	std::vector<int> usedMeshIndices;

	// For each cell, attempt to place a mesh
	for (int cz = 0; cz < cellsZ; cz++)
	{
		for (int cx = 0; cx < cellsX; cx++)
		{
			// Calculate cell center
			float baseCenterX = cx * cellSize + cellSize * 0.5f;
			float baseCenterZ = cz * cellSize + cellSize * 0.5f;

			// Add random offset
			float offsetX = (dist(gen) - 0.5f) * cellSize * rule.randomOffset;
			float offsetZ = (dist(gen) - 0.5f) * cellSize * rule.randomOffset;

			float centerX = baseCenterX + offsetX;
			float centerZ = baseCenterZ + offsetZ;

			// Check bounds
			if (centerX < 0 || centerX >= terrainWidth || centerZ < 0 || centerZ >= terrainDepth)
				continue;

			int x = (int)centerX;
			int z = (int)centerZ;

			// Get terrain info at this position
			float height = GetHeight(x, z);
			Vector3 normal = GetNormalAtWorldPosition((float)x, (float)z);

			// Check placement constraints
			if (!CheckPlacementConstraints(rule, x, z, height, normal))
				continue;

			// Select a variant
			MeshVariant* selectedVariant = SelectWeightedVariant(rule);
			if (!selectedVariant)
				continue;

			// Find or create the terrain mesh for this variant
			int terrainMeshIndex = -1;
			for (int i = 0; i < GetTerrainMeshCount(); i++)
			{
				TerrainMesh* mesh = GetTerrainMesh(i);
				if (mesh && mesh->modelPath == selectedVariant->modelPath)
				{
					terrainMeshIndex = i;
					break;
				}
			}

			// If terrain mesh doesn't exist, create it
			if (terrainMeshIndex == -1)
			{
				AddTerrainMesh(selectedVariant->modelPath, selectedVariant->materialPath,
					std::filesystem::path(selectedVariant->modelPath).stem().string());
				terrainMeshIndex = GetTerrainMeshCount() - 1;
			}

			TerrainMesh* terrainMesh = GetTerrainMesh(terrainMeshIndex);
			if (!terrainMesh)
				continue;

			// Create mesh instance
			MeshInstance instance;

			// World position
			Vector3 terrainPos = gameObject->transform.GetPosition();
			instance.position = {
				terrainPos.x + centerX - (terrainWidth * 0.5f),
				terrainPos.y + height,
				terrainPos.z + centerZ - (terrainDepth * 0.5f)
			};

			// Scale with variation
			float baseScale = 1.0f;
			float scaleRandom = 1.0f + (dist(gen) - 0.5f) * 2.0f * rule.scaleVariation;
			instance.scale = { baseScale * scaleRandom, baseScale * scaleRandom, baseScale * scaleRandom };

			// Rotation
			if (rule.alignToNormal)
			{
				// Align to terrain normal
				Vector3 up = { 0.0f, 1.0f, 0.0f };
				Vector3 axis = {
					up.y * normal.z - up.z * normal.y,
					up.z * normal.x - up.x * normal.z,
					up.x * normal.y - up.y * normal.x
				};

				float axisLength = sqrtf(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
				if (axisLength > 0.001f)
				{
					axis.x /= axisLength;
					axis.y /= axisLength;
					axis.z /= axisLength;

					float angle = acosf(up.x * normal.x + up.y * normal.y + up.z * normal.z);
					instance.rotation.x = axis.x * angle;
					instance.rotation.y = axis.y * angle;
					instance.rotation.z = axis.z * angle;
				}
				else
					instance.rotation = { 0.0f, 0.0f, 0.0f };

				// Add random Y rotation
				instance.rotation.y += dist(gen) * 2.0f * 3.14159f * rule.rotationRandomness;
			}
			else
			{
				// Random rotation around Y axis
				instance.rotation.x = 0.0f;
				instance.rotation.y = dist(gen) * 2.0f * 3.14159f;
				instance.rotation.z = 0.0f;
			}

			// Add instance to terrain mesh
			terrainMesh->instances.push_back(instance);

			// Track this for the rule
			if (std::find(usedMeshIndices.begin(), usedMeshIndices.end(), terrainMeshIndex) == usedMeshIndices.end())
				usedMeshIndices.push_back(terrainMeshIndex);
		}
	}

	// Store which meshes were generated by this rule
	int ruleIndex = -1;
	for (int i = 0; i < GetAutoMeshRuleCount(); i++)
	{
		if (&autoMeshRules[i] == &rule)
		{
			ruleIndex = i;
			break;
		}
	}

	if (ruleIndex >= 0)
		autoGeneratedMeshIndices[ruleIndex] = usedMeshIndices;
}

nlohmann::json Terrain::SerializeAutoMeshRules()
{
	nlohmann::json rulesJson;

	for (const AutoMeshRule& rule : autoMeshRules)
	{
		nlohmann::json ruleJson;
		ruleJson["name"] = rule.name;
		ruleJson["targetLayerIndex"] = rule.targetLayerIndex;
		ruleJson["density"] = rule.density;
		ruleJson["minSlope"] = rule.minSlope;
		ruleJson["maxSlope"] = rule.maxSlope;
		ruleJson["minHeight"] = rule.minHeight;
		ruleJson["maxHeight"] = rule.maxHeight;
		ruleJson["textureWeightThreshold"] = rule.textureWeightThreshold;
		ruleJson["scaleVariation"] = rule.scaleVariation;
		ruleJson["rotationRandomness"] = rule.rotationRandomness;
		ruleJson["alignToNormal"] = rule.alignToNormal;
		ruleJson["randomOffset"] = rule.randomOffset;

		nlohmann::json variantsJson;
		for (const MeshVariant& variant : rule.meshVariants)
		{
			nlohmann::json variantJson;
			variantJson["modelPath"] = variant.modelPath;
			variantJson["materialPath"] = variant.materialPath;
			variantJson["weight"] = variant.weight;
			variantsJson.push_back(variantJson);
		}
		ruleJson["variants"] = variantsJson;

		rulesJson.push_back(ruleJson);
	}

	return rulesJson;
}

void Terrain::LoadAutoMeshRules(const nlohmann::json& data)
{
	if (!data.is_array())
	{
		ConsoleLogger::ErrorLog(gameObject->GetName() + "'s Terrain failed to load auto mesh rules.");
		return;
	}

	autoMeshRules.clear();
	autoGeneratedMeshIndices.clear();

	for (const nlohmann::json& ruleJson : data)
	{
		AutoMeshRule rule;

		if (ruleJson.contains("name"))
			rule.name = ruleJson["name"].get<std::string>();

		if (ruleJson.contains("targetLayerIndex"))
			rule.targetLayerIndex = ruleJson["targetLayerIndex"].get<int>();

		if (ruleJson.contains("density"))
			rule.density = ruleJson["density"].get<float>();

		if (ruleJson.contains("minSlope"))
			rule.minSlope = ruleJson["minSlope"].get<float>();

		if (ruleJson.contains("maxSlope"))
			rule.maxSlope = ruleJson["maxSlope"].get<float>();

		if (ruleJson.contains("minHeight"))
			rule.minHeight = ruleJson["minHeight"].get<float>();

		if (ruleJson.contains("maxHeight"))
			rule.maxHeight = ruleJson["maxHeight"].get<float>();

		if (ruleJson.contains("textureWeightThreshold"))
			rule.textureWeightThreshold = ruleJson["textureWeightThreshold"].get<float>();

		if (ruleJson.contains("scaleVariation"))
			rule.scaleVariation = ruleJson["scaleVariation"].get<float>();

		if (ruleJson.contains("rotationRandomness"))
			rule.rotationRandomness = ruleJson["rotationRandomness"].get<float>();

		if (ruleJson.contains("alignToNormal"))
			rule.alignToNormal = ruleJson["alignToNormal"].get<bool>();

		if (ruleJson.contains("randomOffset"))
			rule.randomOffset = ruleJson["randomOffset"].get<float>();

		if (ruleJson.contains("variants") && ruleJson["variants"].is_array())
		{
			for (const nlohmann::json& variantJson : ruleJson["variants"])
			{
				MeshVariant variant;

				if (variantJson.contains("modelPath"))
					variant.modelPath = variantJson["modelPath"].get<std::string>();

				if (variantJson.contains("materialPath"))
					variant.materialPath = variantJson["materialPath"].get<std::string>();

				if (variantJson.contains("weight"))
					variant.weight = variantJson["weight"].get<float>();

				rule.meshVariants.push_back(variant);
			}
		}

		autoMeshRules.push_back(rule);
	}

	// Load models for all rules
	for (AutoMeshRule& rule : autoMeshRules)
	{
		if (!rule.meshVariants.empty())
			LoadAutoMeshRuleModels(rule);
	}
}