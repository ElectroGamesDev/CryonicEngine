#include "RaylibModelWrapper.h"
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "RaylibShaderWrapper.h"
#include <unordered_map>
#include "ConsoleLogger.h"

static std::unordered_map<std::filesystem::path, std::pair<Model, int>> models;
static std::unordered_map<ModelType, std::pair<Model, int>> primitiveModels;
static std::unordered_map<std::filesystem::path, std::vector<Material>> embeddedMaterials;
std::pair<unsigned int, int*> RaylibModel::shadowShader;
std::pair<unsigned int, int*> RaylibModel::materialPreviewShadowShader;
//std::unordered_map<Model, std::vector<RaylibWrapper::Material>> RaylibModel::rWrapperMaterials;

bool RaylibModel::Create(ModelType type, std::filesystem::path path, ShaderManager::Shaders shader, std::filesystem::path projectPath, std::vector<float> data)
{
    // Todo: Don't create new meshes for primitives
    switch (type)
    {
    case ModelType::Custom:
        if (!std::filesystem::exists(projectPath / path))
        {
            // Todo: Send user error
            return false;
        }

        if (auto it = models.find(path); it != models.end())
        {
            model = &(it->second);
            model->second++;
        }
        else
        {
            models[path] = std::make_pair(LoadModel((projectPath/ path).string().c_str()), 1);
            model = &models[path];
        }
        break;
    case ModelType::Cube:
        if (auto it = primitiveModels.find(ModelType::Cube); it != primitiveModels.end())
        {
            model = &(it->second);
            model->second++;
        }
        else
        {
            primitiveModels[ModelType::Cube] = std::make_pair(LoadModelFromMesh(GenMeshCube(1, 1, 1)), 1);
            model = &primitiveModels[ModelType::Cube];
        }
        primitiveModel = true;
        break;
    case ModelType::Sphere:
        if (auto it = primitiveModels.find(ModelType::Sphere); it != primitiveModels.end())
        {
            model = &(it->second);
            model->second++;
        }
        else
        {
            primitiveModels[ModelType::Sphere] = std::make_pair(LoadModelFromMesh(GenMeshSphere(0.5f, 32, 32)), 1);
            model = &primitiveModels[ModelType::Sphere];
        }
        primitiveModel = true;
        break;
    case ModelType::Plane:
        if (auto it = primitiveModels.find(ModelType::Plane); it != primitiveModels.end())
        {
            model = &(it->second);
            model->second++;
        }
        else
        {
			int size = 1;
			if (!data.empty() && data[0] > 0)
                size = data[0];

            int subDivisions = 1;
            if (!data.empty() && data[1] > 0)
				subDivisions = data[1];

            primitiveModels[ModelType::Plane] = std::make_pair(LoadModelFromMesh(GenMeshPlane(size, size, subDivisions, subDivisions)), 1);
            model = &primitiveModels[ModelType::Plane];
        }
        primitiveModel = true;
        break;
    case ModelType::Cylinder:
        if (auto it = primitiveModels.find(ModelType::Cylinder); it != primitiveModels.end())
        {
            model = &(it->second);
            model->second++;
        }
        else
        {
            primitiveModels[ModelType::Cylinder] = std::make_pair(LoadModelFromMesh(GenMeshCylinder(0.5f, 2, 32)), 1);
            model = &primitiveModels[ModelType::Cylinder];
        }
        primitiveModel = true;
        break;
    case ModelType::Cone:
        if (auto it = primitiveModels.find(ModelType::Cone); it != primitiveModels.end())
        {
            model = &(it->second);
            model->second++;
        }
        else
        {
            primitiveModels[ModelType::Cone] = std::make_pair(LoadModelFromMesh(GenMeshCone(0.5f, 1, 32)), 1);
            model = &primitiveModels[ModelType::Cone];
        }
        primitiveModel = true;
        break;
	case ModelType::Skybox:
        model = new std::pair<Model, int>(LoadModelFromMesh(GenMeshCube(1, 1, 1)), 1);
		skyboxModel = true;
		break;
    default:
        return false;
    }

    modelShader = shader;

    if (path == "MaterialPreview" && projectPath == "MaterialPreview")
    {
        for (size_t i = 0; i < model->first.materialCount; ++i)
            model->first.materials[i].shader = { materialPreviewShadowShader.first, materialPreviewShadowShader.second };
    }
    else if (shader == ShaderManager::LitStandard)
    {
        for (size_t i = 0; i < model->first.materialCount; ++i)
            //model->first.materials[i].shader = RaylibShader::shaders[modelShader].shader;
            model->first.materials[i].shader = { shadowShader.first, shadowShader.second };
    }
	else if (shader == ShaderManager::Skybox)
	{
		std::pair<unsigned int, int*> skyShader = ShaderManager::GetShader(ShaderManager::Skybox);
		for (size_t i = 0; i < model->first.materialCount; ++i)
			model->first.materials[i].shader = { skyShader.first, skyShader.second };
	}
    else // Default to the lit shader
    {
		for (size_t i = 0; i < model->first.materialCount; ++i)
			model->first.materials[i].shader = { shadowShader.first, shadowShader.second };
    }

    for (int i = 0; i < model->first.materialCount; i++)
    {
        // Setting the embedded materials' ID to 1 (the ID for embedded materials)
        model->first.materials[i].params[0] = static_cast<int>(1);

        // Setup the rWrapperMaterials so functions like GetMaterials() can be used
        //RaylibWrapper::Material mat;

        //mat.maps = new RaylibWrapper::MaterialMap[RaylibWrapper::MAX_MATERIAL_MAPS];

        //for (int j = 0; j < RaylibWrapper::MAX_MATERIAL_MAPS; ++j)
        //{
        //    MaterialMap& map = model->first.materials[i].maps[j];
        //    mat.maps[j] = {
        //        { map.texture.id, map.texture.width, map.texture.height, map.texture.mipmaps, map.texture.format},
        //        { map.color.r, map.color.g, map.color.b, map.color.a },
        //        1.0f
        //    };
        //}

        //mat.shader.id = model->first.materials[i].shader.id;
        //mat.shader.locs = model->first.materials[i].shader.locs;

        //for (int j = 0; j < 4; j++)
        //    mat.params[j] = model->first.materials[i].params[j];

        //rWrapperMaterials[model->first].push_back(mat);
    }

    this->path = path;

    SetEmbeddedMaterials();

    return true;
}

