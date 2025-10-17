#pragma once

#include "Components/Component.h"
#include "Core/CryonicCore.h"
#include "Utilities/FontManager.h"
#include "Resources/Sprite.h"
#include "Resources/Font.h"

class Button : public Component
{
public:
    Button(GameObject* obj, int id) : Component(obj, id)
	{
		runInEditor = true;
		name = "Button";

#if defined(EDITOR)
		std::string variables = R"(
        [
            0,
            [
                [
                    "Sprite",
                    "image",
                    "Square",
                    "Image",
                    {
                        "Extensions": [".png", ".jpg", ".jpeg"]
                    }
                ],
                [
                    "Color",
                    "normalColor",
                    [ 200, 200, 200, 255 ],
                    "Normal Color"
                ],
                [
                    "Color",
                    "hoveredColor",
                    [ 210, 210, 210, 255 ],
                    "Hovered Color"
                ],
                [
                    "Color",
                    "pressedColor",
                    [ 190, 190, 190, 255 ],
                    "Pressed Color"
                ],
                [
                    "Color",
                    "disabledColor",
                    [ 125, 125, 125, 255 ],
                    "Disabled Color"
                ],
                [
                    "bool",
                    "disabled",
                    false,
                    "Disabled"
                ],
                [
                    "string",
                    "text",
                    "New Button",
                    "Text"
                ],
                [
                    "Color",
                    "textColor",
                    [ 0, 0, 0, 255 ],
                    "Text Color"
                ],
                [
                    "Color",
                    "disabledTextColor",
                    [ 60, 60, 60, 200 ],
                    "Disabled Text Color"
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
    Button* Clone() override
	{
		return new Button(gameObject, -1);
	}
	// Hide everything from API
	void Awake() override;
	void RenderGui() override;
#if defined(EDITOR)
	void EditorUpdate() override;
#endif

    void SetImage(Sprite* sprite);
    void SetNormalColor(Color color);
    Color GetNormalColor() const;
    void SetHoveredColor(Color color);
    Color GetHoveredColor() const;
    void SetPressedColor(Color color);
    Color GetPressedColor() const;
    void SetDisabledColor(Color color);
    Color GetDisabledColor() const;
    void SetDisabled(bool disable);
    bool IsDisabled();
    void SetText(std::string text);
    std::string GetText() const;
    void SetFontSize(int size);
    int GetFontSize() const;
    void SetFont(Font* font);
    void SetTextColor(Color color);
    Color GetTextColor() const;
    void SetDisabledTextColor (Color color);
    Color GetDisabledTextColor() const;
    bool IsHovered() const;

    // This only works for static functions and lambda
    void AddClickListener(std::function<void()> callback);

    template <typename T, typename U>
    void AddClickListener(T* obj, void (U::* func)()) {
        clickCallbacks.emplace_back([obj, func] { (obj->*func)(); });
    }

private:
    Sprite* image = nullptr;
    Color normalColor = { 255, 255, 255, 255 };
    Color hoveredColor = { 255, 255, 255, 255 };
    Color pressedColor = { 255, 255, 255, 255 };
    Color disabledColor = { 255, 255, 255, 255 };
    bool disabled;
    std::string text;
    Color textColor = { 255, 255, 255, 255 };
    Color disabledTextColor = { 255, 255, 255, 255 };
    Font* font = nullptr;
    int fontSize = 16;
    bool hovered = false;
    std::vector<std::function<void()>> clickCallbacks;
    bool setup = false;
};