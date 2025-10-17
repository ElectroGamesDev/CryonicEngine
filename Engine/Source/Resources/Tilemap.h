#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include "ThirdParty/Misc/json.hpp"
#include "Core/CryonicCore.h"
#include "Utilities/ConsoleLogger.h"
#include "Raylib/RaylibWrapper.h"
#if defined (EDITOR)
#include "Core/ProjectManager.h"
#else
#include "Game.h"
#endif

// Hide in API
struct Tile
{
	Vector2 position;
	RaylibWrapper::Rectangle texturePosition;
	RaylibWrapper::Texture2D* tileset = nullptr;
	Vector2 size;
};

class Tilemap
{
public:
	Tilemap(std::string path)
	{
		for (char& c : path) // Reformats the path for unix.
		{
			if (c == '\\')
				c = '/';
		}

		relativePath = path;
		std::string assetsPath;

#if defined(EDITOR)
		assetsPath = ProjectManager::projectData.path.string() + "/Assets";
		path = assetsPath + "/" + path;
#else
		if (exeParent.empty())
		{
			assetsPath = "Resources/Assets";
			path = assetsPath + "/" + path;
		}
		else
		{
			assetsPath = exeParent.string() + "/Resources/Assets";
			path = assetsPath + "/" + path;
		}
#endif

		if (!std::filesystem::exists(path)) // todo: Handle if it doesn't exist
		{
			ConsoleLogger::ErrorLog("Failed to find tilemap at the path " + path);
			this->path = "None";
			return;
		}

		if (std::filesystem::path(path).extension() == ".ldtk")
		{
			// Todo: Properly handle if something is wrong with the data
			// I'm not iterating the data["defs"]["tilesets"] to load tilesets since only some timesets may be used
			// Currently each "level" in ldtk is combined into one tilemap
			// Todo: Add a vector variable in TilemapRenderer to select which level to use

			std::ifstream file(path);
			if (!file.is_open())
			{
				ConsoleLogger::ErrorLog("Failed to open the tilemap at the path " + path);
				this->path = "None";
				return;
			}

			this->path = path;

			std::ifstream dataFile(path + ".data");
			nlohmann::json dataFileJson = nullptr;

			if (dataFile.is_open())
				dataFileJson = nlohmann::json::parse(dataFile);

			nlohmann::json data = nlohmann::json::parse(file);
			for (int i = 0; i < data["levels"].size(); i++) // Iterating each level
			{
				for (const auto& layer : data["levels"][i]["layerInstances"]) // Iterating each layer
				{

					if (layer["__type"] != "Tiles") // Only tiles are supported
						continue;

					int tileSize = layer["__gridSize"];

					// Todo: Remove the duplicate code below
					// Checking if the path is in the .data file
					if (!dataFileJson.is_null() && dataFileJson["public"].contains(layer["__type"]) && dataFileJson["public"][layer["__type"]][2] != "nullptr")
					{
						if (!std::filesystem::exists(assetsPath + "/" + dataFileJson["public"][layer["__type"]][2].get<std::string>()))
						{
							ConsoleLogger::ErrorLog("Tileset could not be found at " + assetsPath + "/" + dataFileJson["public"][layer["__type"]][2].get<std::string>() + ". You can set a new path in the tilemap properties.");
							//this->path = assetsPath + "/" + dataFileJson["public"][layer["__type"]][2].get<std::string>();
							continue;
						}

						if (textures.find(layer["__tilesetRelPath"]) == textures.end())
							textures[layer["__tilesetRelPath"]].first = new RaylibWrapper::Texture2D(RaylibWrapper::LoadTexture((assetsPath + "/" + dataFileJson["public"][layer["__type"]][2].get<std::string>()).c_str()));
					}
					else
					{
						if (!std::filesystem::exists(assetsPath + "/" + layer["__tilesetRelPath"].get<std::string>())) // Checking if tileset exists using the path in the .ldtk file
						{
							// Todo: Properly handle this
							ConsoleLogger::ErrorLog("Tileset could not be found at " + assetsPath + "/" + layer["__tilesetRelPath"].get<std::string>() + ". You can set a new path in the tilemap properties.");
							//this->path = assetsPath + "/" + layer["__type"].get<std::string>();
							continue;
						}

						if (textures.find(layer["__tilesetRelPath"]) == textures.end())
							textures[layer["__tilesetRelPath"]].first = new RaylibWrapper::Texture2D(RaylibWrapper::LoadTexture((assetsPath + "/" + layer["__type"].get<std::string>()).c_str()));
					}

					for (const auto& gridTile : layer["gridTiles"])
					{
						// Continues if the tile isnt set. Edit: 0 might be a valid id
						//if (gridTile["t"] == 0)
						//	continue;

						Tile tile;
						tile.position = { gridTile["px"][0].get<float>() / (tileSize*0.63f), gridTile["px"][1].get<float>() / (tileSize*0.63f) }; // LDTK stores positions in pixels // Todo: Stop using "magic numbers"
						tile.texturePosition = {
							gridTile["src"][0].get<float>(),
							gridTile["src"][1].get<float>(),
							(float)tileSize,
							(float)tileSize * -1
						};
						tile.tileset = textures[layer["__tilesetRelPath"]].first;
						tile.size = { (float)tileSize, (float)tileSize };
						tiles.push_back(tile);
					}
				}
			}
		}
	}

	/**
	 * @brief Returns the full path to the tilemap file.
	 *
	 * @return [string] The full path to the tilemap file.
	 */
	const std::string GetPath() { return path; };

	/**
	* @brief Returns the relative path to the tilemap file.
	*
	* @return [string] The relative path to the tilemap file.
	*/
	const std::string GetRelativePath() { return relativePath; };

	// Hide in API
	const std::vector<Tile>* GetTiles() { return &tiles; }

	// Hide in API
	static std::unordered_map<std::filesystem::path, std::pair<RaylibWrapper::Texture2D*, int>> textures;

private:
	std::string path;
	std::string relativePath;
	std::vector<Tile> tiles;
};