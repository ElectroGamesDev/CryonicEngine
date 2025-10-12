#pragma once
#include "Component.h"
#include "../AnimationGraph.h"
#include "SpriteRenderer.h"
#include <memory>

class AnimationPlayer : public Component
{
public:
	AnimationPlayer(GameObject* obj, int id) : Component(obj, id)
	{
		name = "AnimationPlayer";
		iconUnicode = "\xef\x9c\x8c";
#if defined(EDITOR)
		std::string variables = R"(
        [
            0,
            [
                [
                    "AnimationGraph",
                    "animationGraphPath",
                    "",
                    "Animation Graph",
                    {
                        "Extensions": [".animgraph"]
                    }
                ]
            ]
        ]
    )";
		exposedVariables = nlohmann::json::parse(variables);
#endif
	}

	void Awake() override;
	void Update() override;
	void Destroy() override;

	// Playback control
	void Pause();
	void Unpause();
	bool IsPaused();
	void Stop();
	void Play();

	// Graph management
	void SetAnimationGraph(std::shared_ptr<AnimationGraph> graph);
	void SetAnimationGraph(const std::string& path);
	std::shared_ptr<AnimationGraph> GetAnimationGraph();

	// Animation state control
	void SetActiveAnimation(const std::string& name);
	void SetActiveAnimation(Animation* animation);
	Animation* GetActiveAnimation();
	void CrossFade(const std::string& name, float duration);
	void CrossFade(Animation* animation, float duration);

	// Parameter forwarding to graph
	void SetBool(const std::string& name, bool value);
	void SetInt(const std::string& name, int value);
	void SetFloat(const std::string& name, float value);
	void SetTrigger(const std::string& name);

	bool GetBool(const std::string& name, bool defaultValue = false);
	int GetInt(const std::string& name, int defaultValue = 0);
	float GetFloat(const std::string& name, float defaultValue = 0.0f);

	// Animation info
	float GetNormalizedTime() const;
	float GetCurrentTime() const;
	float GetAnimationDuration() const;
	bool IsTransitioning() const { return isBlending; }

	bool paused = false;

protected:
	struct BlendState
	{
		AnimationGraph::AnimationState* fromState = nullptr;
		AnimationGraph::AnimationState* toState = nullptr;
		float blendTime = 0.0f;
		float blendDuration = 0.3f;
		int fromSpriteIndex = 0;
		int toSpriteIndex = 0;
		float fromTime = 0.0f;
		float toTime = 0.0f;
	};

	SpriteRenderer* spriteRenderer = nullptr;
	std::shared_ptr<AnimationGraph> animationGraph;
	AnimationGraph::AnimationState* activeAnimationState = nullptr;
	bool previouslyExisted = true;
	float timeElapsed = 0.0f;
	int previousSprite = -1;
	bool isPlaying = true;
	bool isBlending = false;
	BlendState blendState;
	std::string animationGraphPath; // For editor serialization

	void UpdateAnimation(float deltaTime);
	void UpdateBlending(float deltaTime);
	void CheckTransitions();
	void SetSpriteFromAnimation(AnimationGraph::AnimationState* state, int index);
	void StartTransition(AnimationGraph::AnimationState* toState, float duration);
	int GetCurrentSpriteIndex(AnimationGraph::AnimationState* state, float time);
};