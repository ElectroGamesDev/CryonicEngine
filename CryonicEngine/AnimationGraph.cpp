#include "AnimationGraph.h"

std::unordered_map<std::string, std::weak_ptr<AnimationGraph>> AnimationGraphCache::cache;

AnimationGraph::AnimationGraph(std::string path)
{
	filePath = path;
	for (char& c : filePath)
	{
		if (c == '\\')
			c = '/';
	}

	std::ifstream file;
#ifndef EDITOR
	if (exeParent.empty())
		file.open("Resources/Assets/" + filePath);
	else
		file.open(std::filesystem::path(exeParent) / "Resources" / "Assets" / filePath);
#else
	file.open(filePath);
#endif

	if (!file.is_open())
	{
		ConsoleLogger::ErrorLog("Animation Graph failed to load. Path: " + filePath);
		return;
	}

	nlohmann::json jsonData;
	file >> jsonData;
	file.close();

	LoadFromJson(jsonData);
}

AnimationGraph::~AnimationGraph()
{
	for (auto& state : animationStates)
	{
		for (auto* sprite : state.animation.sprites)
			delete sprite;
		state.animation.sprites.clear();
	}
}

void AnimationGraph::LoadFromJson(const nlohmann::json& jsonData)
{
	animationStates.clear();
	anyStateTransitions.clear();
	parameters.clear();
	startAnimationState = nullptr;

	// Load parameters
	if (jsonData.contains("parameters"))
	{
		for (const auto& param : jsonData["parameters"])
		{
			Parameter p;
			p.name = param["name"].get<std::string>();
			std::string typeStr = param["type"].get<std::string>();

			if (typeStr == "bool")
			{
				p.type = ParameterType::Bool;
				p.value = param.value("value", false);
			}
			else if (typeStr == "int")
			{
				p.type = ParameterType::Int;
				p.value = param.value("value", 0);
			}
			else if (typeStr == "float")
			{
				p.type = ParameterType::Float;
				p.value = param.value("value", 0.0f);
			}
			else if (typeStr == "trigger")
			{
				p.type = ParameterType::Trigger;
				p.value = false;
				p.triggered = false;
			}

			parameters[p.name] = p;
		}
	}

	// Load animation states
	for (const auto& node : jsonData["nodes"])
	{
		int id = node["id"].get<int>();
		if (id == -5 || id == -6) // Skip Start and Any State nodes
			continue;

		AnimationState state;
		state.id = id;
		state.animation.id = id;
		state.animation.name = node["name"].get<std::string>();
		state.animation.loop = node["loop"].get<bool>();
		state.animation.speed = node["speed"].get<float>();

		if (node.contains("sprites") && !node["sprites"].is_null() && !node["sprites"].empty())
		{
			for (const auto& sprite : node["sprites"])
				state.animation.sprites.push_back(new Sprite(sprite.get<std::string>()));
		}

		// Load 3D animation data if present
		if (node.contains("animation_index"))
			state.animation.animationIndex = node["animation_index"].get<int>();
		if (node.contains("model_path"))
			state.animation.modelPath = node["model_path"].get<std::string>();

		animationStates.push_back(state);
	}

	// Load transitions
	if (jsonData.contains("transitions"))
	{
		for (const auto& transData : jsonData["transitions"])
		{
			Transition trans;
			trans.fromStateId = transData["from"].get<int>();
			trans.toStateId = transData["to"].get<int>();
			trans.transitionDuration = transData.value("duration", 0.0f);
			trans.exitTime = transData.value("exit_time", 0.0f);
			trans.hasExitTime = transData.value("has_exit_time", false);
			trans.canTransitionToSelf = transData.value("can_transition_to_self", false);

			// Load conditions
			if (transData.contains("conditions"))
			{
				for (const auto& condData : transData["conditions"])
				{
					Condition cond;
					cond.parameter = condData["parameter"].get<std::string>();

					std::string condTypeStr = condData["type"].get<std::string>();
					if (condTypeStr == "less") cond.conditionType = ConditionType::Less;
					else if (condTypeStr == "less_equal") cond.conditionType = ConditionType::LessOrEqual;
					else if (condTypeStr == "equal") cond.conditionType = ConditionType::Equal;
					else if (condTypeStr == "not_equal") cond.conditionType = ConditionType::NotEqual;
					else if (condTypeStr == "greater_equal") cond.conditionType = ConditionType::GreaterOrEqual;
					else if (condTypeStr == "greater") cond.conditionType = ConditionType::Greater;

					// Parse value based on parameter type
					if (parameters.find(cond.parameter) != parameters.end())
					{
						auto& param = parameters[cond.parameter];
						if (param.type == ParameterType::Bool)
							cond.value = condData["value"].get<bool>();
						else if (param.type == ParameterType::Int)
							cond.value = condData["value"].get<int>();
						else if (param.type == ParameterType::Float)
							cond.value = condData["value"].get<float>();
						else if (param.type == ParameterType::Trigger)
							cond.value = true;
					}

					trans.conditions.push_back(cond);
				}
			}

			// Determine if it's an any-state transition or regular transition
			if (trans.fromStateId == -6) // Any State
			{
				anyStateTransitions.push_back(trans);
			}
			else
			{
				// Add to the specific state
				for (auto& state : animationStates)
				{
					if (state.id == trans.fromStateId)
					{
						state.transitions.push_back(trans);
						break;
					}
				}
			}
		}
	}

	// Find start animation from links (legacy support)
	if (jsonData.contains("links"))
	{
		int startAnimationId = 0;
		for (const auto& link : jsonData["links"])
		{
			if (link[0] == -5)
			{
				startAnimationId = std::abs(link[1].get<int>());
				break;
			}
			else if (link[1] == -5)
			{
				startAnimationId = std::abs(link[0].get<int>());
				break;
			}
		}

		for (auto& state : animationStates)
		{
			if (state.id == startAnimationId)
			{
				startAnimationState = &state;
				break;
			}
		}
	}

	// If no start state found, use first animation
	if (startAnimationState == nullptr && !animationStates.empty())
		startAnimationState = &animationStates[0];
}

