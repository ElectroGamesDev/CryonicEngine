#pragma once

#include "Components/Component.h"
#include "Core/CryonicCore.h"
#include "Resources/Font.h"
#include "Utilities/FontManager.h"

class Label : public Component
{
public:
	Label(GameObject* obj, int id) : Component(obj, id)
	{
		runInEditor = true;
		name = "Label";
		iconUnicode = "\xef\x80\xb1";

#if defined(EDITOR)
		std::string variables = R"(
        [
            0,
            [
                [
                    "string",
                    "text",
                    "New Text",
                    "Text"
                ],
                [
                    "Color",
                    "color",
                    [ 255, 255, 255, 255 ],
                    "Color"
                ],
                [
                    "Font",
                    "font",
                    "nullptr",
                    "Font",
                    {
                        "Extensions": [".ttf", ".otf"]
                    }
                ],
                [
                    "int",
                    "fontSize",
                    16,
                    "Font Size"
                ]
            ]
        ]
    )";
		exposedVariables = nlohmann::json::parse(variables);
#endif
	}
	Label* Clone() override
	{
		return new Label(gameObject, -1);
	}
	// Hide everything from API
	void Awake() override;
	void RenderGui() override;
#if defined(EDITOR)
	void EditorUpdate() override;
#endif

	void SetText(std::string text);
	std::string GetText() const;
    void SetFontSize(int size);
    int GetFontSize() const;
    void SetFont(Font* font);
    void SetColor(Color color);
    Color GetColor() const;

private:
	Color color = { 255, 255, 255, 255 };
	std::string text;
    Font* font = nullptr;
    int fontSize = 16;
    bool setup = false;
};