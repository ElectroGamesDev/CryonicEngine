#include "RenderableTexture.h"
#include <algorithm>

std::vector<RenderableTexture*> RenderableTexture::textures;
bool RenderableTexture::sorted = true;

void RenderableTexture::SortTextures()
{
    std::sort(textures.begin(), textures.end(), [](const RenderableTexture* a, const RenderableTexture* b) {
        return a->GetRenderOrder() < b->GetRenderOrder();
        });
}