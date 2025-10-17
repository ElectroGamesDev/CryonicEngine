#include "Button.h"
#include "Raylib/RaylibWrapper.h"
#include "Components/Misc/CameraComponent.h"
#if defined (EDITOR)
#include "Core/Editor.h"
#else
#include "ThirdParty/imgui/imgui.h"
#endif

void Button::Awake()
{
#if defined(EDITOR)
    image = new Sprite(exposedVariables[1][0][2].get<std::string>()); // Todo: Handle if the path no longer exists
#endif
    //SetSprite(sprite);

#if defined(EDITOR)
    if (exposedVariables[1][9][2].get<std::string>() == "nullptr") // Font
        return;

    font = new Font(exposedVariables[1][9][2].get<std::string>()); // Todo: Handle if the path no longer exists
#else
    if (!font)
        return;
#endif
    SetFont(font);
}

void Button::RenderGui()
{
    Vector2 position = CameraComponent::main->GetWorldToScreen(gameObject->transform.GetPosition());

#if defined(EDITOR)
    // Divding positions by Raylib window size then multiply it by Viewport window size
    position.x = position.x / RaylibWrapper::GetScreenWidth() * ImGui::GetWindowSize().x;
    position.y = position.y / RaylibWrapper::GetScreenHeight() * ImGui::GetWindowSize().y;
#endif

    ImVec2 size = {100, 40};
    Vector2 textPosition = position;
    bool buttonClicked = false;

    position.x -= size.x * gameObject->transform.GetScale().x / 2;
    position.y -= size.y * gameObject->transform.GetScale().y / 2;

    ImGui::SetCursorPos({ position.x, position.y });

    // Image / Button

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0,0});
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

    if (image->GetTexture())
    {
        RaylibWrapper::Texture2D* texture = image->GetTexture();

        Color color = normalColor;
        if (disabled)
            color = disabledColor;
        else
        {
#if !defined(EDITOR)
            bool isHovered = ImGui::IsMouseHoveringRect({ ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y }, { size.x * gameObject->transform.GetScale().x, size.y * gameObject->transform.GetScale().y });
            color = isHovered ? (ImGui::IsMouseDown(ImGuiKey_MouseLeft) ? pressedColor : hoveredColor) : normalColor;
#endif
        }

        if (RaylibWrapper::rlImGuiImageButtonSizeTint(("##" + text + std::to_string(id)).c_str(), texture, { size.x * gameObject->transform.GetScale().x, size.y * gameObject->transform.GetScale().y }, { (float)color.r / 255, (float)color.g / 255, (float)color.b / 255, (float)color.a / 255 }))
            buttonClicked = true;
    }
    else if (image->GetPath() == "Square")
    {
        if (!disabled)
        {
#if defined(EDITOR)
            ImGui::PushStyleColor(ImGuiCol_Button, { (float)normalColor.r / 255, (float)normalColor.g / 255, (float)normalColor.b / 255, (float)normalColor.a / 255 });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { (float)normalColor.r / 255, (float)normalColor.g / 255, (float)normalColor.b / 255, (float)normalColor.a / 255 });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, { (float)normalColor.r / 255, (float)normalColor.g / 255, (float)normalColor.b / 255, (float)normalColor.a / 255 });
#else
            ImGui::PushStyleColor(ImGuiCol_Button, { (float)normalColor.r / 255, (float)normalColor.g / 255, (float)normalColor.b / 255, (float)normalColor.a / 255 });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { (float)hoveredColor.r / 255, (float)hoveredColor.g / 255, (float)hoveredColor.b / 255, (float)hoveredColor.a / 255 });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, { (float)pressedColor.r / 255, (float)pressedColor.g / 255, (float)pressedColor.b / 255, (float)pressedColor.a / 255 });
#endif
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, { (float)disabledColor.r / 255, (float)disabledColor.g / 255, (float)disabledColor.b / 255, (float)disabledColor.a / 255 });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { (float)disabledColor.r / 255, (float)disabledColor.g / 255, (float)disabledColor.b / 255, (float)disabledColor.a / 255 });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, { (float)disabledColor.r / 255, (float)disabledColor.g / 255, (float)disabledColor.b / 255, (float)disabledColor.a / 255 });
        }

#if defined(EDITOR)
        // Todo: This should be changed to an image
        ImGui::Button(("##" + text + std::to_string(id)).c_str(), { size.x * gameObject->transform.GetScale().x, size.y * gameObject->transform.GetScale().y });
#else
        if (ImGui::Button(("##" + text + std::to_string(id)).c_str(), { size.x * gameObject->transform.GetScale().x, size.y * gameObject->transform.GetScale().y }) && !disabled)
            buttonClicked = true;
