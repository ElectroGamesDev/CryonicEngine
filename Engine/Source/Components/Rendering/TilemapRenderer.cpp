#include "TilemapRenderer.h"
#include "Raylib/RaylibDrawWrapper.h"
#include "Core/GameObject.h"


void TilemapRenderer::Awake()
{
#if defined(EDITOR)
    tilemap = new Tilemap(exposedVariables[1][0][2].get<std::string>()); // Todo: Handle if the path no longer exists
#endif

    textures.push_back(this);
    sorted = false;
}

void TilemapRenderer::Start()
{
    if (sorted)
        return;

    SortTextures();
    sorted = true;
}

void TilemapRenderer::SetTilemap(Tilemap* tilemap)
{
    if (this->tilemap != nullptr)
    {
        // Todo: If its in editor, then see if it can be unloaded
        //if (path == texturePath)
        //    return;
        //Destroy();
    }

    this->tilemap = tilemap;

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

Tilemap* TilemapRenderer::GetTilemap()
{
    return tilemap;
}

void TilemapRenderer::SetRenderOrder(int order)
{
    renderOrder = order;

    SortTextures();
}

int TilemapRenderer::GetRenderOrder() const
{
    return renderOrder;
}

void TilemapRenderer::Render()
{
    if (!tilemap || !gameObject->IsActive() || !gameObject->IsGlobalActive() || !IsActive())
        return;

    int i = 0;
    for (const auto& tile : *(tilemap->GetTiles()))
    {
        i++;
        if (tile.tileset != nullptr)
        {
            //ConsoleLogger::InfoLog("Rendering " + std::to_string(i));
            //ConsoleLogger::InfoLog("Rendering " + std::to_string(tile.texturePosition.x) + ", " + std::to_string(tile.texturePosition.y) + " at " + std::to_string(tile.position.x) + ", " + std::to_string(tile.position.y));

            RaylibWrapper::DrawTextureProFlipped({ tile.tileset->id, tile.tileset->width, tile.tileset->height, tile.tileset->mipmaps, tile.tileset->format },
                tile.texturePosition,
                { 0, 0, tile.size.x / 10, tile.size.y / 10 },
                { (tile.position.x * -1) + gameObject->transform.GetPosition().x / 10 / 2, tile.position.y + gameObject->transform.GetPosition().y / 10 / 2 },
                0,
                { 255, 255, 255, 255 }
            );

            //RaylibWrapper::DrawTextureProFlipped(tempTexture,
            //    tile.texturePosition,
            //    { gameObject->transform.GetPosition().x, gameObject->transform.GetPosition().y, tile.size.x * gameObject->transform.GetScale().x / 10, tile.size.y * gameObject->transform.GetScale().y / 10 },
            //    { tile.size.x * gameObject->transform.GetScale().x / 10 / 2, tile.size.y * gameObject->transform.GetScale().y / 10 / 2 },
            //    0,
            //    { 255, 255, 255, 255 }
            //);

            //RaylibWrapper::DrawTextureProFlipped(tempTexture,
            //    {0, 0, 16, 16},
            //    { gameObject->transform.GetPosition().x, gameObject->transform.GetPosition().y, tile.size.x * gameObject->transform.GetScale().x, tile.size.y * gameObject->transform.GetScale().y },
            //    { tile.size.x * gameObject->transform.GetScale().x, tile.size.y * gameObject->transform.GetScale().y },
            //    0,
            //    { 255, 255, 255, 255 }
            //);
        }
        else
        {
            // Todo: Invalid tilemap texture path
        }
    }

}

#if defined(EDITOR)
void TilemapRenderer::EditorUpdate()
{
    if (!setup)
    {
        Awake();
        setup = true;
    }

    if (tilemap && tilemap->GetRelativePath() != exposedVariables[1][0][2])
    {
        SetTilemap(new Tilemap(exposedVariables[1][0][2].get<std::string>()));
        if (tilemap->GetRelativePath() != exposedVariables[1][0][2])
            exposedVariables[1][0][2] = tilemap->GetRelativePath();

        // Todo: It should unload the tilemap if its not being used somewhere else
    }

    if (renderOrder != exposedVariables[1][1][2].get<int>())
    {
        renderOrder = exposedVariables[1][1][2].get<int>();
        SortTextures();
    }
}
#endif

void TilemapRenderer::Destroy()
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