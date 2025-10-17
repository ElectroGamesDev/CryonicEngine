#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include "ThirdParty/Misc/json.hpp"
#include "Raylib/RaylibWrapper.h"
#if defined (EDITOR)
#include "Core/ProjectManager.h"
#else
#include "Game.h"
#endif

class Sprite
{
public:
	Sprite(std::string path)
	{
//		std::ifstream file;
//#ifndef EDITOR
//		file.open(std::filesystem::path(exeParent) / "Resources" / "Assets" / std::string(path + ".data"));
//#endif
//		if (!file.is_open())
//		{
//			ConsoleLogger::ErrorLog("Sprite failed to load. Path: " + path);
//			return;
//		}

		for (char& c : path) // Reformatted the path for unix.
		{
			if (c == '\\')
				c = '/';
		}

		relativePath = path;

		if (path != "Square" && path != "Circle" && path != "None")
		{
#if defined(EDITOR)
			path = ProjectManager::projectData.path.string() + "/Assets/" + path;
#else
			if (exeParent.empty())
				path = "Resources/Assets/" + path;
			else
				path = exeParent.string() + "/Resources/Assets/" + path;
#endif

			if (auto it = textures.find(relativePath); it != textures.end())
				texture = &it->second;
			else
			{
				textures[relativePath].first = new RaylibWrapper::Texture2D(RaylibWrapper::LoadTexture(path.c_str()));
				texture = &textures[relativePath];
			}
		}

		this->path = path;
	}

	/**
	 * @brief Returns the full path to the sprite file.
	 *
	 * @return [string] The full path to the sprite file.
	 */
	const std::string GetPath() { return path; };

	/**
	* @brief Returns the relative path to the sprite file.
	*
	* @return [string] The relative path to the sprite file.
	*/
	const std::string GetRelativePath() { return relativePath; };

	// Hide in API
	RaylibWrapper::Texture2D* GetTexture()
	{
#if defined(EDITOR)
 		// This wont work since some of the code checks GetTexture() to see if a texture is loaded every frame.
		if (texture && !texture->first) // Texture will be nullptr if the Sprite gets reloaded.
		{
			auto it = textures.find(relativePath);
			if (it != textures.end())
				texture->first = it->second.first;
			else
			{
				if (std::filesystem::exists(path) && (std::filesystem::path(path).extension().string() == ".png") || std::filesystem::path(path).extension().string() == ".jpg" || std::filesystem::path(path).extension().string() == ".jpeg")
				{
					textures[path].first = new RaylibWrapper::Texture2D(RaylibWrapper::LoadTexture(path.c_str()));
					texture->first = textures[path].first;
				}
				else
				{
					path = "None";
					relativePath = path;
				}
			}
		}
#endif
		return texture ? texture->first : nullptr;
	};

	// Hide in API
	static std::unordered_map<std::filesystem::path, std::pair<RaylibWrapper::Texture2D*, int>> textures;

private:
	std::string path;
	std::string relativePath;
	std::pair<RaylibWrapper::Texture2D*, int>* texture = nullptr;
};