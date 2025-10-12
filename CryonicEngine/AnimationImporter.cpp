#include "AnimationImporter.h"
#include "tiny_gltf.h"
#include "ConsoleLogger.h"
#include <fstream>
#include <thread>
#include <random>
#include "ProjectManager.h"

std::unordered_map<std::string, AnimationImporter::ModelAnimationCache> AnimationImporter::cache;
std::filesystem::path AnimationImporter::cacheFilePath;
std::mutex AnimationImporter::cacheMutex;

void AnimationImporter::Init()
{
#if defined(EDITOR)
	cacheFilePath = ProjectManager::projectData.path / "Cache" / "AnimationCache.json";
#else
	cacheFilePath = "Cache/AnimationCache.json";
#endif
}

std::future<std::vector<AnimationImporter::AnimationInfo>> AnimationImporter::LoadAnimationsAsync(const std::filesystem::path& modelPath)
{
	return std::async(std::launch::async, [modelPath]() {
		return GetAnimationInfo(modelPath);
		});
}

std::vector<AnimationImporter::AnimationInfo> AnimationImporter::GetAnimationInfo(const std::filesystem::path& modelPath)
{
	std::lock_guard<std::mutex> lock(cacheMutex);

	std::string pathKey = modelPath.string();

	// Check if cached and valid
	if (cache.find(pathKey) != cache.end() && IsCacheValid(modelPath))
	{
		return cache[pathKey].animations;
	}

	// Extract animations from model
	std::vector<AnimationInfo> animations = ExtractAnimations(modelPath);

	// Update cache
	ModelAnimationCache newCache;
	newCache.modelPath = modelPath;
	newCache.animations = animations;
	newCache.lastModified = std::filesystem::last_write_time(modelPath);
	cache[pathKey] = newCache;

	// Save cache to disk
	SaveCache();

	return animations;
}

std::vector<AnimationImporter::AnimationInfo> AnimationImporter::ExtractAnimations(const std::filesystem::path& path)
{
	std::vector<AnimationInfo> result;

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err, warn;
	bool ret = false;

	// Load model based on extension
	if (path.extension() == ".gltf")
		ret = loader.LoadASCIIFromFile(&model, &err, &warn, path.string());
	else if (path.extension() == ".glb")
		ret = loader.LoadBinaryFromFile(&model, &err, &warn, path.string());

	if (!warn.empty())
		ConsoleLogger::WarningLog("GLTF Warning: " + warn);
	if (!err.empty())
		ConsoleLogger::ErrorLog("GLTF Error: " + err);
	if (!ret)
	{
		ConsoleLogger::ErrorLog("Failed to parse GLTF model: " + path.string());
		return result;
	}

	// Extract animation info (fast - only reading metadata)
	result.reserve(model.animations.size());
	for (size_t i = 0; i < model.animations.size(); ++i)
	{
		AnimationInfo info;
		info.name = model.animations[i].name.empty() ? "Animation_" + std::to_string(i) : model.animations[i].name;
		info.index = static_cast<int>(i);

		// Calculate duration from samplers
		float maxTime = 0.0f;
		for (const auto& channel : model.animations[i].channels)
		{
			if (channel.sampler >= 0 && channel.sampler < model.animations[i].samplers.size())
			{
				const auto& sampler = model.animations[i].samplers[channel.sampler];
				if (sampler.input >= 0 && sampler.input < model.accessors.size())
				{
					const auto& accessor = model.accessors[sampler.input];
					if (!accessor.maxValues.empty())
						maxTime = std::max(maxTime, static_cast<float>(accessor.maxValues[0]));
				}
			}
		}
		info.duration = maxTime;

		result.push_back(info);
	}

	return result;
}

bool AnimationImporter::IsCacheValid(const std::filesystem::path& modelPath)
{
	std::string pathKey = modelPath.string();
	if (cache.find(pathKey) == cache.end())
		return false;

	if (!std::filesystem::exists(modelPath))
		return false;

	auto cachedTime = cache[pathKey].lastModified;
	auto currentTime = std::filesystem::last_write_time(modelPath);

	return cachedTime == currentTime;
}

