#pragma once
#include <string>
#include <vector>
#include "Sprite.h"

class Animation
{
public:
	int GetId() const
	{
		return id;
	}

	std::string GetName() const
	{
		return name;
	}

	void SetLooped(bool loop)
	{
		this->loop = loop;
	}

	bool IsLooped() const
	{
		return loop;
	}

	void SetSpeed(float speed)
	{
		this->speed = speed;
	}

	float GetSpeed() const
	{
		return speed;
	}

	const std::vector<Sprite*>& GetSprites() const
	{
		return sprites;
	}

	// 3D animation support (for GLTF/GLB models)
	void SetAnimationIndex(int index)
	{
		animationIndex = index;
	}

	int GetAnimationIndex() const
	{
		return animationIndex;
	}

	void SetModelPath(const std::string& path)
	{
		modelPath = path;
	}

	std::string GetModelPath() const
	{
		return modelPath;
	}

	// Check animation type
	bool Is3DAnimation() const
	{
		return animationIndex >= 0 && !modelPath.empty();
	}

	bool Is2DAnimation() const
	{
		return !sprites.empty();
	}

	// Public members (Hide in API)
	int id = 0;
	std::string name;
	bool loop = false;
	float speed = 1.0f;
	std::vector<Sprite*> sprites;

	// 3D animation data (for GLTF/GLB models)
	int animationIndex = -1;
	std::string modelPath;
};