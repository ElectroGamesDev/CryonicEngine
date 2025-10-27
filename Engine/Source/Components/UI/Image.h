#pragma once

#include "UIElement.h"
#include "Core/CryonicCore.h"
#include "Resources/Sprite.h"
#include "RectTransform.h"

class Image : public UIElement
{
public:
    Image(GameObject* obj, int id) : UIElement(obj, id)
	{
		runInEditor = true;
		name = "Image";
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
                    "color",
                    [ 255, 255, 255, 255 ],
                    "Color"
                ]
            ]
        ]
    )";
		exposedVariables = nlohmann::json::parse(variables);
#endif
	}
    Image* Clone() override
	{
		return new Image(gameObject, -1);
	}
	// Hide everything from API
	void Awake() override;
	void RenderGui() override;
#if defined(EDITOR)
	void EditorUpdate() override;
#endif

    void SetSprite(Sprite* sprite);
    void SetColor(Color color);
    Color GetColor() const;

private:
    Sprite* sprite = nullptr;
	Color color = { 255, 255, 255, 255 };
    bool setup = false;
	RectTransform* rectTransform;
};