bool RaylibModel::CreateFromHeightData(const std::vector<std::vector<float>>& heightData,
	int width,
	int depth,
	float maxHeight,
	RaylibWrapper::Material* material)
{
	if (width <= 1 || depth <= 1) return false;

	// Cleanup old model if any
	DeleteInstance();

	const int vertexCount = width * depth;
	const int quadCount = (width - 1) * (depth - 1) * 2;

	std::vector<Vector3> vertices(vertexCount);
	std::vector<Vector3> normals(vertexCount);
	std::vector<Vector2> texcoords(vertexCount);
	std::vector<unsigned short> indices(quadCount * 3);

	// --- Generate vertex data ---
	for (int z = 0; z < depth; z++)
	{
		for (int x = 0; x < width; x++)
		{
			int i = z * width + x;
			float height = heightData[z][x];
			vertices[i] = { (float)x, height, (float)z };
			texcoords[i] = { (float)x / (float)width, (float)z / (float)depth };
			normals[i] = { 0, 1, 0 }; // will be recalculated later
		}
	}

	// --- Generate triangle indices ---
	int idx = 0;
	for (int z = 0; z < depth - 1; z++)
	{
		for (int x = 0; x < width - 1; x++)
		{
			int topLeft = z * width + x;
			int topRight = topLeft + 1;
			int bottomLeft = (z + 1) * width + x;
			int bottomRight = bottomLeft + 1;

			// First triangle
			indices[idx++] = topLeft;
			indices[idx++] = bottomLeft;
			indices[idx++] = topRight;

			// Second triangle
			indices[idx++] = topRight;
			indices[idx++] = bottomLeft;
			indices[idx++] = bottomRight;
		}
	}

	// --- Compute normals ---
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		int i0 = indices[i];
		int i1 = indices[i + 1];
		int i2 = indices[i + 2];

		Vector3 v0 = vertices[i0];
		Vector3 v1 = vertices[i1];
		Vector3 v2 = vertices[i2];

		Vector3 edge1 = Vector3Subtract(v1, v0);
		Vector3 edge2 = Vector3Subtract(v2, v0);
		Vector3 faceNormal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));

		normals[i0] = Vector3Add(normals[i0], faceNormal);
		normals[i1] = Vector3Add(normals[i1], faceNormal);
		normals[i2] = Vector3Add(normals[i2], faceNormal);
	}

	for (Vector3& n : normals)
		n = Vector3Normalize(n);

	// --- Build Mesh ---
	Mesh mesh = { 0 };
	mesh.vertexCount = vertexCount;
	mesh.triangleCount = quadCount;

	mesh.vertices = (float*)MemAlloc(vertexCount * 3 * sizeof(float));
	mesh.normals = (float*)MemAlloc(vertexCount * 3 * sizeof(float));
	mesh.texcoords = (float*)MemAlloc(vertexCount * 2 * sizeof(float));
	mesh.indices = (unsigned short*)MemAlloc(quadCount * 3 * sizeof(unsigned short));

	for (int i = 0; i < vertexCount; i++)
	{
		mesh.vertices[i * 3 + 0] = vertices[i].x;
		mesh.vertices[i * 3 + 1] = vertices[i].y;
		mesh.vertices[i * 3 + 2] = vertices[i].z;

		mesh.normals[i * 3 + 0] = normals[i].x;
		mesh.normals[i * 3 + 1] = normals[i].y;
		mesh.normals[i * 3 + 2] = normals[i].z;

		mesh.texcoords[i * 2 + 0] = texcoords[i].x;
		mesh.texcoords[i * 2 + 1] = texcoords[i].y;
	}

	memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned short));

	UploadMesh(&mesh, true);

	// --- Build Model ---
    model = new std::pair<Model, int>(LoadModelFromMesh(mesh), 1);

	//if (material)
 //       model->first.materials[0] = *material->GetRaylibMaterial();
	//else
 //       model->first.materials[0] = Material::defaultMaterial;

    terrainModel = true;

	// Set shader
	modelShader = ShaderManager::Terrain;

    SetMaterials({ material });

    for (size_t i = 0; i < model->first.materialCount; ++i)
    {
        std::pair<unsigned int, int*> terrainShader = ShaderManager::GetShader(ShaderManager::Terrain);
        model->first.materials[i].shader = { terrainShader.first, terrainShader.second };
    }

	for (int i = 0; i < model->first.materialCount; i++)
		model->first.materials[i].params[0] = static_cast<int>(1);

	this->path = path;

	// Set embeded materials
	SetEmbeddedMaterials();

	return true;
}

