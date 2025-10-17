#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include "ThirdParty/Misc/json.hpp"

class Font
{
public:
	Font(std::string path)
	{
//		std::ifstream file;
//#ifndef EDITOR
//		file.open(std::filesystem::path(exeParent) / "Resources" / "Assets" / std::string(path + ".data"));
//#endif
//		if (!file.is_open())
//		{
//			ConsoleLogger::ErrorLog("Font failed to load. Path: " + path);
//			return;
//		}

		this->relativePath = path;
		this->path = path;
	}

	/**
	 * @brief Returns the relative path to the font file.
	 *
	 * @return [string] The relative path to the font file.
	 */
	std::string GetPath() const { return path; };
	std::string GetRelativePath() const { return relativePath; }

private:
	std::string path = "";
	std::string relativePath = "";
};