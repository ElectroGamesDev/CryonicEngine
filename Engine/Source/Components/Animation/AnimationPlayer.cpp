#include "AnimationPlayer.h"
#ifndef EDITOR
#include "Game.h"
#endif

void AnimationPlayer::Awake()
{
	spriteRenderer = gameObject->GetComponent<SpriteRenderer>();

	// Load animation graph from path if set
	if (!animationGraphPath.empty() && animationGraph == nullptr)
	{
		SetAnimationGraph(animationGraphPath);
	}

	if (animationGraph != nullptr)
	{
		activeAnimationState = animationGraph->GetStartAnimationState();
		timeElapsed = 0.0f;
		previousSprite = -1;
	}
}

void AnimationPlayer::Update()
{
	if (!isPlaying || paused || animationGraph == nullptr)
		return;

	if (spriteRenderer == nullptr)
	{
		spriteRenderer = gameObject->GetComponent<SpriteRenderer>();
		if (spriteRenderer == nullptr)
		{
			if (previouslyExisted)
			{
				previouslyExisted = false;
				ConsoleLogger::WarningLog(gameObject->GetName() + ":Animation Player - Game object does not have a Sprite Renderer. Add one or remove/disable the Animation Player.", false);
			}
			return;
		}
	}

	previouslyExisted = true;

	if (isBlending)
	{
		UpdateBlending(deltaTime);
	}
	else
	{
		UpdateAnimation(deltaTime);
		CheckTransitions();
	}
}

void AnimationPlayer::UpdateAnimation(float deltaTime)
{
	if (!activeAnimationState || !spriteRenderer->IsActive())
		return;

	const auto& sprites = activeAnimationState->animation.GetSprites();
	if (sprites.empty())
		return;

	float animSpeed = activeAnimationState->animation.GetSpeed();
	if (animSpeed <= 0.0f)
		return;

	float animDuration = animSpeed;

	// Update time
	timeElapsed += deltaTime;

	// Handle looping or animation end
	if (timeElapsed >= animDuration)
	{
		if (activeAnimationState->animation.IsLooped())
		{
			timeElapsed = fmod(timeElapsed, animDuration);
		}
		else
		{
			timeElapsed = animDuration;
			// Animation finished, check for transitions
			CheckTransitions();
			return;
		}
	}

	// Calculate sprite index
	int index = GetCurrentSpriteIndex(activeAnimationState, timeElapsed);

	if (index != previousSprite && index >= 0 && index < sprites.size())
	{
		SetSpriteFromAnimation(activeAnimationState, index);
		previousSprite = index;
	}
}

void AnimationPlayer::UpdateBlending(float deltaTime)
{
	if (!spriteRenderer->IsActive())
		return;

	blendState.blendTime += deltaTime;
	float blendFactor = blendState.blendTime / blendState.blendDuration;

	if (blendFactor >= 1.0f)
	{
		// Blend complete
		isBlending = false;
		activeAnimationState = blendState.toState;
		timeElapsed = blendState.toTime;
		previousSprite = blendState.toSpriteIndex;
		SetSpriteFromAnimation(activeAnimationState, previousSprite);
		return;
	}

	// Update both animations
	blendState.fromTime += deltaTime;
	blendState.toTime += deltaTime;

	// For sprite animation, we'll do a simple crossfade by switching at 50%
	// More sophisticated blending would require shader support
	if (blendFactor < 0.5f)
	{
		int fromIndex = GetCurrentSpriteIndex(blendState.fromState, blendState.fromTime);
		if (fromIndex != blendState.fromSpriteIndex)
		{
			SetSpriteFromAnimation(blendState.fromState, fromIndex);
			blendState.fromSpriteIndex = fromIndex;
		}
	}
	else
	{
		int toIndex = GetCurrentSpriteIndex(blendState.toState, blendState.toTime);
		if (toIndex != blendState.toSpriteIndex)
		{
			SetSpriteFromAnimation(blendState.toState, toIndex);
			blendState.toSpriteIndex = toIndex;
		}
	}
}

void AnimationPlayer::CheckTransitions()
{
	if (!activeAnimationState || !animationGraph)
		return;

	float normalizedTime = GetNormalizedTime();
	AnimationGraph::Transition* transition = animationGraph->CheckTransitions(activeAnimationState, normalizedTime);

	if (transition)
	{
		AnimationGraph::AnimationState* nextState = animationGraph->GetAnimationStateById(transition->toStateId);
		if (nextState)
		{
			if (transition->transitionDuration > 0.0f)
				StartTransition(nextState, transition->transitionDuration);
			else
			{
				activeAnimationState = nextState;
				timeElapsed = 0.0f;
				previousSprite = -1;
			}
		}
	}
}

void AnimationPlayer::StartTransition(AnimationGraph::AnimationState* toState, float duration)
{
	isBlending = true;
	blendState.fromState = activeAnimationState;
	blendState.toState = toState;
	blendState.blendTime = 0.0f;
	blendState.blendDuration = duration;
	blendState.fromSpriteIndex = previousSprite;
	blendState.toSpriteIndex = 0;
	blendState.fromTime = timeElapsed;
	blendState.toTime = 0.0f;
}