void RaylibModel::Unload()
{
    if (model == nullptr)
        return;

    UnloadModel(model->first);
    if (primitiveModel)
    {
        auto it = primitiveModels.begin();
        while (it != primitiveModels.end())
        {
            if (&it->second == model)
            {
                it = primitiveModels.erase(it);
                break;
            }
            else
                ++it;
        }
    }
    else if (!terrainModel)
    {
        auto it = models.begin();
        while (it != models.end())
        {
            if (&it->second == model)
            {
                it = models.erase(it);
                break;
            }
            else
                ++it;
        }
    }
    else
        delete model; // Todo: Should it delete the model for primitive and custom too, not just terrain and skybox?

    model = nullptr;

    // Only delete the maps for embedded materials. I don't think I need to delete the maps for non-embedded materials since they use the same pointers as the mats paramter. If there are issues, then deep copy the maps.
    //if (rWrapperMaterials[model->first][0].params[0] == 1)
    //    for (RaylibWrapper::Material& mat : rWrapperMaterials[model->first])
    //        delete[] mat.maps;

    //rWrapperMaterials.clear();
}

void RaylibModel::DeleteInstance()
{
    if (!model)
        return;

    model->second--;
    if (model->second <= 0)
        Unload();
}

void RaylibModel::DrawModelWrapper(float posX, float posY, float posZ, float sizeX, float sizeY, float sizeZ, float rotationX, float rotationY, float rotationZ, float rotationW, unsigned char colorR, unsigned char colorG, unsigned char colorB, unsigned char colorA, bool loadIdentity, bool ortho, bool ndc)
{
    if (model == nullptr || model->first.meshCount < 1)
    {
        ConsoleLogger::ErrorLog("Error drawing model");
        return;

    }

    rlPushMatrix();

    if (loadIdentity)
        rlLoadIdentity();

    if (ortho)
        rlOrtho(-1, 1, -1, 1, 0, 1); // I'm not sure if these parameters will work with everything, but its used for clouds. If not, it might be best to derive from this class, and make a custom DrawModelWrapper()

    // build up the transform
    Matrix transform = MatrixTranslate(posX, posY, posZ);

    transform = MatrixMultiply(QuaternionToMatrix({rotationX, rotationY, rotationZ, rotationW}), transform);

    transform = MatrixMultiply(MatrixScale(sizeX, sizeY, sizeZ), transform);

    if (ndc)
    {
		// Add NDC transformation: Scale and translate to map world space to NDC
        // Assuming the plane is generated in a unit size, scale it to fit -1 to 1
		Matrix ndcTransform = MatrixScale(2.0f, 2.0f, 1.0f);  // Scale to cover -1 to 1 in X and Z
		ndcTransform = MatrixMultiply(MatrixTranslate(-1.0f, 0.0f, -1.0f), ndcTransform);  // Translate to NDC origin
		transform = MatrixMultiply(ndcTransform, transform);  // Apply NDC transform
    }

    // apply the transform
    rlMultMatrixf(MatrixToFloat(transform));

    //BeginShaderMode(ShaderManager::shaders[_shader]); // Todo: I think I can remove this since I'm setting the shader in Create()

    //if (renderShadow)
    //    BeginShaderMode({ shadowShader.first, shadowShader.second });

    // Draw model
    DrawModel(model->first, Vector3Zero(), 1, { colorR, colorG, colorB, colorA });

    //if (renderShadow)
    //    EndShaderMode();

    //EndShaderMode();

    rlPopMatrix();
}