void AnimationImporter::ImportAnimationsToGraph(nlohmann::json& graphData, const std::filesystem::path& modelPath, const ImVec2& dropPosition)
{
	std::vector<AnimationInfo> animations = GetAnimationInfo(modelPath);

	if (animations.empty())
	{
		ConsoleLogger::WarningLog("No animations found in model: " + modelPath.string());
		return;
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distribution(999999, 99999999);

	int offsetX = 0;
	int offsetY = 0;

	for (const auto& animInfo : animations)
	{
		// Generate unique ID
		int id = 0;
		while (id == 0 || id == -5 || id == -6)
		{
			id = distribution(gen);
			for (auto& node : graphData["nodes"])
			{
				if (id == node["id"])
				{
					id = 0;
					break;
				}
			}
		}

		nlohmann::json node = {
			{"id", id},
			{"name", animInfo.name},
			{"index", animInfo.index},
			{"x", dropPosition.x + offsetX},
			{"y", dropPosition.y + offsetY},
			{"loop", true},
			{"speed", animInfo.duration > 0 ? animInfo.duration : 1.0f},
			{"model_path", modelPath.string()},
			{"animation_index", animInfo.index}
		};

		graphData["nodes"].push_back(node);

		// Offset for next node
		offsetX += 30;
		offsetY += 30;
	}

	ConsoleLogger::InfoLog("Imported " + std::to_string(animations.size()) + " animations from " + modelPath.filename().string());
}

void AnimationImporter::SaveCache()
{
	try
	{
		std::filesystem::create_directories(cacheFilePath.parent_path());

		nlohmann::json cacheJson = nlohmann::json::array();

		for (const auto& [path, cacheData] : cache)
		{
			nlohmann::json entry;
			entry["path"] = path;
			entry["last_modified"] = std::chrono::duration_cast<std::chrono::milliseconds>(cacheData.lastModified.time_since_epoch()).count();

			entry["animations"] = nlohmann::json::array();

			for (const auto& anim : cacheData.animations)
			{
				entry["animations"].push_back({
					{"name", anim.name},
					{"index", anim.index},
					{"duration", anim.duration}
					});
			}

			cacheJson.push_back(entry);
		}

		std::ofstream file(cacheFilePath);
		if (file.is_open())
		{
			file << std::setw(2) << cacheJson;
			file.close();
		}
	}
	catch (const std::exception& e)
	{
		ConsoleLogger::ErrorLog("Failed to save animation cache: " + std::string(e.what()));
	}
}

void AnimationImporter::LoadCache()
{
	std::lock_guard<std::mutex> lock(cacheMutex);

	if (!std::filesystem::exists(cacheFilePath))
		return;

	try
	{
		std::ifstream file(cacheFilePath);
		if (!file.is_open())
			return;

		nlohmann::json cacheJson;
		file >> cacheJson;
		file.close();

		cache.clear();

		for (const auto& entry : cacheJson)
		{
			ModelAnimationCache cacheData;
			cacheData.modelPath = entry["path"].get<std::string>();

			auto timeT = entry["last_modified"].get<time_t>();
			cacheData.lastModified = std::filesystem::file_time_type(std::chrono::milliseconds(timeT));

			for (const auto& animJson : entry["animations"])
			{
				AnimationInfo info;
				info.name = animJson["name"].get<std::string>();
				info.index = animJson["index"].get<int>();
				info.duration = animJson["duration"].get<float>();
				cacheData.animations.push_back(info);
			}

			cache[entry["path"].get<std::string>()] = cacheData;
		}
	}
	catch (const std::exception& e)
	{
		ConsoleLogger::ErrorLog("Failed to load animation cache: " + std::string(e.what()));
		cache.clear();
	}
}

void AnimationImporter::ClearCache()
{
	std::lock_guard<std::mutex> lock(cacheMutex);
	cache.clear();

	if (std::filesystem::exists(cacheFilePath))
		std::filesystem::remove(cacheFilePath);
}

void AnimationImporter::RemoveFromCache(const std::filesystem::path& modelPath)
{
	std::lock_guard<std::mutex> lock(cacheMutex);
	cache.erase(modelPath.string());
	SaveCache();
}