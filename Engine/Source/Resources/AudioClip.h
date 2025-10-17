#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include "Raylib/RaylibWrapper.h"
#include "Utilities/ConsoleLogger.h"
#include "ThirdParty/Misc/json.hpp"
#ifndef EDITOR
#include "Game.h"
#endif
// This is used by AudioPlayer. It's in here since AudioPlayer.h couldn't include it without it using the wrong include path for game builds.
namespace Raylib
{
#include "ThirdParty/Raylib/include/raylib.h"
}

class AudioClip
{
public:
	AudioClip(std::string path)
	{
		for (char& c : path) // Reformat the path for unix
		{
			if (c == '\\')
				c = '/';
		}

		std::ifstream file;
#ifndef EDITOR
		if (exeParent.empty())
			file.open("Resources/Assets/" + path + ".data");
		else
			file.open(std::filesystem::path(exeParent) / "Resources" / "Assets" / std::string(path + ".data"));
#endif
		if (!file.is_open()) // Todo: Instead if the audio file exists, but the data file doesnt, just give it default values
		{
			ConsoleLogger::ErrorLog("Audio Clip failed to load. Path: " + path);
			return;
		}

		nlohmann::json jsonData;
		file >> jsonData;

		file.close();

		loadInMemory = jsonData["public"]["loadInMemory"].get<bool>();
#ifndef EDITOR
		if (loadInMemory)
		{
			if (auto it = sounds.find(path); it != sounds.end())
				sound = &it->second;
			else
			{
				if (exeParent.empty())
					sounds[path] = Raylib::LoadSound(("Resources/Assets/" + path).c_str());
				else
					sounds[path] = Raylib::LoadSound((exeParent.string() + "/Resources/Assets/" + path).c_str());
				sound = &sounds[path];
			}
		}
#endif

		this->path = path;
	}

	~AudioClip()
	{
		// Sounds are unloaded in Game.cpp
		//if (sound)
		//	Raylib::UnloadSound(*sound);
	}

	/**
	 * @brief Returns whether an audio clip should be loaded into memory or if it should be streamed.
	 *
	 * @return [bool] True if the audio clip should be loaded into memory, false if it should is streamed.
	 */
	bool LoadedInMemory() const; // This is useless to scripting API, but is useful for AudioPlayer.

	/**
	 * @brief Returns the relative path to the audio clip file.
	 *
	 * @return [std::string] The relative path to the audio clip file.
	 */
	std::string GetPath() const;
	
	// Hide in API
	Raylib::Sound* GetRaylibSound();

	// Hide in API
	static std::unordered_map<std::filesystem::path, Raylib::Sound> sounds;

private:
	std::string path = "";
	bool loadInMemory = false; // True = load the audio clip into the disk, False = stream the audio clip
	Raylib::Sound* sound; // Used for sounds in memory
};