void RaylibModel::SetShadowShader(unsigned int id, int* locs)
{
    shadowShader = { id, locs };
}

void RaylibModel::SetMaterialPreviewShader(unsigned int id, int* locs)
{
    materialPreviewShadowShader = { id, locs };
}

void RaylibModel::SetShaderValue(int materialIndex, int locIndex, const void* value, int uniformType)
{
    if (!model)
        return;

    ::SetShaderValue(model->first.materials[materialIndex].shader, locIndex, value, uniformType);
}

int RaylibModel::GetShaderLocation(int materialIndex, std::string uniformName)
{
    if (!model)
        return -1;

    return ::GetShaderLocation(model->first.materials[materialIndex].shader, uniformName.c_str());
}

void RaylibModel::SetShader(int materialIndex, ShaderManager::Shaders shader)
{
    modelShader = shader;
    if (shader == ShaderManager::LitStandard)
    {
        model->first.materials[materialIndex].shader = { shadowShader.first, shadowShader.second };
        //rWrapperMaterials[model->first][materialIndex].shader = { shadowShader.first, shadowShader.second };
    }
    else if (shader == ShaderManager::None)
    {
        model->first.materials[materialIndex].shader = {};
        //rWrapperMaterials[model->first][materialIndex].shader = {};
    }
}

int RaylibModel::GetShaderID(int materialIndex)
{
	return model->first.materials[materialIndex].shader.id;
}

bool RaylibModel::IsPrimitive()
{
    return primitiveModel;
}

int RaylibModel::GetMeshCount()
{
    return model ? model->first.meshCount : 0;
}

int RaylibModel::GetTriangleCount(int meshIndex)
{
    if (!model || meshIndex < 0 || meshIndex >= model->first.meshCount)
        return 0;

    return model->first.meshes[meshIndex].triangleCount;
}

std::vector<int> RaylibModel::GetMaterialIDs()
{
    std::vector<int> ids;
    for (int i = 0; i < model->first.materialCount; i++)
        ids.push_back(static_cast<int>(model->first.materials[i].params[0]));

    return ids;
}

bool RaylibModel::CompareMaterials(std::vector<int> matIDs)
{
    // This assumes the materials are in the same order, which they should be
    if (model->first.materialCount != matIDs.size())
        return false;

    for (int i = 0; i < model->first.materialCount; i++)
    {
        if (model->first.materials[i].params[0] != matIDs[i]) // Compares the materials' IDs
            return false;
    }

    return true;
}

int RaylibModel::GetMaterialCount()
{
    return model->first.materialCount;
}

void RaylibModel::SetMaterialMap(int materialIndex, int mapIndex, RaylibWrapper::Texture2D texture, RaylibWrapper::Color color, float intensity)
{
    if (!model)
        return;

    model->first.materials[materialIndex].maps[mapIndex] = {
        { texture.id, texture.width, texture.height, texture.mipmaps, texture.format },
        { color.r, color.g, color.b, 255 },
        intensity
    };

    //rWrapperMaterials[model->first][materialIndex].maps[mapIndex] = {
    //    { texture.id, texture.width, texture.height, texture.mipmaps, texture.format },
    //    { color.r, color.g, color.b, 255 },
    //    intensity
    //};
}

