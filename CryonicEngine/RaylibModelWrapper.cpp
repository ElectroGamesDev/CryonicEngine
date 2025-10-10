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

bool RaylibModel::Create(ModelType type, std::filesystem::path path, ShaderManager::Shaders shader, std::filesystem::path projectPath)
{
    // Todo: Don't create new meshes for primitives
    switch (type)
    {
    case Custom:
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
    case Cube:
        if (auto it = primitiveModels.find(Cube); it != primitiveModels.end())
        {
            model = &(it->second);
            model->second++;
        }
        else
        {
            primitiveModels[Cube] = std::make_pair(LoadModelFromMesh(GenMeshCube(1, 1, 1)), 1);
            model = &primitiveModels[Cube];
        }
        primitiveModel = true;
        break;
    case Sphere:
        if (auto it = primitiveModels.find(Sphere); it != primitiveModels.end())
        {
            model = &(it->second);
            model->second++;
        }
        else
        {
            primitiveModels[Sphere] = std::make_pair(LoadModelFromMesh(GenMeshSphere(0.5f, 32, 32)), 1);
            model = &primitiveModels[Sphere];
        }
        primitiveModel = true;
        break;
    case Plane:
        if (auto it = primitiveModels.find(Plane); it != primitiveModels.end())
        {
            model = &(it->second);
            model->second++;
        }
        else
        {
            primitiveModels[Plane] = std::make_pair(LoadModelFromMesh(GenMeshPlane(1, 1, 1, 1)), 1);
            model = &primitiveModels[Plane];
        }
        primitiveModel = true;
        break;
    case Cylinder:
        if (auto it = primitiveModels.find(Cylinder); it != primitiveModels.end())
        {
            model = &(it->second);
            model->second++;
        }
        else
        {
            primitiveModels[Cylinder] = std::make_pair(LoadModelFromMesh(GenMeshCylinder(0.5f, 2, 32)), 1);
            model = &primitiveModels[Cylinder];
        }
        primitiveModel = true;
        break;
    case Cone:
        if (auto it = primitiveModels.find(Cone); it != primitiveModels.end())
        {
            model = &(it->second);
            model->second++;
        }
        else
        {
            primitiveModels[Cone] = std::make_pair(LoadModelFromMesh(GenMeshCone(0.5f, 1, 32)), 1);
            model = &primitiveModels[Cone];
        }
        primitiveModel = true;
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
    else if (shader != ShaderManager::None)
    {
        for (size_t i = 0; i < model->first.materialCount; ++i)
            //model->first.materials[i].shader = RaylibShader::shaders[modelShader].shader;
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

void RaylibModel::CreateTerrain(int vertexCount, int triangleCount, unsigned int vaoId, std::vector<unsigned int> vboIds)
{
	if (model)
	{
		// Manually cleanup to avoid double-free
		if (terrainModel) {
			// For terrain, manually unload the GPU resources
			for (int i = 0; i < model->first.meshCount; i++) {
				Mesh& mesh = model->first.meshes[i];
				if (mesh.vaoId > 0) {
					rlUnloadVertexArray(mesh.vaoId);
				}
				// Don't unload vboId - they're managed elsewhere
				RL_FREE(mesh.vboId);
			}
			RL_FREE(model->first.meshes);

			// Free materials
			for (int i = 0; i < model->first.materialCount; i++) {
				RL_FREE(model->first.materials[i].maps);
			}
			RL_FREE(model->first.materials);
			RL_FREE(model->first.meshMaterial);
		}
		else {
			UnloadModel(model->first);
		}
		delete model;
		model = nullptr;
	}

	// Create minimal mesh structure that references GPU data
	Mesh mesh = { 0 };
	mesh.vertexCount = vertexCount;
	mesh.triangleCount = triangleCount;
	mesh.vaoId = vaoId;

	// DON'T allocate vboId array - leave it NULL
	// LoadModelFromMesh will handle this
	mesh.vboId = nullptr;

	// Important: NULL out ALL pointers
	mesh.vertices = nullptr;
	mesh.texcoords = nullptr;
	mesh.normals = nullptr;
	mesh.colors = nullptr;
	mesh.tangents = nullptr;
	mesh.texcoords2 = nullptr;
	mesh.indices = nullptr;
	mesh.animVertices = nullptr;
	mesh.animNormals = nullptr;
	mesh.boneIds = nullptr;
	mesh.boneWeights = nullptr;
	mesh.boneMatrices = nullptr;

	// LoadModelFromMesh will allocate its own vboId array
	Model terrainModelRaw = LoadModelFromMesh(mesh);

	// Now manually set the vboId values we want
	if (terrainModelRaw.meshCount > 0) {
		Mesh& modelMesh = terrainModelRaw.meshes[0];
		if (modelMesh.vboId && vboIds.size() >= 4) {
			modelMesh.vboId[0] = vboIds[0]; // vertices
			modelMesh.vboId[1] = vboIds[1]; // texcoords
			modelMesh.vboId[2] = vboIds[2]; // normals
			modelMesh.vboId[6] = vboIds[3]; // indices
		}
	}

	model = new std::pair<Model, int>(terrainModelRaw, 1);
	terrainModel = true;

	// Set shader
	modelShader = ShaderManager::LitStandard;
	for (size_t i = 0; i < model->first.materialCount; ++i)
		model->first.materials[i].shader = { shadowShader.first, shadowShader.second };

	// Set embedded materials
	for (int i = 0; i < model->first.materialCount; i++)
	{
		model->first.materials[i].params[0] = static_cast<int>(1);
	}

	this->path = "";
	SetEmbeddedMaterials();

	ConsoleLogger::InfoLog("Terrain model created: " + std::to_string(vertexCount) +
		" vertices, " + std::to_string(triangleCount) + " triangles, VAO: " +
		std::to_string(vaoId));
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
    else if (terrainModel)
    {
        UnloadModel(model->first);
    }
    else
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
    model = nullptr;

    // Only delete the maps for embedded materials. I don't think I need to delete the maps for non-embedded materials since they use the same pointers as the mats paramter. If there are issues, then deep copy the maps.
    //if (rWrapperMaterials[model->first][0].params[0] == 1)
    //    for (RaylibWrapper::Material& mat : rWrapperMaterials[model->first])
    //        delete[] mat.maps;

    //rWrapperMaterials.clear();
}

void RaylibModel::DeleteInstance()
{
    model->second--;
    if (model->second <= 0)
        Unload();
}

void RaylibModel::DrawModelWrapper(float posX, float posY, float posZ, float sizeX, float sizeY, float sizeZ, float rotationX, float rotationY, float rotationZ, float rotationW, unsigned char colorR, unsigned char colorG, unsigned char colorB, unsigned char colorA)
{
    if (model == nullptr || model->first.meshCount < 1)
    {
        ConsoleLogger::ErrorLog("Error drawing model");
        return;

    }

    rlPushMatrix();

    // build up the transform
    Matrix transform = MatrixTranslate(posX, posY, posZ);

    transform = MatrixMultiply(QuaternionToMatrix({rotationX, rotationY, rotationZ, rotationW}), transform);

    transform = MatrixMultiply(MatrixScale(sizeX, sizeY, sizeZ), transform);

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