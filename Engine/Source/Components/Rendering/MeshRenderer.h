#pragma once

#include "Components/Component.h"
#include "Raylib/RaylibModelWrapper.h"
#include "Resources/Material.h"

class MeshRenderer : public Component
{
public:
	MeshRenderer(GameObject* obj, int id) : Component(obj, id)
	{
		runInEditor = true;
		name = "MeshRenderer";
		iconUnicode = "\xef\x86\xb2";

#if defined(EDITOR)
		std::string variables = R"(
        [
            0,
            [
                [
                    "Material",
                    "material",
                    "Default",
                    "Material",
                    {
                        "Extensions": [".mat"]
                    }
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
	MeshRenderer* Clone() override
	{
		return new MeshRenderer(gameObject, -1);
	}
	/// Hide everything in this in the API
	void Awake() override;
	void Start() override {};
	void Render(bool renderShadows) override;
#if defined(EDITOR)
	void EditorUpdate() override;
#endif
	void Destroy() override;

	RaylibModel& GetModel();
	void SetModel(ModelType model, std::filesystem::path path, ShaderManager::Shaders shader);
	std::filesystem::path GetModelPath() const;
	void SetModelPath(std::filesystem::path path);
	void SetMaterial(Material* mat);
	Material* GetMaterial();

private:
	bool setShader = false;
	RaylibModel raylibModel;
	bool modelSet = false;
	std::filesystem::path modelPath;
	Material* material = nullptr;
	bool defaultMaterial = false;

	bool castShadows;
};