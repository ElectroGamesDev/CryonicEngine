#include "MeshRenderer.h"
#include "Core/GameObject.h"
#if defined (EDITOR)
#include "Core/ProjectManager.h"
#else
#include "Game.h"
#endif
//#include "rlgl.h"
//#include "Systems/Rendering/ShaderManager.h"

void MeshRenderer::Awake()
{
#if defined (EDITOR)
    //SetMaterial(Material::GetMaterial(exposedVariables[1][0][2])); // Todo: Wouldn't I need to set this for non-editor too?
#endif
}

RaylibModel& MeshRenderer::GetModel()
{
    return raylibModel;
}

void MeshRenderer::SetModel(ModelType model, std::filesystem::path path, ShaderManager::Shaders shader)
{
    // Todo: Remove ModelType and just use the path to determine if its a primitive shape. Path would most likely be an absolute path to the /Assets folder

    // If a custom model was previously used then cleanup the embeded materials
    // Todo: Re-add this, although use embeddedMaterials from RaylibModelWrapper, and make sure no other MeshRenderer is using the model
    //if (this->modelSet && !raylibModel.IsPrimitive())
    //{
    //    for (RaylibWrapper::Material& mat : embededMaterials)
    //        delete[] mat.maps;

    //    embededMaterials.clear();
    //}

    //this->raylibModel = model;
#if defined (EDITOR)
    this->modelSet = raylibModel.Create(model, path, shader, ProjectManager::projectData.path / "Assets");
#else
    if (exeParent.empty())
        this->modelSet = raylibModel.Create(model, path, shader, "Resources/Assets");
    else
        this->modelSet = raylibModel.Create(model, path, shader, std::filesystem::path(exeParent) / "Resources" / "Assets");
#endif

    // This may cause issues when I fix start freeing/deallocating models since their material may already be freed from this. I'm not sure if Raylib creates a new material for each new primitive mesh.
    if (model != ModelType::Custom)
        raylibModel.SetMaterials({ &Material::defaultMaterial });

    //SetMaterial(material);
}

std::filesystem::path MeshRenderer::GetModelPath() const
{
    return modelPath;
}

void MeshRenderer::SetModelPath(std::filesystem::path path)
{
    this->modelPath = path;
}

void MeshRenderer::SetMaterial(Material* mat) // Todo: MEMORY LEAK! We shouldn't be creating new materials here! Materials should already be created.
{
    // Todo: The material can be used by multiple meshes. Track number of meshes using the material and if it only equals 1 (This mesh), then unload it
    material = mat;

    // Commented out the following code since now materials are only changed on the model before loading. This allows different MeshRenderers using the the same model instance to use different materials

    //if (modelSet)
    //{
    //    // I need to use SetMaterials() instead of SetMaterialMap() so it removes the other materials on the model and so it doesn't affect other models with the old material
    //    if (material == nullptr || material->GetPath() == "Default")
    //    {
    //        // Set to the embeded materials
    //        if (embededMaterials.size() > 0)
    //        {
    //            // Todo: If a normal map isn't set, its not using the flat values and if the roughness map isn't set, it's most likely using 0 for the value
    //            std::vector<RaylibWrapper::Material*> embededMats;

    //            for (RaylibWrapper::Material& embededMat : embededMaterials)
    //                embededMats.push_back(&embededMat);

    //            raylibModel.SetMaterials(embededMats);
    //        }
    //        // Set to a default material
    //        else
    //        {
    //            raylibModel.SetMaterials({ &Material::defaultMaterial });
    //        }
    //    }
    //    else
    //    {
    //        // Todo: Consider supporting multiple materials. Do this by checking if the mesh uses multiple materials.
    //        raylibModel.SetMaterials({ material->GetRaylibMaterial()});
    //    }
    //}
}

Material* MeshRenderer::GetMaterial()
{
    return material;
}

void MeshRenderer::Render(bool renderShadows)
{
    if (!modelSet || (renderShadows && !castShadows))
        return;

    Vector3 position = gameObject->transform.GetPosition();
    Quaternion rotation = gameObject->transform.GetRotation();
    Vector3 scale = gameObject->transform.GetScale();

    // If this MeshRenderer is using a different material than the model's embeded/default materials, then change the material on the model. Currently only embedded materials are supported, not default (set in the model data file).
    // We are not resetting the material back to embedded/default after since it's possible that the next MeshRenderer using this model may also not use the default materials, which therefore causes unnecessary overhead.

    if ((!material && raylibModel.GetMaterialID(0) > 1) || // If the material is not set (using embedded) and if the first material ID of the model is not set to 0 or 1 (the embedded and default id)
        (material && !raylibModel.CompareMaterials({ material->GetID()}))) // If is the material is set, and the IDs are not the same
    {
        if (!material)
        {
            // Need to compare the IDs for default and embedded materials
            if (raylibModel.IsPrimitive() && raylibModel.GetMaterialID(0) != 0)
                raylibModel.SetMaterials({ &Material::defaultMaterial });
            else if (!raylibModel.IsPrimitive() && raylibModel.GetMaterialID(0) != 1)
                raylibModel.SetEmbeddedMaterials();
        }
        else
            raylibModel.SetMaterials({ material->GetRaylibMaterial()});
    }

    raylibModel.DrawModelWrapper(position.x, position.y, position.z, scale.x, scale.y, scale.z, rotation.x, rotation.y, rotation.z, rotation.w, 255, 255, 255, 255);
}

// TODO: IN AN UPDATE FUNCTION (make sure editor runs it too), CHEDCK IF MATERIAL updated FLAG HAS BEEN CHECKED, IF IT HAS, THEN UPDATE THE MATERIAL. Although then when would it flip back?
// Maybe I should use callbacks

#if defined(EDITOR)
void MeshRenderer::EditorUpdate()
{
    if (exposedVariables[1][0][2] == "nullptr")
        exposedVariables[1][0][2] = "Default";

    // Set material
    if ((!material && !(exposedVariables[1][0][2] == "Default" && defaultMaterial)) || (material && material->GetPath() != exposedVariables[1][0][2])) // Todo: This will run continuously if the material is deleted and the file is deleted/moved.
    {
        //ConsoleLogger::WarningLog(gameObject->GetName());
        if (exposedVariables[1][0][2] == "Default")
        {
            //ConsoleLogger::WarningLog("Using default material");
            defaultMaterial = true;
            SetMaterial(nullptr);
        }
        else
            SetMaterial(Material::GetMaterial(exposedVariables[1][0][2]));
    }

    castShadows = exposedVariables[1][1][2].get<bool>();
}
#endif

void MeshRenderer::Destroy()
{
    // Todo: Currently Destroy() does work in the editor, but it shouldn't. Will need to move this.
    if (modelSet)
        raylibModel.DeleteInstance();
}