void RaylibModel::SetMaterials(std::vector<RaylibWrapper::Material*> mats) // Todo: There is a memory leak here
{
    if (!model)
        return;

    // Only delete the maps for embedded materials. I don't think I need to delete the maps for non-embedded materials since they use the same pointers as the mats paramter. If there are issues, then deep copy the maps.
    //if (rWrapperMaterials[model->first][0].params[0] == 1)
    //    for (RaylibWrapper::Material& mat : rWrapperMaterials[model->first])
    //        delete[] mat.maps;

    //rWrapperMaterials.clear();

    // Deallocate the materials' maps
    if (model->first.materials)
    {
        for (int i = 0; i < model->first.materialCount; ++i)
        {
            if (model->first.materials[i].maps)
                delete[] model->first.materials[i].maps;
        }
    }

    delete[] model->first.materials;

    model->first.materialCount = mats.size();
    model->first.materials = new Material[mats.size()];

    for (size_t i = 0; i < mats.size(); ++i)
    {
        //rWrapperMaterials[model->first].push_back(*mats[i]);

        model->first.materials[i].maps = new MaterialMap[RaylibWrapper::MAX_MATERIAL_MAPS];

        RaylibWrapper::Material& mat = *mats[i];
        Material& material = model->first.materials[i];

        // Set shader
        material.shader.id = mat.shader.id;
        material.shader.locs = mat.shader.locs;

        // Set params
        material.params[0] = mat.params[0];
        material.params[1] = mat.params[1];
        material.params[2] = mat.params[2];
        material.params[3] = mat.params[3];

        // Set maps
        for (int j = 0; j < RaylibWrapper::MAX_MATERIAL_MAPS; ++j)
            SetMaterialMap(i, j, mat.maps[j].texture, mat.maps[j].color, mat.maps[j].value);
    }
}

int RaylibModel::GetMaterialID(int index)
{
    return static_cast<int>(model->first.materials[index].params[0]);
}

int RaylibModel::GetTextureID(int materialIndex, int mapIndex)
{
	return model->first.materials[materialIndex].maps[mapIndex].texture.id;
}

void RaylibModel::SetEmbeddedMaterials()
{
    if (!model || embeddedMaterials.find(path) != embeddedMaterials.end())
        return;

    std::vector<Material> materials;

    for (int i = 0; i < model->first.materialCount; i++)
    {
        Material srcMaterial = model->first.materials[i];

        // Allocate new maps array for the material
        srcMaterial.maps = new MaterialMap[RaylibWrapper::MAX_MATERIAL_MAPS];

        // Copy each MaterialMap from source to new material
        memcpy(srcMaterial.maps, model->first.materials[i].maps, sizeof(MaterialMap) * RaylibWrapper::MAX_MATERIAL_MAPS);

        materials.push_back(srcMaterial);
    }

    embeddedMaterials[path] = materials;
}

void RaylibModel::SetMaterialsToEmbedded()
{
    if (model->first.materials)
    {
        for (int i = 0; i < model->first.materialCount; ++i)
        {
            if (model->first.materials[i].maps)
                delete[] model->first.materials[i].maps;
        }
    }

    delete[] model->first.materials;

    std::vector<Material> mats = embeddedMaterials[path];

    model->first.materialCount = mats.size();
    model->first.materials = new Material[mats.size()];

    for (size_t i = 0; i < mats.size(); ++i)
    {
        model->first.materials[i].maps = new MaterialMap[RaylibWrapper::MAX_MATERIAL_MAPS];

        Material& mat = mats[i];
        Material& material = model->first.materials[i];

        // Set shader
        material.shader.id = mat.shader.id;
        material.shader.locs = mat.shader.locs;

        // Set params
        material.params[0] = mat.params[0];
        material.params[1] = mat.params[1];
        material.params[2] = mat.params[2];
        material.params[3] = mat.params[3];

        // Set maps
        for (int j = 0; j < RaylibWrapper::MAX_MATERIAL_MAPS; ++j)
        {
            RaylibWrapper::Texture2D texture = { mat.maps[j].texture.id, mat.maps[j].texture.width, mat.maps[j].texture.height, mat.maps[j].texture. mipmaps, mat.maps[j].texture.format };
            RaylibWrapper::Color color = { mat.maps[j].color.r, mat.maps[j].color.g, mat.maps[j].color.b, mat.maps[j].color.a };
            SetMaterialMap(i, j, texture, color, mat.maps[j].value);
        }
    }
}

//RaylibWrapper::Material RaylibModel::GetMaterial(int index)
//{
//    if (!model)
//        return {};
//    
//    return rWrapperMaterials[model->first][index];
//}