#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include "ThirdParty/Misc/json.hpp"
#include "Utilities/ConsoleLogger.h"
#include "Systems/Events/Event.h"
#if defined (EDITOR)
#include "Core/ProjectManager.h"
#include "Systems/FileWatcher/FileWatcher.h"
#else
#include "Game.h"
#endif

class Canvas
{
public:
    Canvas(std::string path)
    {
        for (char& c : path) // Reformatted the path for unix.
        {
            if (c == '\\')
                c = '/';
        }

#if defined(EDITOR)
		FileWatcher::AddFileMoveCallback(path, [this](const std::string& oldPath, const std::string& newPath) {
            OnFileMoved(oldPath, newPath);
		});

		FileWatcher::AddFileModifyCallback(path, [this, path]() {
			this->LoadData(path);
		});
#endif

        LoadData(path);
    }
    
#if defined(EDITOR)
    void OnFileMoved(const std::string& oldPath, const std::string& newPath)
    {
		FileWatcher::RemoveFileMoveCallback(oldPath);
		FileWatcher::RemoveFileModifyCallback(oldPath);

		FileWatcher::AddFileMoveCallback(newPath, [this](const std::string& oldPath, const std::string& newPath) {
            OnFileMoved(oldPath, newPath);
		});

        FileWatcher::AddFileModifyCallback(newPath, [this, newPath]() {
            this->LoadData(newPath);
		});

		this->LoadData(newPath);
    }
#endif

    void LoadData(std::string relativepath)
    {
		for (char& c : relativepath) // Reformatted the path for unix.
		{
			if (c == '\\')
				c = '/';
		}

		this->relativePath = relativepath;

#if defined(EDITOR)
		fullPath = ProjectManager::projectData.path.string() + "/Assets/" + relativepath;
#else
		if (exeParent.empty())
			fullPath = "Resources/Assets/" + relativepath;
		else
			fullPath = exeParent.string() + "/Resources/Assets/" + relativepath;
#endif

		auto it = canvases.find(relativePath);
		if (it != canvases.end())
			jsonData = &it->second;
		else
		{
			std::ifstream file(fullPath);
			if (!file.is_open())
			{
				ConsoleLogger::ErrorLog("Canvas failed to load. Path: " + fullPath);
				return;
			}

			nlohmann::json data;
			file >> data;
			canvases[relativePath] = data;
			jsonData = &canvases[relativePath];
		}
    }

    /**
     * @brief Returns the full path to the canvas file.
     *
     * @return [string] The full path to the canvas file.
     */
    const std::string GetPath() { return fullPath; };

    /**
    * @brief Returns the relative path to the canvas file.
    *
    * @return [string] The relative path to the canvas file.
    */
    const std::string GetRelativePath() { return relativePath; };

    // Hide in API
    nlohmann::json* GetData()
    {
        if (jsonData && jsonData->is_null())
        {
            std::ifstream file(fullPath);
            if (file.is_open())
            {
                *jsonData = nlohmann::json();
                file >> *jsonData;
            }
            else
                *jsonData = nlohmann::json(nullptr);
        }

        return jsonData;
    };

	// Hide in API
	void SetData(nlohmann::json data)
	{
		if (jsonData)
			*jsonData = data;
		else
			jsonData = new nlohmann::json(data);

#if defined (EDITOR)
        if (!fullPath.empty())
        {
            std::ofstream file(fullPath);
            if (file.is_open())
            {
                file << std::setw(4) << data << std::endl;
                file.close();
            }
        }
#endif

        onDataChangeEvent.Invoke();
	};

    static std::unordered_map<std::string, nlohmann::json> canvases;
	static Event onDataChangeEvent;

private:
    std::string fullPath;
    std::string relativePath;
    nlohmann::json* jsonData = nullptr;
};