#endif

        ImGui::PopStyleColor(3);
    }
    //    else if (image->GetPath() == "Circle")
    //    {
    //#if defined(EDITOR)
    //        position.x += Editor::viewportPosition.x;
    //        position.y += Editor::viewportPosition.y;
    //#endif
    //
    //        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(position.x, position.y), gameObject->transform.GetScale().x * 25, IM_COL32(color.r, color.g, color.b, color.a));
    //    }
    else
        if (ImGui::InvisibleButton(("##" + text + std::to_string(id)).c_str(), { size.x * gameObject->transform.GetScale().x, size.y * gameObject->transform.GetScale().y }))
            buttonClicked = true;

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);

    hovered = ImGui::IsItemHovered();

    // Text
    if (font && text != "")
    {
        Color color = textColor;
        if (disabled)
            color = disabledTextColor;

        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        ImGui::SetCursorPos({ textPosition.x - textSize.x / 2, textPosition.y - textSize.y / 2 });
        ImGui::PushFont(FontManager::GetFont(font->GetPath(), fontSize, false));
        ImGui::TextColored({ (float)color.r / 255, (float)color.g / 255, (float)color.b / 255, (float)color.a / 255 }, text.c_str());
        ImGui::PopFont();
    }

    if (buttonClicked)
        for (std::function<void()> callback : clickCallbacks)
            callback();
}

#if defined(EDITOR)
void Button::EditorUpdate()
{
    if (!setup)
    {
        Awake();
        setup = true;
    }

    auto updateColor = [&](Color& color, int index)
    {
        color.r = exposedVariables[1][index][2][0].get<int>();
        color.g = exposedVariables[1][index][2][1].get<int>();
        color.b = exposedVariables[1][index][2][2].get<int>();
        color.a = exposedVariables[1][index][2][3].get<int>();
    };

    updateColor(normalColor, 1);
    updateColor(hoveredColor, 2);
    updateColor(pressedColor, 3);
    updateColor(disabledColor, 4);
    updateColor(textColor, 7);
    updateColor(disabledTextColor, 8);

    disabled = exposedVariables[1][5][2].get<bool>();

    text = exposedVariables[1][6][2].get<std::string>();

    if (image->GetRelativePath() != exposedVariables[1][0][2])
    {
        SetImage(new Sprite(exposedVariables[1][0][2].get<std::string>()));
        // Todo: Should it unload the old sprite?
    }

    if (exposedVariables[1][9][2].get<std::string>() != "nullptr") // Font
    {
        if (fontSize != exposedVariables[1][10][2].get<int>())
            SetFontSize(exposedVariables[1][10][2].get<int>());

        if (!font || font->GetRelativePath() != exposedVariables[1][9][2])
        {
            SetFont(new Font(exposedVariables[1][9][2].get<std::string>()));
            // Todo: Should it unload the old font?
        }
    }
    else
        font = nullptr; // Todo: Should it unload the font if the font != nullptr?
}
#endif

void Button::SetImage(Sprite* image)
{
    this->image = image;
}

void Button::SetNormalColor(Color color)
{
    this->normalColor = color;
}

Color Button::GetNormalColor() const
{
    return normalColor;
}

void Button::SetHoveredColor(Color color)
{
    this->hoveredColor = color;
}

Color Button::GetHoveredColor() const
{
    return hoveredColor;
}

void Button::SetPressedColor(Color color)
{
    this->pressedColor = color;
}

Color Button::GetPressedColor() const
{
    return pressedColor;
}

void Button::SetDisabledColor(Color color)
{
    this->disabledColor = color;
}

Color Button::GetDisabledColor() const
{
    return disabledColor;
}

void Button::SetDisabled(bool disable)
{
    disabled = disable;
}

bool Button::IsDisabled()
{
    return disabled;
}

void Button::SetText(std::string text)
{
    this->text = text;
}
std::string Button::GetText() const
{
    return text;
}
void Button::SetFontSize(int size)
{
    fontSize = size;
    if (font)
        FontManager::LoadFont(font->GetPath(), fontSize, false, true);
}
int Button::GetFontSize() const
{
    return fontSize;
}

void Button::SetFont(Font* font)
{
    this->font = font;
    SetFontSize(fontSize);
}

void Button::SetTextColor(Color color)
{
    this->textColor = color;
}

Color Button::GetTextColor() const
{
    return textColor;
}

void Button::SetDisabledTextColor(Color color)
{
    this->disabledTextColor = color;
}

Color Button::GetDisabledTextColor() const
{
    return disabledTextColor;
}

bool Button::IsHovered() const
{
    return hovered;
}

void Button::AddClickListener(std::function<void()> callback)
{
    clickCallbacks.push_back(callback);
}