Animation* AnimationGraph::GetStartAnimation()
{
	return startAnimationState ? &startAnimationState->animation : nullptr;
}

AnimationGraph::AnimationState* AnimationGraph::GetStartAnimationState()
{
	return startAnimationState;
}

std::vector<Animation*> AnimationGraph::GetAnimations()
{
	std::vector<Animation*> animations;
	for (auto& state : animationStates)
		animations.push_back(&state.animation);
	return animations;
}

std::vector<AnimationGraph::AnimationState>* AnimationGraph::GetAnimationStates()
{
	return &animationStates;
}

AnimationGraph::AnimationState* AnimationGraph::GetAnimationStateById(int id)
{
	for (auto& state : animationStates)
	{
		if (state.id == id)
			return &state;
	}
	return nullptr;
}

AnimationGraph::AnimationState* AnimationGraph::GetAnimationStateByName(const std::string& name)
{
	for (auto& state : animationStates)
	{
		if (state.animation.name == name)
			return &state;
	}
	return nullptr;
}

void AnimationGraph::SetBool(const std::string& name, bool value)
{
	auto it = parameters.find(name);
	if (it != parameters.end() && it->second.type == ParameterType::Bool)
		it->second.value = value;
}

void AnimationGraph::SetInt(const std::string& name, int value)
{
	auto it = parameters.find(name);
	if (it != parameters.end() && it->second.type == ParameterType::Int)
		it->second.value = value;
}

void AnimationGraph::SetFloat(const std::string& name, float value)
{
	auto it = parameters.find(name);
	if (it != parameters.end() && it->second.type == ParameterType::Float)
		it->second.value = value;
}

void AnimationGraph::SetTrigger(const std::string& name)
{
	auto it = parameters.find(name);
	if (it != parameters.end() && it->second.type == ParameterType::Trigger)
	{
		it->second.triggered = true;
		it->second.value = true;
	}
}

void AnimationGraph::ResetTrigger(const std::string& name)
{
	auto it = parameters.find(name);
	if (it != parameters.end() && it->second.type == ParameterType::Trigger)
	{
		it->second.triggered = false;
		it->second.value = false;
	}
}

bool AnimationGraph::GetBool(const std::string& name, bool defaultValue)
{
	auto it = parameters.find(name);
	if (it != parameters.end() && it->second.type == ParameterType::Bool)
		return std::get<bool>(it->second.value);
	return defaultValue;
}

int AnimationGraph::GetInt(const std::string& name, int defaultValue)
{
	auto it = parameters.find(name);
	if (it != parameters.end() && it->second.type == ParameterType::Int)
		return std::get<int>(it->second.value);
	return defaultValue;
}

float AnimationGraph::GetFloat(const std::string& name, float defaultValue)
{
	auto it = parameters.find(name);
	if (it != parameters.end() && it->second.type == ParameterType::Float)
		return std::get<float>(it->second.value);
	return defaultValue;
}

