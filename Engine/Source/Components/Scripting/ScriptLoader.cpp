#include "ScriptLoader.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

void SetupScriptComponent(GameObject* gameObject, int id, bool active, std::string scriptName)
{
    if (true == false) {}
// SetupScriptComponent
        //COMPONENT& component = gameObject->AddComponentInternal<COMPONENT>();
        //component.gameObject = gameObject;
        //component.id = id;
        //component.SetActive(active);
}

bool BuildScripts(std::filesystem::path projectPath, std::filesystem::path buildPath)
{
    //std::filesystem::copy_file(std::filesystem::path(__FILE__), buildPath / "Components" / "ScriptLoader.cpp");
    //std::filesystem::copy_file(std::filesystem::path(__FILE__).parent_path() / "ScriptLoader.h", buildPath / "Components" / "ScriptLoader.h");

    std::filesystem::path scriptLoaderPath = buildPath / "Components" / "Scripting" / "ScriptLoader.cpp";

    std::vector<std::string> scriptNames;
    std::vector<std::filesystem::path> paths;

    std::filesystem::remove_all(buildPath / "Components" / "Custom"); // Removes the directory, else it will keep deleted scripts

    std::filesystem::create_directories(buildPath / "Components" / "Custom");

    paths.push_back(scriptLoaderPath);
	for (const auto& entry : std::filesystem::recursive_directory_iterator(projectPath))
	{
		if (std::filesystem::is_regular_file(entry.path()))
		{
            if (entry.path().extension() != ".h" && entry.path().extension() != ".cpp" && entry.path().extension() != ".c")
                continue;

			std::filesystem::path relativePath = std::filesystem::relative(entry.path(), projectPath);

			std::filesystem::path destPath = buildPath / "Components" / "Custom" / relativePath.parent_path();
			std::filesystem::create_directories(destPath);

			std::filesystem::copy_file(entry.path(), destPath / entry.path().filename(), std::filesystem::copy_options::overwrite_existing);

			if (entry.path().extension() == ".h")
				scriptNames.push_back(entry.path().stem().string());

			paths.push_back(destPath / entry.path().filename());
		}
	}


    for (const std::filesystem::path path : paths)
    {
        std::ifstream fileIn(path);

        if (!fileIn.is_open())
        {
            ConsoleLogger::ErrorLog("Build - ScriptLoader failed to build, terminating build. Error Code 800. Path " + path.string(), false);
            return false;
        }

        std::vector<std::string> lines;
        std::string line;

        if (path == scriptLoaderPath)
        {

            while (std::getline(fileIn, line))
                lines.push_back(line);

            auto insertionPoint = std::find(lines.begin(), lines.end(), "// SetupScriptComponent");

            if (insertionPoint != lines.end())
            {
                for (const std::string name : scriptNames)
                    lines.insert(lines.begin(), "#include \"" + name + ".h\"");

                insertionPoint = std::find(lines.begin(), lines.end(), "// SetupScriptComponent");
                auto functionInsertionIndex = std::distance(lines.begin(), insertionPoint) + 1;
                for (const std::string& name : scriptNames)
                {
                    lines.insert(lines.begin() + functionInsertionIndex, "else if (scriptName == \"" + name + "\") {" + name + "& component = gameObject->AddComponentInternal<" + name + ">(); component.gameObject = gameObject; component.id = id; component.SetActive(active); }");
                    functionInsertionIndex++;
                }
            }
            else
            {
                ConsoleLogger::ErrorLog("Build - ScriptLoader failed to build, terminating build. Error Code 801.", false);
                return false;
            }
        }
        else
        {
            // Removed because the game engine shouldn't be responsible for this, and there could be issues if people are using a library or something
            //bool found = false;
            //while (std::getline(fileIn, line)) {
            //    if (!found && line.find("CryonicAPI.h") != std::string::npos)
            //    {
            //        line = "#include \"CryonicAPI.h\"";;
            //        found = true;
            //    }
            //    lines.push_back(line);
            //}

            // Added this since the lines above have been commented out
            // Todo: We can likely just return true at the end of the if statement above
			fileIn.close();
            continue;
        }

        fileIn.close();

        std::ofstream fileOut(path);
        if (!fileOut.is_open())
        {
            ConsoleLogger::ErrorLog("Build - ScriptLoader failed to build, terminating build. Error Code 802.", false);
            return false;
        }

        for (const std::string& updatedLine : lines)
            fileOut << updatedLine << '\n';

        fileOut.close();
    }

    return true;
}