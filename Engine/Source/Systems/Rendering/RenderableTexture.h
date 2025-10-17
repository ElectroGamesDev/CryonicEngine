#pragma once

#include <vector>

class RenderableTexture
{
public:
	virtual int GetRenderOrder() const = 0;
	virtual void Render() = 0;
	static void SortTextures();

	// Hide in API
	static std::vector<RenderableTexture*> textures;
	static bool sorted;
};