int AnimationPlayer::GetCurrentSpriteIndex(AnimationGraph::AnimationState* state, float time)
{
	if (!state)
		return -1;

	const auto& sprites = state->animation.GetSprites();
	if (sprites.empty())
		return -1;

	float animSpeed = state->animation.GetSpeed();
	if (animSpeed <= 0.0f)
		return 0;

	int index = static_cast<int>(time / (animSpeed / sprites.size()));
	return std::min(index, static_cast<int>(sprites.size()) - 1);
}

void AnimationPlayer::SetSpriteFromAnimation(AnimationGraph::AnimationState* state, int index)
{
	if (!state || !spriteRenderer)
		return;

	const auto& sprites = state->animation.GetSprites();
	if (index >= 0 && index < sprites.size())
		spriteRenderer->SetSprite(new Sprite(sprites[index]->GetRelativePath()));
}

void AnimationPlayer::Destroy()
{
	animationGraph.reset();
}

void AnimationPlayer::Pause()
{
	paused = true;
}

void AnimationPlayer::Unpause()
{
	paused = false;
}

bool AnimationPlayer::IsPaused()
{
	return paused;
}

void AnimationPlayer::Stop()
{
	isPlaying = false;
	timeElapsed = 0.0f;
	previousSprite = -1;
	isBlending = false;
}

void AnimationPlayer::Play()
{
	isPlaying = true;
}

void AnimationPlayer::SetAnimationGraph(std::shared_ptr<AnimationGraph> graph)
{
	animationGraph = graph;
	if (animationGraph)
	{
		activeAnimationState = animationGraph->GetStartAnimationState();
		timeElapsed = 0.0f;
		previousSprite = -1;
		isBlending = false;
	}
}

void AnimationPlayer::SetAnimationGraph(const std::string& path)
{
	animationGraph = AnimationGraphCache::GetOrLoad(path);
	if (animationGraph)
	{
		activeAnimationState = animationGraph->GetStartAnimationState();
		timeElapsed = 0.0f;
		previousSprite = -1;
		isBlending = false;
	}
}

std::shared_ptr<AnimationGraph> AnimationPlayer::GetAnimationGraph()
{
	return animationGraph;
}

void AnimationPlayer::SetActiveAnimation(const std::string& name)
{
	if (!animationGraph)
		return;

	AnimationGraph::AnimationState* newState = animationGraph->GetAnimationStateByName(name);
	if (newState && newState != activeAnimationState)
	{
		activeAnimationState = newState;
		timeElapsed = 0.0f;
		previousSprite = -1;
		isBlending = false;
	}
}

void AnimationPlayer::SetActiveAnimation(Animation* animation)
{
	if (!animationGraph || !animation || animation->GetId() < 0)
		return;

	AnimationGraph::AnimationState* newState = animationGraph->GetAnimationStateById(animation->GetId());
	if (newState && newState != activeAnimationState)
	{
		activeAnimationState = newState;
		timeElapsed = 0.0f;
		previousSprite = -1;
		isBlending = false;
	}
}

Animation* AnimationPlayer::GetActiveAnimation()
{
	return activeAnimationState ? &activeAnimationState->animation : nullptr;
}

void AnimationPlayer::CrossFade(const std::string& name, float duration)
{
	if (!animationGraph)
		return;

	AnimationGraph::AnimationState* newState = animationGraph->GetAnimationStateByName(name);
	if (newState && newState != activeAnimationState)
		StartTransition(newState, duration);
}

void AnimationPlayer::CrossFade(Animation* animation, float duration)
{
	if (!animationGraph || !animation)
		return;

	AnimationGraph::AnimationState* newState = animationGraph->GetAnimationStateById(animation->GetId());
	if (newState && newState != activeAnimationState)
		StartTransition(newState, duration);
}

void AnimationPlayer::SetBool(const std::string& name, bool value)
{
	if (animationGraph)
		animationGraph->SetBool(name, value);
}

void AnimationPlayer::SetInt(const std::string& name, int value)
{
	if (animationGraph)
		animationGraph->SetInt(name, value);
}

void AnimationPlayer::SetFloat(const std::string& name, float value)
{
	if (animationGraph)
		animationGraph->SetFloat(name, value);
}

void AnimationPlayer::SetTrigger(const std::string& name)
{
	if (animationGraph)
		animationGraph->SetTrigger(name);
}

bool AnimationPlayer::GetBool(const std::string& name, bool defaultValue)
{
	return animationGraph ? animationGraph->GetBool(name, defaultValue) : defaultValue;
}

int AnimationPlayer::GetInt(const std::string& name, int defaultValue)
{
	return animationGraph ? animationGraph->GetInt(name, defaultValue) : defaultValue;
}

float AnimationPlayer::GetFloat(const std::string& name, float defaultValue)
{
	return animationGraph ? animationGraph->GetFloat(name, defaultValue) : defaultValue;
}

float AnimationPlayer::GetNormalizedTime() const
{
	if (!activeAnimationState)
		return 0.0f;

	float duration = activeAnimationState->animation.GetSpeed();
	if (duration <= 0.0f)
		return 0.0f;

	return timeElapsed / duration;
}

float AnimationPlayer::GetCurrentTime() const
{
	return timeElapsed;
}

float AnimationPlayer::GetAnimationDuration() const
{
	return activeAnimationState ? activeAnimationState->animation.GetSpeed() : 0.0f;
}