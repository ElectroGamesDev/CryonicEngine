//#include "Terrain.h"
//#include "../RaylibWrapper.h"
//#include "rlgl.h"
//#include <cmath>
//#include <algorithm>
//#if defined (EDITOR)
//#include "../ProjectManager.h"
//#else
//#include "../Game.h"
//#endif
//
//void Terrain::Awake()
//{
//#if defined (EDITOR)
//	// Load settings from exposed variables if needed
//#endif
//
//	// Initialize splat maps for texture painting
//	splatMaps.resize(1); // Start with 1 base layer
//	splatMaps[0].resize(terrainWidth * terrainDepth, 1.0f); // Base layer fully visible
//}
//
//void Terrain::Start()
//{
//	if (!terrainGenerated)
//	{
//		GenerateTerrain();
//	}
//}
//
//void Terrain::Update()
//{
//	// Update LOD based on camera position if enabled
//	if (enableLOD && terrainGenerated)
//	{
//#if defined(EDITOR)
//		// Get camera from editor
//		// Vector3 cameraPos = EditorCamera::GetPosition();
//		// UpdateLOD(cameraPos);
//#else
//		// Get camera from game
//		Camera* mainCamera = Camera::GetMainCamera();
//		if (mainCamera)
//		{
//			Vector3 cameraPos = mainCamera->GetPosition();
//			UpdateLOD(cameraPos);
//		}
//#endif
//	}
//}
//
//void Terrain::Render(bool renderShadows)
//{
//	if (!terrainGenerated || (renderShadows && !castShadows))
//		return;
//
//	Vector3 position = gameObject->transform.GetPosition();
//	Quaternion rotation = gameObject->transform.GetRotation();
//	Vector3 scale = gameObject->transform.GetScale();
//
//	// Determine which model to render (LOD or main)
//	RaylibModel* modelToRender = &terrainModel;
//	if (enableLOD && currentLODLevel < lodMeshes.size() && currentLODLevel > 0)
//	{
//		modelToRender = &lodMeshes[currentLODLevel].model;
//	}
//
//	// Set material if needed
//	if ((!terrainMaterial && modelToRender->GetMaterialID(0) != 0) ||
//		(terrainMaterial && !modelToRender->CompareMaterials({ terrainMaterial->GetID() })))
//	{
//		if (!terrainMaterial)
//			modelToRender->SetMaterials({ &Material::defaultMaterial });
//		else
//			modelToRender->SetMaterials({ terrainMaterial->GetRaylibMaterial() });
//	}
//
//	modelToRender->DrawModelWrapper(position.x, position.y, position.z,
//		scale.x, scale.y, scale.z,
//		rotation.x, rotation.y, rotation.z, rotation.w,
//		255, 255, 255, 255);
//}
//
//#if defined(EDITOR)
//void Terrain::EditorUpdate()
//{
//	bool needsRegeneration = false;
//
//	// Check if terrain dimensions changed
//	if (exposedVariables[1][0][2].get<int>() != terrainWidth)
//	{
//		terrainWidth = exposedVariables[1][0][2].get<int>();
//		needsRegeneration = true;
//	}
//	if (exposedVariables[1][1][2].get<int>() != terrainDepth)
//	{
//		terrainDepth = exposedVariables[1][1][2].get<int>();
//		needsRegeneration = true;
//	}
//
//	// Update other properties
//	terrainHeight = exposedVariables[1][2][2].get<float>();
//	heightScale = exposedVariables[1][3][2].get<float>();
//
//	// Check heightmap changes
//	std::string newHeightmapPath = exposedVariables[1][4][2].get<std::string>();
//	if (newHeightmapPath != heightmapPath.string())
//	{
//		heightmapPath = newHeightmapPath;
//		if (!heightmapPath.empty())
//		{
//			ApplyHeightmapToTerrain(heightmapPath);
//			UpdateTerrainMesh();
//		}
//	}
//
//	// Update material
//	if (exposedVariables[1][5][2] == "nullptr")
//		exposedVariables[1][5][2] = "Default";
//
//	if ((!terrainMaterial && !(exposedVariables[1][5][2] == "Default" && defaultMaterial)) ||
//		(terrainMaterial && terrainMaterial->GetPath() != exposedVariables[1][5][2]))
//	{
//		if (exposedVariables[1][5][2] == "Default")
//		{
//			defaultMaterial = true;
//			SetMaterial(nullptr);
//		}
//		else
//			SetMaterial(Material::GetMaterial(exposedVariables[1][5][2]));
//	}
//
//	// Update LOD settings
//	enableLOD = exposedVariables[1][6][2].get<bool>();
//	int newLodLevels = exposedVariables[1][7][2].get<int>();
//	float newLodDistance = exposedVariables[1][8][2].get<float>();
//
//	if (newLodLevels != lodLevels || newLodDistance != lodDistance)
//	{
//		lodLevels = newLodLevels;
//		lodDistance = newLodDistance;
//		if (enableLOD && terrainGenerated)
//			GenerateLODMeshes();
//	}
//
//	castShadows = exposedVariables[1][9][2].get<bool>();
//
//	if (needsRegeneration)
//		RegenerateTerrain();
//}
//#endif
//
//void Terrain::Destroy()
//{
//	if (terrainGenerated)
//	{
//		terrainModel.DeleteInstance();
//
//		for (auto& lod : lodMeshes)
//		{
//			lod.model.DeleteInstance();
//		}
//		lodMeshes.clear();
//	}
//}
//
//// ============================================================================
//// TERRAIN GENERATION
//// ============================================================================
//
//void Terrain::GenerateTerrain()
//{
//	// Initialize height data
//	heightData.resize(terrainWidth * terrainDepth, 0.0f);
//	normals.resize(terrainWidth * terrainDepth);
//
//	// Load heightmap if specified
//	if (!heightmapPath.empty())
//	{
//		ApplyHeightmapToTerrain(heightmapPath);
//	}
//	else
//	{
//		GenerateFlatTerrain();
//	}
//
//	CalculateNormals();
//	UpdateTerrainMesh();
//
//	if (enableLOD)
//		GenerateLODMeshes();
//
//	terrainGenerated = true;
//}
//
//void Terrain::GenerateFlatTerrain()
//{
//	std::fill(heightData.begin(), heightData.end(), 0.0f);
//}
//
//void Terrain::LoadHeightmap(const std::filesystem::path& path)
//{
//	heightmapPath = path;
//	ApplyHeightmapToTerrain(path);
//	UpdateTerrainMesh();
//}
//
//void Terrain::ApplyHeightmapToTerrain(const std::filesystem::path& path)
//{
//#if defined(EDITOR)
//	std::filesystem::path fullPath = ProjectManager::projectData.path / "Assets" / path;
//#else
//	std::filesystem::path fullPath;
//	if (exeParent.empty())
//		fullPath = std::filesystem::path("Resources/Assets") / path;
//	else
//		fullPath = std::filesystem::path(exeParent) / "Resources" / "Assets" / path;
//#endif
//
//	// Load heightmap using Raylib
//	Image heightmap = RaylibWrapper::LoadImage(fullPath.string().c_str());
//
//	if (heightmap.data == nullptr)
//		return;
//
//	// Resize heightmap to match terrain dimensions if needed
//	if (heightmap.width != terrainWidth || heightmap.height != terrainDepth)
//	{
//		RaylibWrapper::ImageResize(&heightmap, terrainWidth, terrainDepth);
//	}
//
//	// Extract height data from image
//	Color* pixels = RaylibWrapper::LoadImageColors(heightmap);
//
//	for (int z = 0; z < terrainDepth; z++)
//	{
//		for (int x = 0; x < terrainWidth; x++)
//		{
//			int idx = z * terrainWidth + x;
//			Color pixel = pixels[idx];
//
//			// Use grayscale value (average RGB) as height
//			float height = (pixel.r + pixel.g + pixel.b) / (3.0f * 255.0f);
//			heightData[idx] = height * terrainHeight * heightScale;
//		}
//	}
//
//	RaylibWrapper::UnloadImageColors(pixels);
//	RaylibWrapper::UnloadImage(heightmap);
//
//	CalculateNormals();
//}
//
//// ============================================================================
//// HEIGHT MANIPULATION
//// ============================================================================
//
//void Terrain::SetHeight(int x, int z, float height)
//{
//	if (!IsValidTerrainPosition(x, z))
//		return;
//
//	int idx = z * terrainWidth + x;
//	heightData[idx] = std::clamp(height, 0.0f, terrainHeight);
//}
//
//float Terrain::GetHeight(int x, int z) const
//{
//	if (!IsValidTerrainPosition(x, z))
//		return 0.0f;
//
//	int idx = z * terrainWidth + x;
//	return heightData[idx];
//}
//
//float Terrain::GetHeightAtWorldPosition(float worldX, float worldZ) const
//{
//	Vector3 terrainPos = gameObject->transform.GetPosition();
//	Vector3 scale = gameObject->transform.GetScale();
//
//	// Convert world position to terrain local position
//	float localX = (worldX - terrainPos.x) / scale.x;
//	float localZ = (worldZ - terrainPos.z) / scale.z;
//
//	// Convert to terrain grid coordinates
//	float terrainX = localX;
//	float terrainZ = localZ;
//
//	if (terrainX < 0 || terrainX >= terrainWidth - 1 ||
//		terrainZ < 0 || terrainZ >= terrainDepth - 1)
//		return 0.0f;
//
//	// Bilinear interpolation
//	int x0 = static_cast<int>(terrainX);
//	int z0 = static_cast<int>(terrainZ);
//	int x1 = x0 + 1;
//	int z1 = z0 + 1;
//
//	float fx = terrainX - x0;
//	float fz = terrainZ - z0;
//
//	float h00 = GetHeight(x0, z0);
//	float h10 = GetHeight(x1, z0);
//	float h01 = GetHeight(x0, z1);
//	float h11 = GetHeight(x1, z1);
//
//	float h0 = h00 * (1 - fx) + h10 * fx;
//	float h1 = h01 * (1 - fx) + h11 * fx;
//
//	return (h0 * (1 - fz) + h1 * fz) * scale.y + terrainPos.y;
//}
//
//Vector3 Terrain::GetNormalAtWorldPosition(float worldX, float worldZ) const
//{
//	int terrainX, terrainZ;
//	WorldToTerrainCoords(worldX, worldZ, terrainX, terrainZ);
//
//	if (!IsValidTerrainPosition(terrainX, terrainZ))
//		return { 0.0f, 1.0f, 0.0f };
//
//	int idx = terrainZ * terrainWidth + terrainX;
//	return normals[idx];
//}
//
//// ============================================================================
//// SCULPTING FUNCTIONS
//// ============================================================================
//
//void Terrain::RaiseTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime)
//{
//	int centerX, centerZ;
//	WorldToTerrainCoords(worldX, worldZ, centerX, centerZ);
//
//	int radiusInt = static_cast<int>(radius);
//
//	for (int z = centerZ - radiusInt; z <= centerZ + radiusInt; z++)
//	{
//		for (int x = centerX - radiusInt; x <= centerX + radiusInt; x++)
//		{
//			if (!IsValidTerrainPosition(x, z))
//				continue;
//
//			float dx = static_cast<float>(x - centerX);
//			float dz = static_cast<float>(z - centerZ);
//			float distance = std::sqrt(dx * dx + dz * dz);
//
//			if (distance <= radius)
//			{
//				float weight = GetBrushWeight(distance, radius);
//				int idx = z * terrainWidth + x;
//				heightData[idx] += strength * weight * deltaTime;
//				heightData[idx] = std::clamp(heightData[idx], 0.0f, terrainHeight);
//			}
//		}
//	}
//
//	CalculateNormals();
//	UpdateTerrainMesh();
//}
//
//void Terrain::LowerTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime)
//{
//	RaiseTerrain(worldX, worldZ, radius, -strength, deltaTime);
//}
//
//void Terrain::SmoothTerrain(float worldX, float worldZ, float radius, float strength, float deltaTime)
//{
//	int centerX, centerZ;
//	WorldToTerrainCoords(worldX, worldZ, centerX, centerZ);
//
//	int radiusInt = static_cast<int>(radius);
//	std::vector<float> smoothedHeights = heightData; // Copy for reading
//
//	for (int z = centerZ - radiusInt; z <= centerZ + radiusInt; z++)
//	{
//		for (int x = centerX - radiusInt; x <= centerX + radiusInt; x++)
//		{
//			if (!IsValidTerrainPosition(x, z))
//				continue;
//
//			float dx = static_cast<float>(x - centerX);
//			float dz = static_cast<float>(z - centerZ);
//			float distance = std::sqrt(dx * dx + dz * dz);
//
//			if (distance <= radius)
//			{
//				// Calculate average height of neighbors
//				float avgHeight = 0.0f;
//				int count = 0;
//
//				for (int nz = z - 1; nz <= z + 1; nz++)
//				{
//					for (int nx = x - 1; nx <= x + 1; nx++)
//					{
//						if (IsValidTerrainPosition(nx, nz))
//						{
//							avgHeight += heightData[nz * terrainWidth + nx];
//							count++;
//						}
//					}
//				}
//
//				if (count > 0)
//				{
//					avgHeight /= count;
//					float weight = GetBrushWeight(distance, radius);
//					int idx = z * terrainWidth + x;
//					float currentHeight = heightData[idx];
//					smoothedHeights[idx] = currentHeight + (avgHeight - currentHeight) * weight * strength * deltaTime;
//				}
//			}
//		}
//	}
//
//	heightData = smoothedHeights;
//	CalculateNormals();
//	UpdateTerrainMesh();
//}
//
//void Terrain::FlattenTerrain(float worldX, float worldZ, float radius, float targetHeight, float strength, float deltaTime)
//{
//	int centerX, centerZ;
//	WorldToTerrainCoords(worldX, worldZ, centerX, centerZ);
//
//	int radiusInt = static_cast<int>(radius);
//
//	for (int z = centerZ - radiusInt; z <= centerZ + radiusInt; z++)
//	{
//		for (int x = centerX - radiusInt; x <= centerX + radiusInt; x++)
//		{
//			if (!IsValidTerrainPosition(x, z))
//				continue;
//
//			float dx = static_cast<float>(x - centerX);
//			float dz = static_cast<float>(z - centerZ);
//			float distance = std::sqrt(dx * dx + dz * dz);
//
//			if (distance <= radius)
//			{
//				float weight = GetBrushWeight(distance, radius);
//				int idx = z * terrainWidth + x;
//				float currentHeight = heightData[idx];
//				heightData[idx] = currentHeight + (targetHeight - currentHeight) * weight * strength * deltaTime;
//			}
//		}
//	}
//
//	CalculateNormals();
//	UpdateTerrainMesh();
//}
//
//float Terrain::GetBrushWeight(float distance, float radius) const
//{
//	if (distance >= radius)
//		return 0.0f;
//
//	// Smooth falloff using cosine interpolation
//	float t = distance / radius;
//	return (std::cos(t * 3.14159f) + 1.0f) * 0.5f;
//}
//
//void Terrain::WorldToTerrainCoords(float worldX, float worldZ, int& terrainX, int& terrainZ) const
//{
//	Vector3 terrainPos = gameObject->transform.GetPosition();
//	Vector3 scale = gameObject->transform.GetScale();
//
//	float localX = (worldX - terrainPos.x) / scale.x;
//	float localZ = (worldZ - terrainPos.z) / scale.z;
//
//	terrainX = static_cast<int>(localX);
//	terrainZ = static_cast<int>(localZ);
//}
//
//// ============================================================================
//// TEXTURE PAINTING
//// ============================================================================
//
//void Terrain::PaintTexture(float worldX, float worldZ, float radius, int layerIndex, float strength, float deltaTime)
//{
//	if (layerIndex < 0 || layerIndex >= splatMaps.size())
//		return;
//
//	int centerX, centerZ;
//	WorldToTerrainCoords(worldX, worldZ, centerX, centerZ);
//
//	int radiusInt = static_cast<int>(radius);
//
//	for (int z = centerZ - radiusInt; z <= centerZ + radiusInt; z++)
//	{
//		for (int x = centerX - radiusInt; x <= centerX + radiusInt; x++)
//		{
//			if (!IsValidTerrainPosition(x, z))
//				continue;
//
//			float dx = static_cast<float>(x - centerX);
//			float dz = static_cast<float>(z - centerZ);
//			float distance = std::sqrt(dx * dx + dz * dz);
//
//			if (distance <= radius)
//			{
//				float weight = GetBrushWeight(distance, radius);
//				int idx = z * terrainWidth + x;
//
//				// Increase weight for target layer
//				splatMaps[layerIndex][idx] += strength * weight * deltaTime;
//				splatMaps[layerIndex][idx] = std::clamp(splatMaps[layerIndex][idx], 0.0f, 1.0f);
//
//				// Normalize all layer weights at this position
//				NormalizeSplatWeights(x, z);
//			}
//		}
//	}
//
//	UpdateSplatMap();
//}
//
//void Terrain::AddTerrainLayer(Material* material, const std::string& name)
//{
//	TerrainLayer layer;
//	layer.material = material;
//	layer.name = name.empty() ? "Layer " + std::to_string(terrainLayers.size()) : name;
//	terrainLayers.push_back(layer);
//
//	// Add new splat map initialized to 0
//	splatMaps.push_back(std::vector<float>(terrainWidth * terrainDepth, 0.0f));
//}
//
//void Terrain::RemoveTerrainLayer(int layerIndex)
//{
//	if (layerIndex <= 0 || layerIndex >= terrainLayers.size())
//		return; // Can't remove base layer
//
//	terrainLayers.erase(terrainLayers.begin() + layerIndex);
//	splatMaps.erase(splatMaps.begin() + layerIndex);
//
//	// Renormalize all weights
//	for (int z = 0; z < terrainDepth; z++)
//	{
//		for (int x = 0; x < terrainWidth; x++)
//		{
//			NormalizeSplatWeights(x, z);
//		}
//	}
//
//	UpdateSplatMap();
//}
//
//void Terrain::NormalizeSplatWeights(int x, int z)
//{
//	int idx = z * terrainWidth + x;
//	float total = 0.0f;
//
//	for (const auto& splatMap : splatMaps)
//	{
//		total += splatMap[idx];
//	}
//
//	if (total > 0.0f)
//	{
//		for (auto& splatMap : splatMaps)
//		{
//			splatMap[idx] /= total;
//		}
//	}
//}
//
//void Terrain::UpdateSplatMap()
//{
//	// This would update shader uniforms or textures with splat map data
//	// Implementation depends on your material/shader system
//	// For now, this is a placeholder for future shader integration
//}
//
//// ============================================================================
//// MESH GENERATION AND UPDATE
//// ============================================================================
//
//void Terrain::UpdateTerrainMesh()
//{
//	std::vector<float> vertices;
//	std::vector<unsigned short> indices;
//	std::vector<float> texcoords;
//	std::vector<float> normalsData;
//
//	GenerateMeshData(vertices, indices, texcoords, normalsData, 0);
//
//	// Create mesh using rlgl
//	rlDisableBackfaceCulling();
//
//	unsigned int vaoId = 0;
//	unsigned int vboId[4] = { 0 }; // vertices, texcoords, normals, indices
//
//	// Generate VAO
//	vaoId = rlLoadVertexArray();
//	rlEnableVertexArray(vaoId);
//
//	// Vertices
//	vboId[0] = rlLoadVertexBuffer(vertices.data(), vertices.size() * sizeof(float), false);
//	rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
//	rlEnableVertexAttribute(0);
//
//	// Texcoords
//	vboId[1] = rlLoadVertexBuffer(texcoords.data(), texcoords.size() * sizeof(float), false);
//	rlSetVertexAttribute(1, 2, RL_FLOAT, 0, 0, 0);
//	rlEnableVertexAttribute(1);
//
//	// Normals
//	vboId[2] = rlLoadVertexBuffer(normalsData.data(), normalsData.size() * sizeof(float), false);
//	rlSetVertexAttribute(2, 3, RL_FLOAT, 0, 0, 0);
//	rlEnableVertexAttribute(2);
//
//	// Indices
//	vboId[3] = rlLoadVertexBufferElement(indices.data(), indices.size() * sizeof(unsigned short), false);
//
//	rlDisableVertexArray();
//
//	// Cleanup old mesh if it exists
//	if (terrainGenerated)
//	{
//		terrainModel.DeleteInstance();
//	}
//
//	// Create Raylib mesh structure
//	Mesh mesh = { 0 };
//	mesh.vertexCount = static_cast<int>(vertices.size() / 3);
//	mesh.triangleCount = static_cast<int>(indices.size() / 3);
//	mesh.vaoId = vaoId;
//	mesh.vboId[0] = vboId[0]; // vertices
//	mesh.vboId[1] = vboId[1]; // texcoords
//	mesh.vboId[2] = vboId[2]; // normals
//	mesh.vboId[6] = vboId[3]; // indices
//
//	// Create model from mesh
//	Model raylibModel = RaylibWrapper::LoadModelFromMesh(mesh);
//
//	// Wrap in RaylibModel
//#if defined(EDITOR)
//	terrainModel.Create(Custom, "", ShaderManager::Shaders::Default, ProjectManager::projectData.path / "Assets");
//#else
//	if (exeParent.empty())
//		terrainModel.Create(Custom, "", ShaderManager::Shaders::Default, "Resources/Assets");
//	else
//		terrainModel.Create(Custom, "", ShaderManager::Shaders::Default, std::filesystem::path(exeParent) / "Resources" / "Assets");
//#endif
//
//	// Manually set the model data
//	terrainModel.SetRaylibModel(raylibModel);
//	terrainModel.SetMaterials({ &Material::defaultMaterial });
//}
//
//void Terrain::GenerateMeshData(std::vector<float>& vertices, std::vector<unsigned short>& indices,
//	std::vector<float>& texcoords, std::vector<float>& normalsData, int lodLevel)
//{
//	int step = 1 << lodLevel; // 1, 2, 4, 8... based on LOD level
//	int resolutionX = (terrainWidth - 1) / step + 1;
//	int resolutionZ = (terrainDepth - 1) / step + 1;
//
//	vertices.reserve(resolutionX * resolutionZ * 3);
//	texcoords.reserve(resolutionX * resolutionZ * 2);
//	normalsData.reserve(resolutionX * resolutionZ * 3);
//	indices.reserve((resolutionX - 1) * (resolutionZ - 1) * 6);
//
//	// Generate vertices
//	for (int z = 0; z < terrainDepth; z += step)
//	{
//		for (int x = 0; x < terrainWidth; x += step)
//		{
//			float height = GetHeight(x, z);
//
//			// Vertex position
//			vertices.push_back(static_cast<float>(x));
//			vertices.push_back(height);
//			vertices.push_back(static_cast<float>(z));
//
//			// Texture coordinates
//			texcoords.push_back(static_cast<float>(x) / terrainWidth);
//			texcoords.push_back(static_cast<float>(z) / terrainDepth);
//
//			// Normals
//			int idx = z * terrainWidth + x;
//			normalsData.push_back(normals[idx].x);
//			normalsData.push_back(normals[idx].y);
//			normalsData.push_back(normals[idx].z);
//		}
//	}
//
//	// Generate indices
//	for (int z = 0; z < resolutionZ - 1; z++)
//	{
//		for (int x = 0; x < resolutionX - 1; x++)
//		{
//			unsigned short topLeft = z * resolutionX + x;
//			unsigned short topRight = topLeft + 1;
//			unsigned short bottomLeft = (z + 1) * resolutionX + x;
//			unsigned short bottomRight = bottomLeft + 1;
//
//			// First triangle
//			indices.push_back(topLeft);
//			indices.push_back(bottomLeft);
//			indices.push_back(topRight);
//
//			// Second triangle
//			indices.push_back(topRight);
//			indices.push_back(bottomLeft);
//			indices.push_back(bottomRight);
//		}
//	}
//}
//
//void Terrain::CalculateNormals()
//{
//	// Initialize normals to zero
//	for (auto& normal : normals)
//	{
//		normal = { 0.0f, 0.0f, 0.0f };
//	}
//
//	// Calculate normals for each vertex
//	for (int z = 0; z < terrainDepth; z++)
//	{
//		for (int x = 0; x < terrainWidth; x++)
//		{
//			normals[z * terrainWidth + x] = CalculateNormalAt(x, z);
//		}
//	}
//}
//
//Vector3 Terrain::CalculateNormalAt(int x, int z) const
//{
//	// Use central differences to calculate normal
//	float heightL = GetHeight(x - 1, z);
//	float heightR = GetHeight(x + 1, z);
//	float heightD = GetHeight(x, z - 1);
//	float heightU = GetHeight(x, z + 1);
//
//	Vector3 normal;
//	normal.x = heightL - heightR;
//	normal.y = 2.0f;
//	normal.z = heightD - heightU;
//
//	// Normalize
//	float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
//	if (length > 0.0f)
//	{
//		normal.x /= length;
//		normal.y /= length;
//		normal.z /= length;
//	}
//
//	return normal;
//}
//
//// ============================================================================
//// LOD SYSTEM
//// ============================================================================
//
//void Terrain::GenerateLODMeshes()
//{
//	// Clear existing LOD meshes
//	for (auto& lod : lodMeshes)
//	{
//		lod.model.DeleteInstance();
//	}
//	lodMeshes.clear();
//
//	if (!enableLOD)
//		return;
//
//	// Generate LOD levels
//	for (int i = 0; i < lodLevels; i++)
//	{
//		TerrainLOD lod;
//		lod.resolution = (terrainWidth - 1) / (1 << i) + 1;
//		lod.maxDistance = lodDistance * (i + 1);
//
//		std::vector<float> vertices;
//		std::vector<unsigned short> indices;
//		std::vector<float> texcoords;
//		std::vector<float> normalsData;
//
//		GenerateMeshData(vertices, indices, texcoords, normalsData, i);
//
//		// Create mesh using rlgl
//		unsigned int vaoId = rlLoadVertexArray();
//		rlEnableVertexArray(vaoId);
//
//		unsigned int vboId[4];
//		vboId[0] = rlLoadVertexBuffer(vertices.data(), vertices.size() * sizeof(float), false);
//		rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
//		rlEnableVertexAttribute(0);
//
//		vboId[1] = rlLoadVertexBuffer(texcoords.data(), texcoords.size() * sizeof(float), false);
//		rlSetVertexAttribute(1, 2, RL_FLOAT, 0, 0, 0);
//		rlEnableVertexAttribute(1);
//
//		vboId[2] = rlLoadVertexBuffer(normalsData.data(), normalsData.size() * sizeof(float), false);
//		rlSetVertexAttribute(2, 3, RL_FLOAT, 0, 0, 0);
//		rlEnableVertexAttribute(2);
//
//		vboId[3] = rlLoadVertexBufferElement(indices.data(), indices.size() * sizeof(unsigned short), false);
//
//		rlDisableVertexArray();
//
//		Mesh mesh = { 0 };
//		mesh.vertexCount = static_cast<int>(vertices.size() / 3);
//		mesh.triangleCount = static_cast<int>(indices.size() / 3);
//		mesh.vaoId = vaoId;
//		mesh.vboId[0] = vboId[0];
//		mesh.vboId[1] = vboId[1];
//		mesh.vboId[2] = vboId[2];
//		mesh.vboId[6] = vboId[3];
//
//		Model raylibModel = RaylibWrapper::LoadModelFromMesh(mesh);
//
//#if defined(EDITOR)
//		lod.model.Create(Custom, "", ShaderManager::Shaders::Default, ProjectManager::projectData.path / "Assets");
//#else
//		if (exeParent.empty())
//			lod.model.Create(Custom, "", ShaderManager::Shaders::Default, "Resources/Assets");
//		else
//			lod.model.Create(Custom, "", ShaderManager::Shaders::Default, std::filesystem::path(exeParent) / "Resources" / "Assets");
//#endif
//
//		lod.model.SetRaylibModel(raylibModel);
//		lod.model.SetMaterials({ &Material::defaultMaterial });
//
//		lodMeshes.push_back(lod);
//	}
//}
//
//void Terrain::UpdateLOD(const Vector3& cameraPosition)
//{
//	if (!enableLOD || lodMeshes.empty())
//	{
//		currentLODLevel = 0;
//		return;
//	}
//
//	// Calculate distance from camera to terrain center
//	Vector3 terrainPos = gameObject->transform.GetPosition();
//	Vector3 terrainCenter = {
//		terrainPos.x + terrainWidth * 0.5f,
//		terrainPos.y,
//		terrainPos.z + terrainDepth * 0.5f
//	};
//
//	float dx = cameraPosition.x - terrainCenter.x;
//	float dy = cameraPosition.y - terrainCenter.y;
//	float dz = cameraPosition.z - terrainCenter.z;
//	float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
//
//	currentLODLevel = CalculateLODLevel(distance);
//}
//
//int Terrain::CalculateLODLevel(float distance) const
//{
//	for (int i = 0; i < lodMeshes.size(); i++)
//	{
//		if (distance < lodMeshes[i].maxDistance)
//			return i;
//	}
//	return lodMeshes.size() - 1;
//}
//
//void Terrain::SwitchLODMesh(int lodLevel)
//{
//	if (lodLevel < 0 || lodLevel >= lodMeshes.size())
//		return;
//
//	currentLODLevel = lodLevel;
//}
//
//// ============================================================================
//// MATERIAL AND UTILITY
//// ============================================================================
//
//void Terrain::SetMaterial(Material* mat)
//{
//	terrainMaterial = mat;
//}
//
//void Terrain::RegenerateTerrain()
//{
//	if (terrainGenerated)
//	{
//		terrainModel.DeleteInstance();
//		for (auto& lod : lodMeshes)
//		{
//			lod.model.DeleteInstance();
//		}
//		lodMeshes.clear();
//		terrainGenerated = false;
//	}
//
//	// Resize data structures
//	heightData.resize(terrainWidth * terrainDepth, 0.0f);
//	normals.resize(terrainWidth * terrainDepth);
//
//	for (auto& splatMap : splatMaps)
//	{
//		splatMap.resize(terrainWidth * terrainDepth, 0.0f);
//	}
//
//	// Fill base layer
//	if (!splatMaps.empty())
//	{
//		std::fill(splatMaps[0].begin(), splatMaps[0].end(), 1.0f);
//	}
//
//	GenerateTerrain();
//}
//
//Vector3 Terrain::GetTerrainSize() const
//{
//	Vector3 scale = gameObject->transform.GetScale();
//	return {
//		terrainWidth * scale.x,
//		terrainHeight * scale.y,
//		terrainDepth * scale.z
//	};
//}
//
//bool Terrain::IsValidTerrainPosition(int x, int z) const
//{
//	return x >= 0 && x < terrainWidth && z >= 0 && z < terrainDepth;
//}