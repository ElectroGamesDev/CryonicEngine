#pragma once

#include <filesystem>

namespace ScriptHeaderGenerator
{
	void Init();

	// The path must be the .cpp file path
	void GenerateHeader(std::filesystem::path path);
}