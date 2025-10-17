#pragma once
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <memory>
#include "Raylib/RaylibWrapper.h"
#include "Utilities/ConsoleLogger.h"
#include "ThirdParty/Misc/json.hpp"
#include "Animation.h"
#include <variant>
#ifndef EDITOR
#include "Game.h"
#endif

class AnimationGraph
{
public:
	enum ConditionType
	{
		Less,
		LessOrEqual,
		Equal,
		NotEqual,
		GreaterOrEqual,
		Greater
	};

	enum ParameterType
	{
		Bool,
		Int,
		Float,
		Trigger
	};

	struct Parameter
	{
		std::string name;
		ParameterType type;
		std::variant<bool, int, float> value;
		bool triggered = false; // For trigger parameters
	};

	struct Condition
	{
		std::string parameter;
		ConditionType conditionType;
		std::variant<bool, int, float> value;
	};

	struct Transition
	{
		int fromStateId;
		int toStateId;
		std::vector<Condition> conditions;
		float transitionDuration = 0.0f; // Duration for blending
		float exitTime = 0.0f; // Normalized time to allow transition (0-1)
		bool hasExitTime = false;
		bool canTransitionToSelf = false;
	};

	struct AnimationState
	{
		Animation animation;
		std::vector<Transition> transitions;
		int id;
	};

	AnimationGraph(std::string path);
	~AnimationGraph();

	// Animation control
	Animation* GetStartAnimation();
	AnimationState* GetStartAnimationState();
	std::vector<Animation*> GetAnimations();
	std::vector<AnimationState>* GetAnimationStates();
	AnimationState* GetAnimationStateById(int id);
	AnimationState* GetAnimationStateByName(const std::string& name);

	// Parameter management
	void SetBool(const std::string& name, bool value);
	void SetInt(const std::string& name, int value);
	void SetFloat(const std::string& name, float value);
	void SetTrigger(const std::string& name);
	void ResetTrigger(const std::string& name);

	bool GetBool(const std::string& name, bool defaultValue = false);
	int GetInt(const std::string& name, int defaultValue = 0);
	float GetFloat(const std::string& name, float defaultValue = 0.0f);
	bool GetTrigger(const std::string& name);

	// Transition checking
	Transition* CheckTransitions(AnimationState* currentState, float normalizedTime);
	bool EvaluateConditions(const std::vector<Condition>& conditions);

	// Utilities
	void ReloadFromFile();
	const std::string& GetPath() const { return filePath; }

private:
	std::vector<AnimationState> animationStates;
	AnimationState* startAnimationState = nullptr;
	std::unordered_map<std::string, Parameter> parameters;
	std::vector<Transition> anyStateTransitions; // Global transitions from any state
	std::string filePath;

	bool EvaluateCondition(const Condition& condition);
	void LoadFromJson(const nlohmann::json& jsonData);
};

// Shared animation graph cache to avoid loading duplicates
class AnimationGraphCache
{
public:
	static std::shared_ptr<AnimationGraph> GetOrLoad(const std::string& path);
	static void Clear();
	static void Remove(const std::string& path);

private:
	static std::unordered_map<std::string, std::weak_ptr<AnimationGraph>> cache;
};