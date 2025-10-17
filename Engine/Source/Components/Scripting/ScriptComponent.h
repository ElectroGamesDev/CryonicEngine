#pragma once

#include "Components/Component.h"
#include <filesystem>

class ScriptComponent : public Component
{
public:
	ScriptComponent(GameObject* obj, int id) : Component(obj, id)
	{
		name = "ScriptComponent";
	}
	ScriptComponent* Clone() override
	{
		return new ScriptComponent(gameObject, -1);
	}
	void Start() override;
	void Update() override;
	void Destroy() override;
	void SetHeaderPath(std::filesystem::path path);
	void SetCppPath(std::filesystem::path path);
	std::filesystem::path GetHeaderPath();
	std::filesystem::path GetCppPath();
	void SetName(std::string name);
	std::string GetName();

private:
	std::filesystem::path _headerPath;
	std::filesystem::path _CppPath;
	std::string _name;
}; 