bool AnimationGraph::GetTrigger(const std::string& name)
{
	auto it = parameters.find(name);
	if (it != parameters.end() && it->second.type == ParameterType::Trigger)
		return it->second.triggered;
	return false;
}

bool AnimationGraph::EvaluateCondition(const Condition& cond)
{
	auto it = parameters.find(cond.parameter);
	if (it == parameters.end())
		return false;

	const Parameter& param = it->second;

	if (param.type == ParameterType::Trigger)
		return param.triggered;

	if (param.type == ParameterType::Bool)
	{
		bool paramVal = std::get<bool>(param.value);
		bool condVal = std::get<bool>(cond.value);

		switch (cond.conditionType)
		{
		case ConditionType::Equal: return paramVal == condVal;
		case ConditionType::NotEqual: return paramVal != condVal;
		default: return false;
		}
	}
	else if (param.type == ParameterType::Int)
	{
		int paramVal = std::get<int>(param.value);
		int condVal = std::get<int>(cond.value);

		switch (cond.conditionType)
		{
		case ConditionType::Less: return paramVal < condVal;
		case ConditionType::LessOrEqual: return paramVal <= condVal;
		case ConditionType::Equal: return paramVal == condVal;
		case ConditionType::NotEqual: return paramVal != condVal;
		case ConditionType::GreaterOrEqual: return paramVal >= condVal;
		case ConditionType::Greater: return paramVal > condVal;
		}
	}
	else if (param.type == ParameterType::Float)
	{
		float paramVal = std::get<float>(param.value);
		float condVal = std::get<float>(cond.value);
		const float epsilon = 0.0001f;

		switch (cond.conditionType)
		{
		case ConditionType::Less: return paramVal < condVal;
		case ConditionType::LessOrEqual: return paramVal <= condVal;
		case ConditionType::Equal: return std::abs(paramVal - condVal) < epsilon;
		case ConditionType::NotEqual: return std::abs(paramVal - condVal) >= epsilon;
		case ConditionType::GreaterOrEqual: return paramVal >= condVal;
		case ConditionType::Greater: return paramVal > condVal;
		}
	}

	return false;
}

bool AnimationGraph::EvaluateConditions(const std::vector<Condition>& conditions)
{
	if (conditions.empty())
		return true;

	for (const auto& cond : conditions)
	{
		if (!EvaluateCondition(cond))
			return false;
	}
	return true;
}

AnimationGraph::Transition* AnimationGraph::CheckTransitions(AnimationState* currentState, float normalizedTime)
{
	if (!currentState)
		return nullptr;

	// Check any-state transitions first (higher priority)
	for (auto& trans : anyStateTransitions)
	{
		if (!trans.canTransitionToSelf && trans.toStateId == currentState->id)
			continue;

		if (EvaluateConditions(trans.conditions))
		{
			if (!trans.hasExitTime || normalizedTime >= trans.exitTime)
			{
				// Reset triggers
				for (const auto& cond : trans.conditions)
					ResetTrigger(cond.parameter);
				return &trans;
			}
		}
	}

	// Check state-specific transitions
	for (auto& trans : currentState->transitions)
	{
		if (!trans.canTransitionToSelf && trans.toStateId == currentState->id)
			continue;

		if (EvaluateConditions(trans.conditions))
		{
			if (!trans.hasExitTime || normalizedTime >= trans.exitTime)
			{
				// Reset triggers
				for (const auto& cond : trans.conditions)
					ResetTrigger(cond.parameter);
				return &trans;
			}
		}
	}

	return nullptr;
}

void AnimationGraph::ReloadFromFile()
{
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		ConsoleLogger::ErrorLog("Failed to reload Animation Graph: " + filePath);
		return;
	}

	nlohmann::json jsonData;
	file >> jsonData;
	file.close();

	LoadFromJson(jsonData);
}

// AnimationGraphCache implementation
std::shared_ptr<AnimationGraph> AnimationGraphCache::GetOrLoad(const std::string& path)
{
	auto it = cache.find(path);
	if (it != cache.end())
	{
		if (auto shared = it->second.lock())
			return shared;
		else
			cache.erase(it);
	}

	auto newGraph = std::make_shared<AnimationGraph>(path);
	cache[path] = newGraph;
	return newGraph;
}

void AnimationGraphCache::Clear()
{
	cache.clear();
}

void AnimationGraphCache::Remove(const std::string& path)
{
	cache.erase(path);
}