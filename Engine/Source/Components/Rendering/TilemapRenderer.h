#pragma once

#include "Components/Component.h"
#include "Core/CryonicCore.h"
#include <unordered_map>
#include "Resources/Tilemap.h"
#include "Systems/Rendering/RenderableTexture.h"

class TilemapRenderer : public Component, public RenderableTexture
{
public:
	TilemapRenderer(GameObject* obj, int id) : Component(obj, id)
	{
		runInEditor = true;
		name = "TilemapRenderer";
		iconUnicode = "\xef\x87\x85";

#if defined(EDITOR)
		std::string variables = R"(
        [
            0,
            [
                [
                    "Tilemap",
                    "tilemap",
                    "nullptr",
                    "Tilemap",
                    {
                        "Extensions": [".ldtk"]
                    }
                ],
				[
                    "int",
                    "renderOrder",
                    0,
                    "Render Order"
                ]
            ]
        ]
    )";
		exposedVariables = nlohmann::json::parse(variables);
#endif
	}
	TilemapRenderer* Clone() override
	{
		return new TilemapRenderer(gameObject, -1);
	}
	// Hide everything from API
	void Awake() override;
	void Start() override;
#if defined(EDITOR)
	void EditorUpdate() override;
#endif
	void Destroy() override;
	void Render() override;

	void SetTilemap(Tilemap* tilemap);
	Tilemap* GetTilemap();
	void SetRenderOrder(int order);
	int GetRenderOrder() const override;

	int renderOrder = 0;

private:
	Tilemap* tilemap = nullptr;
	RaylibWrapper::Texture2D tempTexture;
#if defined(EDITOR)
	bool setup = false;
#endif
};