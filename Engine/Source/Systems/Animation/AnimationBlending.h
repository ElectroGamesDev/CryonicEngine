#pragma once
#include <vector>
#include <functional>
#include "Resources/Animation.h"

class AnimationBlending
{
public:
	// Blending modes
	enum BlendMode
	{
		Linear,
		EaseIn,
		EaseOut,
		EaseInOut,
		Cubic,
		Exponential
	};

	// Blend between two animation frames
	static float BlendFloat(float a, float b, float t, BlendMode mode = Linear);
	static int BlendSpriteIndex(int a, int b, float t);

	// Easing functions
	static float EaseInQuad(float t);
	static float EaseOutQuad(float t);
	static float EaseInOutQuad(float t);
	static float EaseInCubic(float t);
	static float EaseOutCubic(float t);
	static float EaseInOutCubic(float t);
	static float EaseInExpo(float t);
	static float EaseOutExpo(float t);

	// 2D blend space (for directional animations like walk/run in 8 directions)
	struct BlendSpace2D
	{
		struct BlendSample
		{
			Animation* animation;
			float x; // e.g., forward/backward
			float y; // e.g., left/right
			float weight;
		};

		std::vector<BlendSample> samples;

		void AddSample(Animation* anim, float x, float y);
		void CalculateWeights(float inputX, float inputY);
		Animation* GetDominantAnimation();
		std::vector<BlendSample*> GetActiveAnimations(float threshold = 0.01f);
	};

	// 1D blend space (for speed-based blending like idle->walk->run)
	struct BlendSpace1D
	{
		struct BlendSample
		{
			Animation* animation;
			float position; // e.g., 0=idle, 0.5=walk, 1.0=run
			float weight;
		};

		std::vector<BlendSample> samples;

		void AddSample(Animation* anim, float position);
		void CalculateWeights(float input);
		Animation* GetDominantAnimation();
		std::vector<BlendSample*> GetActiveAnimations(float threshold = 0.01f);
	};
};