#include "Label.h"
#include "Raylib/RaylibDrawWrapper.h"
#include "Raylib/RaylibWrapper.h"
#include "Components/Misc/CameraComponent.h"
#if defined (EDITOR)
#include "ThirdParty/imgui/imgui.h"
#else
#include "ThirdParty/imgui/imgui.h"
#endif

void Label::Awake()
{
#if defined(EDITOR)
    if (exposedVariables[1][2][2].get<std::string>() == "nullptr")
        return;

    font = new Font(exposedVariables[1][2][2].get<std::string>()); // Todo: Handle if the path no longer exists
#else
    if (!font)
        return;
#endif

    SetFont(font);
}

void Label::RenderGui()
{
    if (!font || text.empty())
        return;

    ImGui::PushFont(FontManager::GetFont(font->GetPath(), fontSize, false));
    Vector2 screenPosition = CameraComponent::main->GetWorldToScreen(gameObject->transform.GetPosition());

#if defined(EDITOR)
    // Divding positions by Raylib window size then multiply it by Viewport window size
    screenPosition.x = screenPosition.x / RaylibWrapper::GetScreenWidth() * ImGui::GetWindowSize().x;
    screenPosition.y = screenPosition.y / RaylibWrapper::GetScreenHeight() * ImGui::GetWindowSize().y;
#endif

    // Center the text to the game object
    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    screenPosition.x -= textSize.x / 2;
    screenPosition.y -= textSize.y / 2 - 10;

    ImGui::SetCursorPos({ screenPosition.x, screenPosition.y });
    ImGui::TextColored({ (float)color.r / 255, (float)color.g / 255, (float)color.b / 255, (float)color.a / 255 }, text.c_str());
    ImGui::PopFont();
}

#if defined(EDITOR)
void Label::EditorUpdate()
{
    if (exposedVariables[1][2][2].get<std::string>() == "nullptr")
    {
        // Todo: Should it unload the font if font != nullptr?
        font = nullptr;
        return;
    }

    if (!setup)
    {
        Awake();
        setup = true;
    }

    color.r = exposedVariables[1][1][2][0].get<int>();
    color.g = exposedVariables[1][1][2][1].get<int>();
    color.b = exposedVariables[1][1][2][2].get<int>();
    color.a = exposedVariables[1][1][2][3].get<int>();

    text = exposedVariables[1][0][2].get<std::string>();

    if (fontSize != exposedVariables[1][3][2].get<int>())
        SetFontSize(exposedVariables[1][3][2].get<int>());

    if (!font || font->GetRelativePath() != exposedVariables[1][2][2])
    {
        SetFont(new Font(exposedVariables[1][2][2].get<std::string>()));
        // Todo: Should it unload the old font?
    }
}
#endif

void Label::SetText(std::string text)
{
    this->text = text;
}
std::string Label::GetText() const
{
    return text;
}
void Label::SetFontSize(int size)
{
    fontSize = size;
    if (font)
        FontManager::LoadFont(font->GetPath(), fontSize, false, true);
}
int Label::GetFontSize() const
{
    return fontSize;
}

void Label::SetFont(Font* font)
{
    this->font = font;
    SetFontSize(fontSize);
}

void Label::SetColor(Color color)
{
    this->color = color;
}

Color Label::GetColor() const
{
    return color;
}
