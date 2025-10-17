#include "SpriteRenderer.h"
#include "Raylib/RaylibDrawWrapper.h"
#include "Core/GameObject.h"

//std::vector<SpriteRenderer*> SpriteRenderer::spriteRenderers;
//bool SpriteRenderer::sorted = true;

void SpriteRenderer::Awake()
{
#if defined(EDITOR)
    if (!sprite)
        sprite = new Sprite(exposedVariables[1][0][2].get<std::string>()); // Todo: Handle if the path no longer exists
#endif

    textures.push_back(this);
    sorted = false;
}

void SpriteRenderer::Start()
{
    if (sorted)
        return;

    RenderableTexture::SortTextures();
    sorted = true;
}

bool SpriteRenderer::IntersectsPoint(Vector2 pos)
{
    if (!sprite)
        return false;

    RaylibWrapper::Texture2D* texture = sprite->GetTexture();

    if (!texture)
        return false;

    Vector2 size = { gameObject->transform.GetScale().x, gameObject->transform.GetScale().y };

    // TODO: Rotation needs to be considered
    // Todo: Currently not working
    return RaylibWrapper::CheckCollisionRecs(
        { pos.x, pos.y, 1, 1 },
        { gameObject->transform.GetPosition().x - size.x / 2, gameObject->transform.GetPosition().y - size.y / 2, size.x, size.y } // Subtracting by size / 2 so it uses the correct position since the sprite is centered
    );
}

void SpriteRenderer::SetSprite(Sprite* sprite)
{
    if (this->sprite != nullptr)
    {
        // Todo: If its in editor, then see if it can be unloaded
        //if (path == texturePath)
        //    return;
        //Destroy();
    }

    this->sprite = sprite;

#if defined (EDITOR)
    exposedVariables[1][0][2] = sprite->GetRelativePath();
#endif

    // Todo: Check to make sure the texture file exists, if it doesn't then replace it with some kind of NULL txture

    //if (auto it = textures.find(path); it != textures.end())
    //{
    //    texture = &(it->second);
    //    texture->second++;
    //}
    //else
    //{
    //    RaylibWrapper::Texture2D tempTexture = RaylibWrapper::LoadTexture(path.string().c_str());
    //    textures[path] = std::make_pair(Texture2D{ tempTexture.id, tempTexture.width, tempTexture.height, tempTexture.mipmaps, tempTexture.format }, 1);
    //    texture = &textures[path];
    //}

    //texturePath = path;
}

Sprite* SpriteRenderer::GetSprite()
{
    return sprite;
}

void SpriteRenderer::SetTint(Color tint)
{
    this->tint = tint;
}

Color SpriteRenderer::GetTint() const
{
    return tint;
}

void SpriteRenderer::SetRenderOrder(int order)
{
    renderOrder = order;

    SortTextures();
}

int SpriteRenderer::GetRenderOrder() const
{
    return renderOrder;
}

void SpriteRenderer::SetFlipX(bool flip)
{
    flipX = flip;
}

bool SpriteRenderer::GetFlipX() const
{
    return flipX;
}

void SpriteRenderer::SetFlipY(bool flip)
{
    flipY = flip;
}

bool SpriteRenderer::GetFlipY() const
{
    return flipY;
}

void SpriteRenderer::Render()
{
    if (!sprite || !gameObject->IsActive() || !gameObject->IsGlobalActive() || !IsActive())
        return;

    // Todo: I should create an enum to store whether if its a sqaure, circle, etc, if its a shape. It will be faster than comparing strings in an if-else
    if (sprite->GetTexture() != nullptr)
    {
        int xFlip = flipX ? -1 : 1;
        int yFlip = flipY ? 1 : -1; // The Y is already flipped

        RaylibWrapper::Texture2D* texture = sprite->GetTexture();

        RaylibWrapper::DrawTextureProFlipped({ texture->id, texture->width, texture->height, texture->mipmaps, texture->format },
            { 0, 0, static_cast<float>(texture->width) * xFlip, static_cast<float>(texture->height) * yFlip },
            { gameObject->transform.GetPosition().x, gameObject->transform.GetPosition().y, texture->width * gameObject->transform.GetScale().x / 10, texture->height * gameObject->transform.GetScale().y / 10 },
            { texture->width * gameObject->transform.GetScale().x / 10 / 2, texture->height * gameObject->transform.GetScale().y / 10 / 2 },
            gameObject->transform.GetRotationEuler().z,
            { tint.r, tint.g, tint.b, tint.a });
    }
    else if (sprite->GetPath() == "Square")
    {
        Vector3 position = gameObject->transform.GetPosition();
        Vector3 scale = gameObject->transform.GetScale();
        //DrawRectangleWrapper(position.x, position.y, scale.x, scale.y, gameObject->transform.GetRotationEuler().z, tint.r, tint.g, tint.b, tint.a);
        RaylibWrapper::DrawRectangleProFlipped({ position.x, position.y, scale.x * 3, scale.y * 3 },
            {
                scale.x * 3 / 2,
                scale.y * 3 / 2
            },
            gameObject->transform.GetRotationEuler().z,
            { tint.r, tint.g, tint.b, tint.a });
    }
    else if (sprite->GetPath() == "Circle")
    {
        Vector3 position = gameObject->transform.GetPosition();
        //DrawCircleWrapper(position.x, position.y, gameObject->transform.GetScale().x, tint.r, tint.g, tint.b, tint.a);
        RaylibWrapper::DrawCircleSectorFlipped({ position.x, position.y }, gameObject->transform.GetScale().x * 1.5f, 0, 360, 36, { tint.r, tint.g, tint.b, tint.a });
    }
    else
    {
        // Todo: Invalid texture path
    }

}

#if defined(EDITOR)
void SpriteRenderer::EditorUpdate()
{
    if (!setup)
    {
        Awake();
        setup = true;
    }

    if (sprite && sprite->GetRelativePath() != exposedVariables[1][0][2])
    {
        SetSprite(new Sprite(exposedVariables[1][0][2].get<std::string>()));
        // Todo: Should it unload the old sprite?
    }

    tint.r = exposedVariables[1][1][2][0].get<int>();
    tint.g = exposedVariables[1][1][2][1].get<int>();
    tint.b = exposedVariables[1][1][2][2].get<int>();
    tint.a = exposedVariables[1][1][2][3].get<int>();

    if (renderOrder != exposedVariables[1][2][2].get<int>())
    {
        renderOrder = exposedVariables[1][2][2].get<int>();
        SortTextures();
    }

    flipX = exposedVariables[1][3][2].get<bool>();
    flipY = exposedVariables[1][4][2].get<bool>();
}
#endif

void SpriteRenderer::Destroy()
{
    auto it = std::find(textures.begin(), textures.end(), this);
    if (it != textures.end())
    {
        textures.erase(it);
        SortTextures();
    }
//    if (texture == nullptr)
//        return;
//
//    texture->second--;
//    if (texture->second <= 0)
//    {
//        RaylibWrapper::UnloadTexture({ texture->first.id, texture->first.width, texture->first.height, texture->first.mipmaps, texture->first.format });
//        auto it = textures.begin();
//        while (it != textures.end())
//        {
//            if (&it->second == texture)
//            {
//                it = textures.erase(it);
//                break;
//            }
//            else
//                ++it;
//        }
//    }
//    texture = nullptr;
}