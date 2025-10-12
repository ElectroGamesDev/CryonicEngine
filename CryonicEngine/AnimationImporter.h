#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <unordered_map>
#include <future>
#include "json.hpp"
#include <imgui.h>

class AnimationImporter
{
public:
	struct AnimationInfo
	{
		std::string name;
		int index;
		float duration;
	};

	struct ModelAnimationCache
	{
		std::filesystem::path modelPath;
		std::vector<AnimationInfo> animations;
		std::filesystem::file_time_type lastModified;
	};

	// Fast async animation loading
	static std::future<std::vector<AnimationInfo>> LoadAnimationsAsync(const std::filesystem::path& modelPath);

	// Cached animation info retrieval
	static std::vector<AnimationInfo> GetAnimationInfo(const std::filesystem::path& modelPath);

	// Import animations to graph
	static void ImportAnimationsToGraph(nlohmann::json& graphData, const std::filesystem::path& modelPath,
		const ImVec2& dropPosition = ImVec2(350, 350));

	// Cache management
	static void Init();
	static void SaveCache();
	static void LoadCache();
	static void ClearCache();
	static void RemoveFromCache(const std::filesystem::path& modelPath);

private:
	static std::unordered_map<std::string, ModelAnimationCache> cache;
	static std::filesystem::path cacheFilePath;
	static std::mutex cacheMutex;

	static std::vector<AnimationInfo> ExtractAnimations(const std::filesystem::path& path);
	static bool IsCacheValid(const std::filesystem::path& modelPath);
};