#include "Image.h"
#include "Raylib/RaylibWrapper.h"
#include "Components/Misc/CameraComponent.h"
#if defined (EDITOR)
#include "Core/Editor.h"
#else
#include "ThirdParty/imgui/imgui.h"
#endif

void Image::Awake()
{
#if defined(EDITOR)
    sprite = new Sprite(exposedVariables[1][0][2].get<std::string>()); // Todo: Handle if the path no longer exists
#endif
    //SetSprite(sprite);
}

void Image::RenderGui()
{
    Vector2 position = CameraComponent::main->GetWorldToScreen(gameObject->transform.GetPosition());

#if defined(EDITOR)
    // Divding positions by Raylib window size then multiply it by Viewport window size
    position.x = position.x / RaylibWrapper::GetScreenWidth() * ImGui::GetWindowSize().x;
    position.y = position.y / RaylibWrapper::GetScreenHeight() * ImGui::GetWindowSize().y;
#endif

    if (sprite->GetTexture())
    {
        RaylibWrapper::Texture2D* texture = sprite->GetTexture();

        position.x -= texture->width * gameObject->transform.GetScale().x / 2;
        position.y -= texture->height * gameObject->transform.GetScale().y / 2;

        ImGui::SetCursorPos({ position.x, position.y });

        //RaylibWrapper::Texture2D raylibTexture = { texture->id, texture->width, texture->height, texture->mipmaps, texture->format };
        //RaylibWrapper::rlImGuiImageSizeTintV(&raylibTexture, { texture->width * gameObject->transform.GetScale().x, texture->height * gameObject->transform.GetScale().y }, { (float)color.r / 255, (float)color.g / 255, (float)color.b / 255, (float)color.a / 255 });
        RaylibWrapper::rlImGuiImageSizeV(texture, { texture->width * gameObject->transform.GetScale().x, texture->height * gameObject->transform.GetScale().y });
    }
    else if (sprite->GetPath() == "Square")
    {
#if defined(EDITOR)
        position.x += Editor::viewportPosition.x;
        position.y += Editor::viewportPosition.y;
#endif

        ImVec2 min = ImVec2(gameObject->transform.GetScale().x * 50 / 2, gameObject->transform.GetScale().y * 50 / 2);
        ImVec2 max = ImVec2(gameObject->transform.GetScale().x * 50 / 2, gameObject->transform.GetScale().y * 50 / 2);
        min = ImVec2(position.x - min.x, position.y - min.y);
        max = ImVec2(position.x + max.x, position.y + max.y);

        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x, min.y), ImVec2(max.x, max.y), IM_COL32(color.r, color.g, color.b, color.a));
    }
    else if (sprite->GetPath() == "Circle")
    {
#if defined(EDITOR)
        position.x += Editor::viewportPosition.x;
        position.y += Editor::viewportPosition.y;
#endif

        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(position.x, position.y), gameObject->transform.GetScale().x * 25, IM_COL32(color.r, color.g, color.b, color.a));
    }
}

#if defined(EDITOR)
void Image::EditorUpdate()
{
    if (!setup)
    {
        Awake();
        setup = true;
    }

    color.r = exposedVariables[1][1][2][0].get<int>();
    color.g = exposedVariables[1][1][2][1].get<int>();
    color.b = exposedVariables[1][1][2][2].get<int>();
    color.a = exposedVariables[1][1][2][3].get<int>();

    if (sprite->GetRelativePath() != exposedVariables[1][0][2])
    {
        SetSprite(new Sprite(exposedVariables[1][0][2].get<std::string>()));
        // Todo: Should it unload the old sprite?
    }
}
#endif

void Image::SetSprite(Sprite* sprite)
{
    this->sprite = sprite;
}

void Image::SetColor(Color color)
{
    this->color = color;
}

Color Image::GetColor() const
{
    return color;
}
