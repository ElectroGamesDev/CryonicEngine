#pragma once

#include "Components/Component.h"
#include "Core/CryonicCore.h"
#include <unordered_map>
#include "Resources/Sprite.h"
#include "Systems/Rendering/RenderableTexture.h"

class SpriteRenderer : public Component, public RenderableTexture
{
public:
	SpriteRenderer(GameObject* obj, int id) : Component(obj, id)
	{
		runInEditor = true;
		name = "SpriteRenderer";
		iconUnicode = "\xef\x87\x85";

#if defined(EDITOR)
		std::string variables = R"(
        [
            0,
            [
                [
                    "Sprite",
                    "sprite",
                    "Square",
                    "Sprite",
                    {
                        "Extensions": [".png", ".jpg", ".jpeg"]
                    }
                ],
                [
                    "Color",
                    "tint",
                    [ 255, 255, 255, 255 ],
                    "Tint"
                ],
				[
                    "int",
                    "renderOrder",
                    0,
                    "Render Order"
                ],
				[
                    "bool",
                    "flipX",
                    false,
                    "Flip X"
                ],
				[
                    "bool",
                    "flipY",
                    false,
                    "Flip Y"
                ]
            ]
        ]
    )";
		exposedVariables = nlohmann::json::parse(variables);
#endif
	}
	SpriteRenderer* Clone() override
	{
		return new SpriteRenderer(gameObject, -1);
	}
	// Hide everything from API
	void Awake() override;
	void Start() override;
#if defined(EDITOR)
	void EditorUpdate() override;
#endif
	void Destroy() override;
	void Render() override;

	bool IntersectsPoint(Vector2 pos);

	void SetSprite(Sprite* sprite);
	Sprite* GetSprite();
	void SetTint(Color tint);
	Color GetTint() const;
	void SetRenderOrder(int order);
	int GetRenderOrder() const override;
	void SetFlipX(bool flip);
	bool GetFlipX() const;
	void SetFlipY(bool flip);
	bool GetFlipY() const;

	Color tint = { 255, 255, 255, 255 };
	int renderOrder = 0;
	bool flipX = false;
	bool flipY = false;

	// Hide in API
	//static std::unordered_map<std::filesystem::path, std::pair<Texture2D, int>> textures;

private:
	Sprite* sprite = nullptr;
#if defined(EDITOR)
	bool setup = false;
#endif
	//static std::vector<SpriteRenderer*> spriteRenderers;
	//static bool sorted;
	//std::pair<Texture2D, int>* texture = nullptr;
	//std::filesystem::path texturePath;
};