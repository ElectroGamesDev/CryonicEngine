#include "AnimationBlending.h"
#include <cmath>
#include <algorithm>

float AnimationBlending::BlendFloat(float a, float b, float t, BlendMode mode)
{
	// Clamp t
	t = std::max(0.0f, std::min(1.0f, t));

	float easedT = t;
	switch (mode)
	{
	case EaseIn:
		easedT = EaseInQuad(t);
		break;
	case EaseOut:
		easedT = EaseOutQuad(t);
		break;
	case EaseInOut:
		easedT = EaseInOutQuad(t);
		break;
	case Cubic:
		easedT = EaseInOutCubic(t);
		break;
	case Exponential:
		easedT = EaseInExpo(t);
		break;
	case Linear:
	default:
		break;
	}

	return a + (b - a) * easedT;
}

int AnimationBlending::BlendSpriteIndex(int a, int b, float t)
{
	// For sprite indices, we do a simple crossfade at 0.5
	return (t < 0.5f) ? a : b;
}

float AnimationBlending::EaseInQuad(float t)
{
	return t * t;
}

float AnimationBlending::EaseOutQuad(float t)
{
	return t * (2.0f - t);
}

float AnimationBlending::EaseInOutQuad(float t)
{
	return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

float AnimationBlending::EaseInCubic(float t)
{
	return t * t * t;
}

float AnimationBlending::EaseOutCubic(float t)
{
	float f = t - 1.0f;
	return f * f * f + 1.0f;
}

float AnimationBlending::EaseInOutCubic(float t)
{
	return t < 0.5f ? 4.0f * t * t * t : 1.0f + (--t) * 4.0f * t * t;
}

float AnimationBlending::EaseInExpo(float t)
{
	return t == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * (t - 1.0f));
}

float AnimationBlending::EaseOutExpo(float t)
{
	return t == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
}

// BlendSpace2D Implementation
void AnimationBlending::BlendSpace2D::AddSample(Animation* anim, float x, float y)
{
	BlendSample sample;
	sample.animation = anim;
	sample.x = x;
	sample.y = y;
	sample.weight = 0.0f;
	samples.push_back(sample);
}

void AnimationBlending::BlendSpace2D::CalculateWeights(float inputX, float inputY)
{
	if (samples.empty())
		return;

	float totalWeight = 0.0f;

	// Calculate distances and weights
	for (auto& sample : samples)
	{
		float dx = sample.x - inputX;
		float dy = sample.y - inputY;
		float distance = std::sqrt(dx * dx + dy * dy);

		// Inverse distance weighting (closer samples have higher weight)
		if (distance < 0.001f)
		{
			// Exact match
			sample.weight = 1.0f;
			totalWeight = 1.0f;

			// Zero out other samples
			for (auto& other : samples)
			{
				if (&other != &sample)
					other.weight = 0.0f;
			}
			return;
		}
		else
		{
			sample.weight = 1.0f / (distance * distance);
			totalWeight += sample.weight;
		}
	}

	// Normalize weights
	if (totalWeight > 0.0f)
	{
		for (auto& sample : samples)
			sample.weight /= totalWeight;
	}
}

Animation* AnimationBlending::BlendSpace2D::GetDominantAnimation()
{
	if (samples.empty())
		return nullptr;

	auto maxIt = std::max_element(samples.begin(), samples.end(),
		[](const BlendSample& a, const BlendSample& b) {
			return a.weight < b.weight;
		});

	return maxIt->animation;
}

std::vector<AnimationBlending::BlendSpace2D::BlendSample*> AnimationBlending::BlendSpace2D::GetActiveAnimations(float threshold)
{
	std::vector<BlendSample*> active;

	for (auto& sample : samples)
	{
		if (sample.weight >= threshold)
			active.push_back(&sample);
	}

	return active;
}

// BlendSpace1D Implementation
void AnimationBlending::BlendSpace1D::AddSample(Animation* anim, float position)
{
	BlendSample sample;
	sample.animation = anim;
	sample.position = position;
	sample.weight = 0.0f;
	samples.push_back(sample);

	// Sort by position
	std::sort(samples.begin(), samples.end(),
		[](const BlendSample& a, const BlendSample& b) {
			return a.position < b.position;
		});
}

void AnimationBlending::BlendSpace1D::CalculateWeights(float input)
{
	if (samples.empty())
		return;

	// Clamp input
	float minPos = samples.front().position;
	float maxPos = samples.back().position;
	input = std::max(minPos, std::min(maxPos, input));

	// Reset weights
	for (auto& sample : samples)
		sample.weight = 0.0f;

	// Find surrounding samples
	for (size_t i = 0; i < samples.size(); ++i)
	{
		if (std::abs(samples[i].position - input) < 0.001f)
		{
			// Exact match
			samples[i].weight = 1.0f;
			return;
		}

		if (i < samples.size() - 1 && input >= samples[i].position && input <= samples[i + 1].position)
		{
			// Linear interpolation between two samples
			float range = samples[i + 1].position - samples[i].position;
			float t = (input - samples[i].position) / range;

			samples[i].weight = 1.0f - t;
			samples[i + 1].weight = t;
			return;
		}
	}

	// Edge cases: use closest sample
	if (input < samples.front().position)
		samples.front().weight = 1.0f;
	else if (input > samples.back().position)
		samples.back().weight = 1.0f;
}

Animation* AnimationBlending::BlendSpace1D::GetDominantAnimation()
{
	if (samples.empty())
		return nullptr;

	auto maxIt = std::max_element(samples.begin(), samples.end(),
		[](const BlendSample& a, const BlendSample& b) {
			return a.weight < b.weight;
		});

	return maxIt->animation;
}

std::vector<AnimationBlending::BlendSpace1D::BlendSample*> AnimationBlending::BlendSpace1D::GetActiveAnimations(float threshold)
{
	std::vector<BlendSample*> active;

	for (auto& sample : samples)
	{
		if (sample.weight >= threshold)
			active.push_back(&sample);
	}

	return active;
}