#include "ProjectManager.h"
#include "ConsoleLogger.h"
#include <cstdlib>
#include <direct.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "Scenes/SceneManager.h"
#include "Utilities.h"
#include "Components/ScriptLoader.h"
#include "json.hpp"
#include "imgui.h"
#include <vector>
#include "ImGuiPopup.h"
#include <Windows.h>
#include <iostream>
#include <regex>
#include <chrono>
#include <ctime>
#include <cstdio>

using json = nlohmann::json;

ProjectData ProjectManager::projectData;

void ProjectManager::CopyApiFiles(std::filesystem::path source, std::filesystem::path destination)
{
    std::vector<std::string> filesToCopy = { "CryonicAPI", "CryonicCore", "Scenes", "ConsoleLogger", "FontManager", "Font", "Sprite", "GameObject", "Components", "ShaderManager", "MenuManager", "RenderableTexture", "InputSystem", "CollisionListener2D", "Event", "EventSystem", "Animation", "AnimationGraph", "AnimationBlending", "MotionMatchingSystem", "AudioClip", "Tilemap", "Material", "Physics2DDebugDraw", "ShadowManager", "RaylibInputWrapper", "Wrappers", "RaylibCameraWrapper", "RaylibDrawWrapper", "RaylibLightWrapper", "RaylibModelWrapper", "RaylibShaderWrapper", "RaylibWrapper"};
    
    if (projectData.is3D)
    {
        filesToCopy.push_back("CollisionListener3D");
        filesToCopy.push_back("Physics3DDebugDraw"); // Todo: Remove this
    }

    if (!std::filesystem::exists(destination))
        std::filesystem::create_directories(destination);

    for (const auto& file : std::filesystem::directory_iterator(source))
    {
        if (std::find(filesToCopy.begin(), filesToCopy.end(), file.path().stem()) != filesToCopy.end())
        {
            if (file.is_directory())
            {
                if (!std::filesystem::exists(destination / file.path().filename()))
                    std::filesystem::create_directory(destination / file.path().filename());
                std::filesystem::copy(file.path(), destination / file.path().filename(), std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
            }
            else
                std::filesystem::copy(file.path(), destination);
        }
    }

    // Remove 3D files if its a 2D game
    if (!projectData.is3D)
    {
        std::filesystem::remove(destination / "Components" / "Collider3D.cpp");
        std::filesystem::remove(destination / "Components" / "Collider3D.h");
        std::filesystem::remove(destination / "Components" / "Rigidbody3D.cpp");
        std::filesystem::remove(destination / "Components" / "Rigidbody3D.h");
    }
}

void ProjectManager::CopyAssetFiles(std::filesystem::path destination)
{
    // Todo: Only copy scenes Build Scenes, and only copy resources being used in a build scene.
    // Todo: Put files in an archive format, and give the option to embed it into the exe. When game is built, generate a random key, encode the archive file with that key, and put the key in the game.cpp before its compiled. Maybe in the Game.cpp, "#define DecryptionKey keyHere" so then just use "DecryptionKey" where its needed, or make a const variable

    if (!std::filesystem::exists(destination))
        std::filesystem::create_directories(destination);

    for (const auto& file : std::filesystem::directory_iterator(ProjectManager::projectData.path / "Assets"))
    {
        if (file.is_directory())
        {
            // Todo: Make sure this doesn't copy scripts since they're already compiled in the exe
            if (!std::filesystem::exists(destination / file.path().filename()))
                std::filesystem::create_directory(destination / file.path().filename());
            std::filesystem::copy(file.path(), destination / file.path().filename(), std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
        }
        else if (file.path().extension() != ".cpp" && file.path().extension() != ".h")
            std::filesystem::copy(file.path(), destination);
    }
}

int ProjectManager::CreateProject(ProjectData projectData) // Todo: Add try-catch
{
    projectData.path = projectData.path / projectData.name;
    //std::filesystem::path path = projectData.path / projectData.name;
    ConsoleLogger::InfoLog("Project \"" + projectData.name + "\" is being created at the path \"" + projectData.path.string() + "\"");

    std::filesystem::create_directory(projectData.path);
    if (!std::filesystem::exists(projectData.path)) return 1;

    std::filesystem::create_directory(projectData.path / "api");
    std::filesystem::create_directory(projectData.path / "Settings");
    std::filesystem::create_directory(projectData.path / "Plugins");
    std::filesystem::create_directory(projectData.path / "Internal");
    std::filesystem::create_directory(projectData.path / "Assets");
    std::filesystem::create_directory(projectData.path / "Assets" / "Scripts");
    std::filesystem::create_directory(projectData.path / "Assets" / "Scenes");
    std::filesystem::create_directory(projectData.path / "Assets" / "Audio");

    Utilities::HideFile(projectData.path / "api");

    std::ofstream file(projectData.path / "assets.json");
    if (file.is_open())
    {
        file << nlohmann::json::array().dump(4);
        file.close();
    }

    // Todo: __FILE__ won't work on other computers. I should store
    CopyApiFiles(std::filesystem::path(__FILE__).parent_path(), projectData.path / "api");
    // Todo: copy internal shaders

    switch (projectData.templateData._template)
    {
    case Sidescroller3D:
    case Blank3D:
        std::filesystem::create_directory(projectData.path / "Assets" / "Models");
        std::filesystem::create_directory(projectData.path / "Assets" / "Materials");
        std::filesystem::create_directory(projectData.path / "Assets" / "Textures");
        std::filesystem::create_directory(projectData.path / "Assets" / "Animations");
        break;
    case Blank2D:
        std::filesystem::create_directory(projectData.path / "Assets" / "Sprites");
        break;
    default:
        break;
    }

    SaveProjectData(projectData);

    return 0;
}

void ProjectManager::CleanupBuildFolder(std::filesystem::path path)
{
    try
    {
        for (const auto& file : std::filesystem::directory_iterator(path))
        {
            try
            {
                if (file.is_directory())
                {
                    if (file.path().filename() != "Resources" && file.path().filename() != "CMakeFilesBackup" && file.path().filename().string() != "CMakeFiles")
                        std::filesystem::remove_all(file.path());
                }
                else if (file.path().extension() != ".exe" && file.path().extension() != ".html" && file.path().extension() != ".data" && file.path().extension() != ".js" && file.path().extension() != ".wasm" && file.path().filename().string() != "cmake_install.cmake" && file.path().filename().string() != "CMakeCache.txt" && file.path().filename().string() != "CMakeLists.txt" && file.path().filename().string() != "Makefile")
                    std::filesystem::remove(file.path());
            }
            catch (const std::exception& e) {
                ConsoleLogger::ErrorLog("Build - Error deleting file: " + file.path().string() + " while cleaning up. Error: " + e.what());
                continue;
            }
        }
    }
    catch (const std::exception& e) {
        ConsoleLogger::ErrorLog("Build - Error during build cleanup: " + (std::string)e.what());
        return;
    }
}

//bool ProjectManager::ExtractZip(const char* zip_path, const char* dest_folder)
//{
//    mz_zip_archive zip_archive;
//    mz_bool status;
//
//    memset(&zip_archive, 0, sizeof(zip_archive));
//    status = mz_zip_reader_init_file(&zip_archive, zip_path, 0);
//    if (!status) {
//        printf("Failed to open ZIP file\n");
//        return false;
//    }
//
//    int num_files = mz_zip_reader_get_num_files(&zip_archive);
//    for (int i = 0; i < num_files; ++i) {
//        mz_zip_archive_file_stat file_stat;
//        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
//            printf("Failed to get file info\n");
//            mz_zip_reader_end(&zip_archive);
//            return false;
//        }
//
//        if (mz_zip_reader_is_file_a_directory(&zip_archive, i)) {
//            // Directory entry - create folder if needed
//            // You might want to handle creating directories here
//        }
//        else {
//            mz_uint uncompressed_size = static_cast<mz_uint>(file_stat.m_uncomp_size);
//            void* file_data = malloc(uncompressed_size);
//
//            if (!file_data) {
//                printf("Failed to allocate memory\n");
//                mz_zip_reader_end(&zip_archive);
//                return false;
//            }
//
//            if (!mz_zip_reader_extract_to_mem(&zip_archive, i, file_data, uncompressed_size, 0)) {
//                printf("Failed to extract file\n");
//                free(file_data);
//                mz_zip_reader_end(&zip_archive);
//                return false;
//            }
//
//            char file_path[256]; // Adjust size as needed
//            sprintf_s(file_path, sizeof(file_path), "%s/%s", dest_folder, file_stat.m_filename);
//
//            FILE* file;
//            if (fopen_s(&file, file_path, "wb") != 0 || !file) {
//                printf("Failed to create file %s\n", file_path);
//                free(file_data);
//                mz_zip_reader_end(&zip_archive);
//                return false;
//            }
//
//            fwrite(file_data, 1, uncompressed_size, file);
//            fclose(file);
//            free(file_data);
//        }
//    }
//
//    mz_zip_reader_end(&zip_archive);
//    return true;
//}

void ProjectManager::BackupCMakeFiles(std::filesystem::path buildPath, std::filesystem::path backupPath)
{
    try
    {
        if (std::filesystem::exists(backupPath))
            std::filesystem::remove_all(backupPath);

        std::filesystem::create_directory(backupPath);

        for (const auto& file : std::filesystem::directory_iterator(buildPath))
        {
            try
            {
                std::vector<std::string> cMakeFiles = { "CMakeFiles", "cmake_install.cmake", "CMakeCache.txt", "CMakeLists.txt", "Makefile"};
                if (std::find(cMakeFiles.begin(), cMakeFiles.end(), file.path().filename()) != cMakeFiles.end())
                {
                    if (std::filesystem::is_directory(file.path()))
                        std::filesystem::copy(file.path(), backupPath / file.path().filename());
                    else
                        std::filesystem::copy(file.path(), backupPath);
                }
            }
            catch (const std::exception& e) {
                ConsoleLogger::ErrorLog("Build - Error backing up CMake file: " + file.path().string() + ". Error: " + e.what());
                continue;
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        if (ConsoleLogger::showDebugMessages)
            ConsoleLogger::ErrorLog("Build - Error backing up CMake files. Error: " + (std::string)e.what(), false);
    }
}

void ProjectManager::RestoreCMakeFiles(std::filesystem::path buildPath, std::filesystem::path backupPath)
{
    try
    {
        if (!std::filesystem::exists(backupPath))
            return;

        for (const auto& file : std::filesystem::directory_iterator(backupPath))
        {
            try {
                if (std::filesystem::exists(buildPath / file.path().filename()))
                    std::filesystem::remove_all(buildPath / file.path().filename());

                if (std::filesystem::is_directory(file.path()))
                    std::filesystem::copy(file.path(), buildPath / file.path().filename());
                else
                    std::filesystem::copy(file.path(), buildPath);
            }
            catch (const std::exception& e) {
                ConsoleLogger::ErrorLog("Build - Error restoring CMake file: " + file.path().string() + ". Error: " + e.what());
                continue;
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        if (ConsoleLogger::showDebugMessages)
            ConsoleLogger::ErrorLog("Build - Error restoring up CMake files. Error: " + (std::string)e.what(), false);
    }
}

bool ProjectManager::SetGameSettings(std::filesystem::path gameFile)
{
    // Todo: Add try-catch
    std::ifstream inputFile(gameFile);
    std::ofstream outputFile(gameFile.parent_path() / "TempGame.cpp");

    if (!inputFile)
    {
        ConsoleLogger::ErrorLog("Build - Error opening Game.cpp. Terminating build.", false);
        return false;
    }
    if (!outputFile)
    {
        ConsoleLogger::ErrorLog("Build - Error creating temporary game.cpp file. Terminating build.", false);
        return false;
    }

    std::string line;
    while (std::getline(inputFile, line))
    {
        if (line.find("float timeStep = 1.0f / 60.0f;") != std::string::npos)
            outputFile << "float timeStep = " + std::to_string(projectData.physicsTimeStep.x) + "f / " + std::to_string(projectData.physicsTimeStep.y) + "f;";
        else if (line.find("int32 velocityIterations = 8;") != std::string::npos)
            outputFile << "int32 velocityIterations = " + std::to_string(projectData.velocityIterations) + ";";
        else if (line.find("int32 positionIterations = 3;") != std::string::npos)
            outputFile << "int32 positionIterations = " + std::to_string(projectData.positionIterations) + ";";
        else if (line.find("RaylibWrapper::SetTargetFPS(60);") != std::string::npos)
            outputFile << "RaylibWrapper::SetTargetFPS(" + std::to_string(projectData.maxFPS) + ");";
        else if (line.find("RaylibWrapper::SetWindowMinSize(100, 100);") != std::string::npos)
            outputFile << "RaylibWrapper::SetWindowMinSize(" + std::to_string(static_cast<int>(projectData.minimumResolution.x)) + "," + std::to_string(static_cast<int>(projectData.minimumResolution.y)) + ");";
        else if (line.find("RaylibWrapper::InitWindow(RaylibWrapper::GetScreenWidth(), RaylibWrapper::GetScreenHeight(), (NAME));") != std::string::npos)
            outputFile << "RaylibWrapper::InitWindow(" + std::to_string(static_cast<int>(projectData.windowResolution.x)) + "," + std::to_string(static_cast<int>(projectData.windowResolution.y)) + ", (NAME));";
        else if (projectData.displayMode == 1 && line.find("//RaylibWrapper::ToggleFullscreen();") != std::string::npos)
            outputFile << "RaylibWrapper::ToggleFullscreen();";
        else if (projectData.displayMode == 0 && line.find("//RaylibWrapper::ToggleBorderlessWindowed();") != std::string::npos)
            outputFile << "RaylibWrapper::ToggleBorderlessWindowed();";
        else if (projectData.displayMode == 2 && line.find("//if (RaylibWrapper::GetScreenWidth() == RaylibWrapper::GetMonitorWidth(RaylibWrapper::GetCurrentMonitor()) && RaylibWrapper::GetScreenHeight() == RaylibWrapper::GetMonitorHeight(RaylibWrapper::GetCurrentMonitor())) RaylibWrapper::MaximizeWindow();") != std::string::npos)
            outputFile << "if (RaylibWrapper::GetScreenWidth() == RaylibWrapper::GetMonitorWidth(RaylibWrapper::GetCurrentMonitor()) && RaylibWrapper::GetScreenHeight() == RaylibWrapper::GetMonitorHeight(RaylibWrapper::GetCurrentMonitor())) RaylibWrapper::MaximizeWindow();";
        else if (line.find("RaylibWrapper::SetConfigFlags(0);") != std::string::npos)
        {
            std::string flags = "0";
            if (projectData.resizableWindow)
            {
                if (flags == "0")
                    flags = "RaylibWrapper::FLAG_WINDOW_RESIZABLE";
                else
                    flags += " | RaylibWrapper::FLAG_WINDOW_RESIZABLE";
            }
            if (projectData.highDPI)
            {
                if (flags == "0")
                    flags = "RaylibWrapper::FLAG_WINDOW_HIGHDPI";
                else
                    flags += " | RaylibWrapper::FLAG_WINDOW_HIGHDPI";
            }
            if (projectData.antialiasing)
            {
                if (flags == "0")
                    flags = "RaylibWrapper::FLAG_MSAA_4X_HINT";
                else
                    flags += " | RaylibWrapper::FLAG_MSAA_4X_HINT";
            }
            if (projectData.vsync)
            {
                if (flags == "0")
                    flags = "RaylibWrapper::FLAG_VSYNC_HINT";
                else
                    flags += " | RaylibWrapper::FLAG_VSYNC_HINT";
            }
            if (projectData.runInBackground)
            {
                if (flags == "0")
                    flags = "RaylibWrapper::FLAG_WINDOW_ALWAYS_RUN";
                else
                    flags += " | RaylibWrapper::FLAG_WINDOW_ALWAYS_RUN";
            }
            outputFile << "RaylibWrapper::SetConfigFlags(" + flags + ");";
        }
        else if (line.find("SceneManager::LoadScene(\"Resources/Assets/Scenes/Default.scene\");") != std::string::npos)
            outputFile << "SceneManager::LoadScene(\"Resources/Assets/" + projectData.defaultScenePath + "\");";
        else if (line.find("SceneManager::LoadScene(exeParent / \"Resources\" / \"Assets\" / \"Scenes\" / \"Default.scene\");") != std::string::npos)
            outputFile << "SceneManager::LoadScene(exeParent / \"Resources\" / \"Assets\" / \"" + projectData.defaultScenePath + "\");";
        else
            outputFile << line << '\n';
    }

    inputFile.close();
    outputFile.close();

    std::remove(gameFile.string().c_str());
    std::rename((gameFile.parent_path() / "TempGame.cpp").string().c_str(), gameFile.string().c_str());
    return true;
}

void CancelBuild()
{
    ImGuiPopup::Cancel();
    ImGuiPopup::SetContent("Cancelling build...");
}

bool ProjectManager::PrepareBuild(std::string platform, std::string& projectName, std::filesystem::path& outputPath, std::filesystem::path& originalPath, std::filesystem::path& buildPath, ProjectData& projectData, bool debug, std::function<void(int, bool)>& callback) // General build function. Used in BuildToWindows() and BuildToWeb()
{
    if (projectData.defaultScenePath.empty() || projectData.defaultScenePath == "None" || !std::filesystem::exists(projectData.path / "Assets" / projectData.defaultScenePath))
    {
        if (projectData.defaultScenePath != "None")
        {
            projectData.defaultScenePath = "None";
            SaveProjectData(projectData);
        }
        ConsoleLogger::ErrorLog("Build - Build failed. Invalid default scene. Please set a valid default scene in the Project Settings.");
        callback(0, debug);
        return false;
    }


    ImGuiPopup::Create("Building to " + platform, "Saving scenes...", { {"Cancel", CancelBuild} }, true);

    // Backing up and restoring cmake files is completely useless since when cmake files are moved, it breaks incremental builds. Not sure if this is an issue with CMake, or my code.
    SceneManager::SaveScene(SceneManager::GetActiveScene());

    if (ImGuiPopup::IsCancelled())
    {
        ImGuiPopup::SetActive(false);
        callback(2, debug);
        return false;
    }
    else
    {
        ImGuiPopup::SetContent("Creating directories...");
        ImGuiPopup::SetProgress(5);
    }

    outputPath = projectData.path / "Builds" / platform;

    try
    {
        if (!std::filesystem::exists(outputPath))
            std::filesystem::create_directories(outputPath);

        if (debug)
            buildPath = projectData.path / "Internal" / "Builds" / platform / "Debug";
        else
            buildPath = projectData.path / "Internal" / "Builds" / platform / "Release";

        if (!std::filesystem::exists(buildPath))
            std::filesystem::create_directories(buildPath);

        // Todo: Removing folders to prevent crash although this shouldn't be handled here.
        if (std::filesystem::exists(buildPath / "Resources"))
            std::filesystem::remove_all(buildPath / "Resources");
        if (std::filesystem::exists(buildPath / "Source"))
            std::filesystem::remove_all(buildPath / "Source");
        // Iterating to remove exe's incase the project name changed
        for (const auto& file : std::filesystem::directory_iterator(buildPath))
            if (file.path().has_extension() && file.path().extension() == ".exe")
                std::filesystem::remove(file.path());

        //RestoreCMakeFiles(buildPath, buildPath / "CMakeFilesBackup");

        projectName = projectData.name; // Project name with underscores instead of spaces
        std::replace(projectName.begin(), projectName.end(), ' ', '_');

        if (ImGuiPopup::IsCancelled())
        {
            ImGuiPopup::SetActive(false);
            callback(2, debug);
            return false;
        }
        else
        {
            ImGuiPopup::SetContent("Copying files...");
            ImGuiPopup::SetProgress(10);
        }

        // Todo: Using __FILE__ won't work on other computers

        // Copying presets
        std::filesystem::copy(std::filesystem::path(__FILE__).parent_path() / "BuildPresets", buildPath, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
        std::filesystem::copy(buildPath / (projectData.is3D ? "3D" : "2D"), buildPath, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
        std::filesystem::remove_all(buildPath / "3D");
        std::filesystem::remove_all(buildPath / "2D");

        std::filesystem::copy(buildPath / platform, buildPath, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
        std::filesystem::remove_all(buildPath / "Windows");
        std::filesystem::remove_all(buildPath / "Web");

        if (ImGuiPopup::IsCancelled())
        {
            ImGuiPopup::SetActive(false);
            callback(2, debug);
            return false;
        }
        else
        {
            ImGuiPopup::SetContent("Setting game name...");
            ImGuiPopup::SetProgress(15);
        }

        // Changing name of the game in the CMakeList.txt
        std::ifstream cmakeListsFile(buildPath / "CMakeLists.txt");
        if (!cmakeListsFile.is_open())
        {
            ConsoleLogger::ErrorLog("Build - Failed to open the CMakesLists.txt to modify the game's name. Terminating build.", false);
            try {
                for (const auto& file : std::filesystem::directory_iterator(buildPath))
                    if (file.path().filename().string() != "CMakeFilesBackup" && file.path().filename().string() != "CMakeFiles" && file.path().filename().string() != "cmake_install.cmake" && file.path().filename().string() != "CMakeCache.txt" && file.path().filename().string() != "CMakeLists.txt" && file.path().filename().string() != "Makefile")
                        std::filesystem::remove_all(file.path());
            }
            catch (const std::filesystem::filesystem_error& ex) {}
            //RestoreCMakeFiles(buildPath, buildPath / "CMakeFilesBackup");
            callback(0, debug);
            ImGuiPopup::SetActive(false);
            return false;
        }

        std::string contents((std::istreambuf_iterator<char>(cmakeListsFile)), std::istreambuf_iterator<char>());
        cmakeListsFile.close();
        size_t pos = contents.find("project(\"GameName\")");
        if (pos != std::string::npos)
            contents.replace(pos, 19, "project(\"" + projectName + "\")");
        else
        {
            ConsoleLogger::ErrorLog("Build - Failed to set the name of the game. Terminating build.", false);
            try {
                for (const auto& file : std::filesystem::directory_iterator(buildPath))
                    if (file.path().filename().string() != "CMakeFilesBackup" && file.path().filename().string() != "CMakeFiles" && file.path().filename().string() != "cmake_install.cmake" && file.path().filename().string() != "CMakeCache.txt" && file.path().filename().string() != "CMakeLists.txt" && file.path().filename().string() != "Makefile")
                        std::filesystem::remove_all(file.path());
            }
            catch (const std::filesystem::filesystem_error& ex) {}
            //RestoreCMakeFiles(buildPath, buildPath / "CMakeFilesBackup");
            callback(0, debug);
            ImGuiPopup::SetActive(false);
            return false;
        }

        std::ofstream(buildPath / "CMakeLists.txt") << contents;

        if (ImGuiPopup::IsCancelled())
        {
            ImGuiPopup::SetActive(false);
            callback(2, debug);
            return false;
        }
        else
        {
            ImGuiPopup::SetContent("Setting project settings...");
            ImGuiPopup::SetProgress(20);
        }

        // Set values from project settings in Game.cxp
        if (!SetGameSettings(buildPath / "Source" / "Game.cpp"))
        {
            try {
                for (const auto& file : std::filesystem::directory_iterator(buildPath))
                    if (file.path().filename().string() != "CMakeFilesBackup" && file.path().filename().string() != "CMakeFiles" && file.path().filename().string() != "cmake_install.cmake" && file.path().filename().string() != "CMakeCache.txt" && file.path().filename().string() != "CMakeLists.txt" && file.path().filename().string() != "Makefile")
                        std::filesystem::remove_all(file.path());
            }
            catch (const std::filesystem::filesystem_error& ex) {}
            //RestoreCMakeFiles(buildPath, buildPath / "CMakeFilesBackup");
            callback(0, debug);
            ImGuiPopup::SetActive(false);
            return false;
        }

        //std::filesystem::copy_file(std::filesystem::path(__FILE__).parent_path() / "BuildPresets.zip", buildPath / "BuildPresets.zip");

        //ExtractZip((buildPath / "BuildPresets.zip").string().c_str(), buildPath.string().c_str());

        // Todo: Copy internal shader files

        //std::filesystem::create_directory(buildPath / "Scenes");
        //std::filesystem::copy(projectData.path / "Assets" / "Scenes", buildPath / "Scenes");
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        ConsoleLogger::ErrorLog("Build - Error copying BuildPresets directory and/or files. Terminating build. Error: " + (std::string)e.what(), false);
        try {
            for (const auto& file : std::filesystem::directory_iterator(buildPath))
                if (file.path().filename().string() != "CMakeFilesBackup" && file.path().filename().string() != "CMakeFiles" && file.path().filename().string() != "cmake_install.cmake" && file.path().filename().string() != "CMakeCache.txt" && file.path().filename().string() != "CMakeLists.txt" && file.path().filename().string() != "Makefile")
                    std::filesystem::remove_all(file.path());
        }
        catch (const std::filesystem::filesystem_error& ex) {}
        //RestoreCMakeFiles(buildPath, buildPath / "CMakeFilesBackup");
        callback(0, debug);
        ImGuiPopup::SetActive(false);
        return false;
    }

    if (ImGuiPopup::IsCancelled())
    {
        ImGuiPopup::SetActive(false);
        callback(2, debug);
        return false;
    }
    else
    {
        ImGuiPopup::SetContent("Copying API files...");
        ImGuiPopup::SetProgress(25);
    }

    if (!std::filesystem::exists(projectData.path / "api"))
    {
        std::filesystem::create_directories(projectData.path / "api");
        Utilities::HideFile(projectData.path / "api");
    }
    std::filesystem::remove_all(projectData.path / "api");

    // Todo: This won't work on other computers and it can only be used in debugging
    CopyApiFiles(std::filesystem::path(__FILE__).parent_path(), projectData.path / "api");
    CopyApiFiles(projectData.path / "api", buildPath / "Source");

    CopyAssetFiles(buildPath / "Resources" / "Assets");

    if (ImGuiPopup::IsCancelled())
    {
        ImGuiPopup::SetActive(false);
        callback(2, debug);
        return false;
    }
    else
    {
        ImGuiPopup::SetContent("Copying resource files...");
        ImGuiPopup::SetProgress(30);
    }

    // Copies resource files
    try {
        // Todo: Using __FILE__ won't work on other computers
        // Todo: Copy the default GUI font too.
        std::filesystem::copy(std::filesystem::path(__FILE__).parent_path() / "resources" / "shaders", buildPath / "Resources" / "shaders", std::filesystem::copy_options::recursive);
    }
    catch (const std::exception& e) {
        ConsoleLogger::ErrorLog("Build - Failed to copy resource files: " + (std::string)e.what());
        try {
            for (const auto& file : std::filesystem::directory_iterator(buildPath))
                if (file.path().filename().string() != "CMakeFilesBackup" && file.path().filename().string() != "CMakeFiles" && file.path().filename().string() != "cmake_install.cmake" && file.path().filename().string() != "CMakeCache.txt" && file.path().filename().string() != "CMakeLists.txt" && file.path().filename().string() != "Makefile")
                    std::filesystem::remove_all(file.path());
        }
        catch (const std::filesystem::filesystem_error& ex) {}
        //RestoreCMakeFiles(buildPath, buildPath / "CMakeFilesBackup");
        callback(0, debug);
        ImGuiPopup::SetActive(false);
        return false;
    }

    if (ImGuiPopup::IsCancelled())
    {
        ImGuiPopup::SetActive(false);
        callback(2, debug);
        return false;
    }
    else
    {
        ImGuiPopup::SetContent("Building scripts...");
        ImGuiPopup::SetProgress(35);
    }

    if (!BuildScripts(projectData.path / "Assets" / "Scripts", buildPath / "Source")) // Todo: This should handle the error messages, not BuildScripts(). BuildScripts() should return an int
    {
        try {
            for (const auto& file : std::filesystem::directory_iterator(buildPath))
                if (file.path().filename().string() != "CMakeFilesBackup" && file.path().filename().string() != "CMakeFiles" && file.path().filename().string() != "cmake_install.cmake" && file.path().filename().string() != "CMakeCache.txt" && file.path().filename().string() != "CMakeLists.txt" && file.path().filename().string() != "Makefile")
                    std::filesystem::remove_all(file.path());
        }
        catch (const std::filesystem::filesystem_error& ex) {}
        //RestoreCMakeFiles(buildPath, buildPath / "CMakeFilesBackup");
        callback(0, debug);
        ImGuiPopup::SetActive(false);
        return false;
    }

    if (ImGuiPopup::IsCancelled())
    {
        ImGuiPopup::SetActive(false);
        callback(2, debug);
        return false;
    }
    else
    {
        ImGuiPopup::SetContent("Setting exposed variables...");
        ImGuiPopup::SetProgress(40);
    }

    GenerateExposedVariablesFunctions(buildPath / "Source");

    // Saves original path, sets new current path, and then run cmake and mingw32-make
    originalPath = std::filesystem::current_path();
    _chdir(buildPath.string().c_str());
}

bool ProjectManager::BuildToWindows(ProjectData projectData, bool debug, std::function<void(int, bool)> callback) // Todo: Maybe make a .json format file that contains information like scenes and which scene should be first opened
{
    // Check if cmake is installed
    if (!Utilities::IsProgramInstalled("cmake --version"))
    {
        ConsoleLogger::ErrorLog("Build - Failed due to not having cmake installed. Please install cmake or add it to your environment variables.", false);
        callback(0, debug);
        return false;
    }

    // Check if MinGW32 is installed
    if (!Utilities::IsProgramInstalled("mingw32-make --version"))
    {
        ConsoleLogger::ErrorLog("Build - Failed due to not having mingw32-make installed. Please install mingw32-make or add it to your environment variables.", false);
        callback(0, debug);
        return false;
    }

    std::filesystem::path exeParent = Utilities::GetExePath().parent_path();

    // Check if ninja is installed
    bool useNinja = true;
    if (!std::filesystem::exists(exeParent / "tools" / "ninja.exe"))
    {
        // Todo: Prompt to install it via git. Ensure they have git installed.
        useNinja = false;
        ConsoleLogger::WarningLog("Build - It is highly recommended to have Ninja installed. This will allow builds to be quicker.");
    }

    // Check if ccache is installed
    bool useCcache = true;
    if (!std::filesystem::exists(exeParent / "tools" / "ccache.exe"))
    {
        // Todo: Prompt to install it via git. Ensure they have git installed.
        useCcache = false;
        ConsoleLogger::WarningLog("Build - It is highly recommended to have Ccache installed. This will allow builds to be quicker.");
    }

    if (projectData.iconPath.empty() || projectData.iconPath == "None" || !std::filesystem::exists(projectData.path / "Assets" / projectData.iconPath))
    {
        ConsoleLogger::WarningLog("Build - Invalid icon path. Will use the default icon.");
        projectData.iconPath = "None";
    }

    std::string projectName;
    std::filesystem::path originalPath, buildPath, outputPath;

    if (!PrepareBuild("Windows", projectName, outputPath, originalPath, buildPath, projectData, debug, callback))
        return false;

    if (ImGuiPopup::IsCancelled())
    {
        ImGuiPopup::SetActive(false);
        callback(2, debug);
        return false;
    }
    else
    {
        ImGuiPopup::SetContent("Running CMake...");
        ImGuiPopup::SetProgress(45);
    }

    ConsoleLogger::InfoLog("Build - Running cmake", false);

    std::string buildType;
    if (debug)
        buildType = "Debug";
    else
        buildType = "Release";

    //system(("cmake -G \"MinGW Makefiles\" -DCMAKE_BUILD_TYPE=" + buildType + " .").c_str());

    auto replaceString = [](std::string s, const std::string& from, const std::string& to) {
        for (size_t pos = 0; (pos = s.find(from, pos)) != std::string::npos; pos += to.length())
            s.replace(pos, from.length(), to);
        return s;
    };

    std::string version2;
    if (projectData.version.empty())
        version2 = "0,0,0,1";
    else
        version2 = replaceString(projectData.version, ".", ",");

    std::time_t now_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm now_tm;
    localtime_s(&now_tm, &now_time_t);
    std::string copyright = replaceString(projectData.copyright, "{company}", projectData.company);
    copyright = replaceString(copyright, "{year}", std::to_string(now_tm.tm_year + 1900));

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    std::string command;

    //std::string generator = useNinja ? "Ninja" : "MinGW Makefiles";
    //std::string command = "cmake -G \"" + generator + "\" "
    //    "-DCMAKE_BUILD_TYPE=" + buildType + " "
    //    "-DPLATFORM=WINDOWS "
    //    "-DICON_PATH=\"" + (projectData.iconPath == "None" ? "Default Cryonic Logo.ico" : projectData.iconPath) + "\" "
    //    "-DGAME_VERSION=\"" + projectData.version + "\" "
    //    "-DGAME_VERSION_2=\"" + version2 + "\" "
    //    "-DCOMPANY_NAME=\"" + projectData.company + "\" "
    //    "-DGAME_DESCRIPTION=\"" + projectData.description + "\" "
    //    "-DCOPYRIGHT=\"" + copyright + "\" "
    //    ".";

    // Todo: Instead, try to merge the two commands below and just have a string for the top cmake line. 

    if (useNinja)
    {
        command = "cmake -G \"Ninja\" "
            "-DCMAKE_MAKE_PROGRAM=\"" + Utilities::GetExePath().parent_path().string() + "/tools/ninja.exe\" "
            "-DCMAKE_BUILD_TYPE=" + buildType + " "
            "-DPLATFORM=WINDOWS "
            "-DICON_PATH=\"" + (projectData.iconPath == "None" ? "Default Cryonic Logo.ico" : projectData.iconPath) + "\" "
            "-DGAME_VERSION=\"" + projectData.version + "\" "
            "-DGAME_VERSION_2=\"" + version2 + "\" "
            "-DCOMPANY_NAME=\"" + projectData.company + "\" "
            "-DGAME_DESCRIPTION=\"" + projectData.description + "\" "
            "-DCOPYRIGHT=\"" + copyright + "\" "
            "-DCMAKE_C_COMPILER=\"C:/Program Files/LLVM/bin/clang.exe\" "
            "-DCMAKE_CXX_COMPILER=\"C:/Program Files/LLVM/bin/clang++.exe\" ";
    }
    else
    {
        command = "cmake -G \"MinGW Makefiles\" " // Should this be "MinGW Makefiles" if its using clang?
            "-DCMAKE_BUILD_TYPE=" + buildType + " "
            "-DPLATFORM=WINDOWS "
            "-DICON_PATH=\"" + (projectData.iconPath == "None" ? "Default Cryonic Logo.ico" : projectData.iconPath) + "\" "
            "-DGAME_VERSION=\"" + projectData.version + "\" "
            "-DGAME_VERSION_2=\"" + version2 + "\" "
            "-DCOMPANY_NAME=\"" + projectData.company + "\" "
            "-DGAME_DESCRIPTION=\"" + projectData.description + "\" "
            "-DCOPYRIGHT=\"" + copyright + "\" ";
    }

    if (useCcache)
        command += "-DCCACHE_PATH=" + (useCcache ? "\"" + Utilities::GetExePath().parent_path().string() + "/tools/ccache.exe\" " : "\"\"") + " ";
    
    command += ".";


    if (!CreateProcessA(NULL, const_cast<LPSTR>(command.c_str()), NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi))
    {
        ConsoleLogger::ErrorLog("Build - Failed to run CMake");
        ImGuiPopup::SetActive(false);
        callback(0, false);
        return false;
    }

    while (true)
    {
        DWORD waitResult = WaitForSingleObject(pi.hProcess, 100);
        if (waitResult == WAIT_OBJECT_0)
            break;
        else if (waitResult != WAIT_TIMEOUT)
        {
            ConsoleLogger::ErrorLog("Build - Failed to run CMake");
            ImGuiPopup::SetActive(false);
            callback(0, debug);
            return false;
        }
        if (ImGuiPopup::IsCancelled())
        {
            Utilities::TerminateProcess(pi.dwProcessId, 1); // Using a custom TerminateProcess() function since window's TerminateProcess(), and CloseHandle() wouldn't work during MinGW make

            //system(("taskkill /F /T /PID " + std::to_string(pi.dwProcessId)).c_str()); // This works, but is a bad solution

            // This does not work
            //TerminateProcess(pi.hProcess, 2);
            //CloseHandle(pi.hProcess);
            //CloseHandle(pi.hThread);

            ConsoleLogger::InfoLog("CMake cancelled");

            ImGuiPopup::SetActive(false);
            callback(2, debug);
            return false;
        }
    }

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode != 0)
    {
        ConsoleLogger::ErrorLog("Build - Failed to run CMake");
        ImGuiPopup::SetActive(false);
        callback(0, debug);
        return false;
    }

    if (ImGuiPopup::IsCancelled())
    {
        ImGuiPopup::SetActive(false);
        callback(2, debug);
        return false;
    }
    else
    {
        ImGuiPopup::SetContent("Compiling..."); // Todo: Consider getting each message and putting them here, or at least a simplified version from the message
        ImGuiPopup::SetProgress(50);
    }

    ConsoleLogger::InfoLog("Build - Running mingw32-make", false);

    // Creating a pipe so I can get the output from MinGW Make
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hStdoutRead, hStdoutWrite;
    bool pipeCreated = true;

    if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &saAttr, 0))
    {
        ConsoleLogger::ErrorLog("Failed to create pipe for MinGW Make");
        pipeCreated = false;
    }

    // Crating a process and running MinGW Make
    
    //system(("mingw32-make -j" + std::to_string(static_cast<int>(std::round(Utilities::GetNumberOfCores() * 1))) + " PLATFORM=PLATFORM_DESKTOP").c_str());

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (pipeCreated)
    {
        SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);
        if (ConsoleLogger::showDebugMessages)
            si.hStdError = GetStdHandle(STD_ERROR_HANDLE); // This is needed to keep the colours on error messages
        else
            si.hStdError = hStdoutWrite;
        si.hStdOutput = hStdoutWrite;
        si.dwFlags |= STARTF_USESTDHANDLES;
    }

    // Todo: Make the number of cores configurable.
    if (useNinja)
        //command = "ninja -j" + std::to_string(static_cast<int>(std::round(Utilities::GetNumberOfCores() * 1))); // Both this line and the one below seems to offer the same speeds?
        command = "cmake --build . -- -j" + std::to_string(static_cast<int>(std::round(Utilities::GetNumberOfCores() * 1)));
    else
        command = "mingw32-make -j" + std::to_string(static_cast<int>(std::round(Utilities::GetNumberOfCores() * 1))) + " PLATFORM=PLATFORM_DESKTOP";

    if (!CreateProcessA(NULL, const_cast<LPSTR>(command.c_str()), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
    {
        ConsoleLogger::ErrorLog("Build - Failed to run mingw32-make");
        if (pipeCreated)
        {
            CloseHandle(hStdoutRead);
            CloseHandle(hStdoutWrite);
        }
        ImGuiPopup::SetActive(false);
        callback(0, false);
        return false;
    }

    if (pipeCreated)
        CloseHandle(hStdoutWrite);

    char buffer[4096];
    DWORD dwRead;

    while (true)
    {
        DWORD waitResult = WaitForSingleObject(pi.hProcess, 100);
        if (waitResult == WAIT_OBJECT_0)
            break;
        else if (waitResult != WAIT_TIMEOUT)
        {
            ConsoleLogger::ErrorLog("Build - Failed to compile game");
            ImGuiPopup::SetActive(false);
            callback(0, debug);
            if (pipeCreated)
                CloseHandle(hStdoutRead);
            return false;
        }

        if (ImGuiPopup::IsCancelled())
        {
            Utilities::TerminateProcess(pi.dwProcessId, 1); // Using a custom TerminateProcess() function since window's TerminateProcess(), and CloseHandle() wouldn't work during MinGW make

            //system(("taskkill /F /T /PID " + std::to_string(pi.dwProcessId)).c_str()); // This works, but is a bad solution

            // This does not work
            //TerminateProcess(pi.hProcess, 2);
            //CloseHandle(pi.hProcess);
            //CloseHandle(pi.hThread);

            ConsoleLogger::InfoLog("MinGW Make cancelled");

            ImGuiPopup::SetActive(false);
            callback(2, debug);
            if (pipeCreated)
                CloseHandle(hStdoutRead);
            return false;
        }

        if (pipeCreated && ReadFile(hStdoutRead, buffer, sizeof(buffer) - 1, &dwRead, NULL) && dwRead > 0)
        {
            buffer[dwRead] = '\0';
            std::string line(buffer, dwRead);

            std::smatch match;

            if (std::regex_search(line, match, std::regex(R"(Linking CXX executable)")))
            {
                ImGuiPopup::SetContent("Linking executable...");
                ImGuiPopup::SetProgress(90);
            }
            else
            {
                if (useNinja)
                {
                    if (std::regex_search(line, match, std::regex(R"(\[(\d+)/(\d+)\])")))
                        ImGuiPopup::SetProgress(50 + ((std::stoi(match[1]) * 49) / (std::stoi(match[2])))); // MinGW compile is only worth 49% of the total build progress which is why it multiplies by 49 and divides by the total amount
                }
                else if (std::regex_search(line, match, std::regex(R"(\[(\s?\d{1,3})%\])")))
                    ImGuiPopup::SetProgress(50 + ((std::stoi(match[1]) * 49) / 100)); // MinGW compile is only worth 49% of the total build progress which is why it multiplies by 49 and divides by 100
            }

            if (ConsoleLogger::showDebugMessages)
                std::cout << buffer;
        }
    }

    if (pipeCreated)
        CloseHandle(hStdoutRead);
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode != 0)
    {
        ConsoleLogger::ErrorLog("Build - Failed to compile game");
        ImGuiPopup::SetActive(false);
        callback(0, debug);
        return false;
    }

    // Resets the current path
    _chdir(originalPath.string().c_str());

    // Renames the .exe to the project name
    //std::filesystem::rename(buildPath / "Game.exe", buildPath / projectData.name + ".exe.); ------------------------ Fix this

    // Rename exe to replace underscores with spaces

    if (ImGuiPopup::IsCancelled())
    {
        ImGuiPopup::SetActive(false);
        callback(2, debug);
        return false;
    }
    else
    {
        ImGuiPopup::SetContent("Finishing up...");
        ImGuiPopup::SetProgress(99);
    }

    ConsoleLogger::InfoLog("Build - Renaming executable", false);
    try {
        std::filesystem::rename(buildPath / std::string(projectName + ".exe"), buildPath / std::string(projectData.name + ".exe"));
    }
    catch (const std::filesystem::filesystem_error& ex)
    {
        if (ConsoleLogger::showDebugMessages)
            ConsoleLogger::WarningLog(std::string("Build - The executable could not be renamed. Error: ") + ex.what());
        else
            ConsoleLogger::WarningLog("Build - The executable failed to be renamed");
    }

    // Backing up CMake Files for incremental builds
    //BackupCMakeFiles(buildPath, buildPath / "CMakeFilesBackup");

    // Cleanup
    if (!debug)
    {
        ConsoleLogger::InfoLog("Build - Cleaning up", false);
        CleanupBuildFolder(buildPath);
    }

    // Move files for Release builds
    if (!debug)
    {
        try
        {
            if (std::filesystem::exists(outputPath))
                for (auto& file : std::filesystem::directory_iterator(outputPath))
                    std::filesystem::remove_all(file);

            for (const auto& file : std::filesystem::directory_iterator(buildPath))
            {
                if (file.path().filename().string() != "CMakeFilesBackup" && file.path().filename().string() != "CMakeFiles" && file.path().filename().string() != "cmake_install.cmake" && file.path().filename().string() != "CMakeCache.txt" && file.path().filename().string() != "CMakeLists.txt" && file.path().filename().string() != "Makefile")
                {
                    //if (std::filesystem::exists(outputPath / file.path().filename()))
                    //    std::filesystem::remove_all(outputPath / file.path().filename());
                    std::filesystem::rename(file.path(), outputPath / file.path().filename());
                }
            }
        }
        catch (const std::filesystem::filesystem_error& ex)
        {
            if (ConsoleLogger::showDebugMessages)
                ConsoleLogger::ErrorLog(std::string("Build - There was an error moving build files to your selected build folder. Terminating build. Error: ") + ex.what());
            else
                ConsoleLogger::ErrorLog("Build - There was an error moving build files to your selected build folder. Terminating build.");

            try {
                for (const auto& file : std::filesystem::directory_iterator(buildPath))
                    if (file.path().filename().string() != "CMakeFilesBackup" && file.path().filename().string() != "CMakeFiles" && file.path().filename().string() != "cmake_install.cmake" && file.path().filename().string() != "CMakeCache.txt" && file.path().filename().string() != "CMakeLists.txt" && file.path().filename().string() != "Makefile")
                        std::filesystem::remove_all(file.path());
            }
            catch (const std::filesystem::filesystem_error& ex) {}

            callback(0, debug);
            ImGuiPopup::SetActive(false);
            return false;
        }

        try {
            for (const auto& file : std::filesystem::directory_iterator(buildPath))
                if (file.path().filename().string() != "CMakeFilesBackup" && file.path().filename().string() != "CMakeFiles" && file.path().filename().string() != "cmake_install.cmake" && file.path().filename().string() != "CMakeCache.txt" && file.path().filename().string() != "CMakeLists.txt" && file.path().filename().string() != "Makefile")
                    std::filesystem::remove_all(file.path());
        }
        catch (const std::filesystem::filesystem_error& ex) {}
    }

    ConsoleLogger::InfoLog("Build - Build complete", false);

    if (!debug)
        Utilities::OpenPathInExplorer(outputPath);

    callback(1, debug);
    ImGuiPopup::SetActive(false);
    return true;
}

bool ProjectManager::BuildToWeb(ProjectData projectData, std::function<void(int, bool)> callback)
{
    // Check if EMSDK is installed
    if (!std::filesystem::exists("C:\\emsdk")) // Todo: Needs to be changed when emsdk is moved to a path relative to Cryonic Engine
    {
        ConsoleLogger::ErrorLog("Build - Failed due to not having emsdk installed. Please install emsdk at \"C:\\emsdk\".", false);
        callback(0, false);
        return false;
    }

    // Check if EMSDK environment bat exists
    if (!std::filesystem::exists("C:\\emsdk\\emsdk_env.bat")) // Todo: Needs to be changed when emsdk is moved to a path relative to Cryonic Engine
    {
        ConsoleLogger::ErrorLog("Build - Failed due to not having emsdk's environment bat installed. Please reinstall emsdk at \"C:\\emsdk\".", false);
        callback(0, false);
        return false;
    }

    std::string projectName;
    std::filesystem::path originalPath, buildPath, outputPath;

    if (!PrepareBuild("Web", projectName, outputPath, originalPath, buildPath, projectData, false, callback))
        return false;

    if (ImGuiPopup::IsCancelled())
    {
        ImGuiPopup::SetActive(false);
        callback(2, false);
        return false;
    }
    else
    {
        ImGuiPopup::SetContent("Running emcmake...");
        ImGuiPopup::SetProgress(45);
    }

    ConsoleLogger::InfoLog("Build - Running emcmake", false);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    std::string command = "cmd.exe /C \"C:\\emsdk\\emsdk_env.bat && emcmake cmake . -DCMAKE_BUILD_TYPE=Release -DPLATFORM=WEB\"";
    //std::string command = "emcmake cmake . -DCMAKE_BUILD_TYPE=Release -DPLATFORM=WEB";

    if (!CreateProcessA(NULL, const_cast<LPSTR>(command.c_str()), NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi))
    {
        ConsoleLogger::ErrorLog("Build - Failed to run emcmake");
        ImGuiPopup::SetActive(false);
        callback(0, false);
        return false;
    }

    while (true)
    {
        DWORD waitResult = WaitForSingleObject(pi.hProcess, 100);
        if (waitResult == WAIT_OBJECT_0)
            break;
        else if (waitResult != WAIT_TIMEOUT)
        {
            ConsoleLogger::ErrorLog("Build - Failed to run emcmake");
            ImGuiPopup::SetActive(false);
            callback(0, false);
            return false;
        }
        if (ImGuiPopup::IsCancelled())
        {
            Utilities::TerminateProcess(pi.dwProcessId, 1); // Using a custom TerminateProcess() function since window's TerminateProcess(), and CloseHandle() wouldn't work during MinGW make
            ConsoleLogger::InfoLog("emcmake cancelled");

            ImGuiPopup::SetActive(false);
            callback(2, false);
            return false;
        }
    }

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode != 0)
    {
        ConsoleLogger::ErrorLog("Build - Failed to run emcmake");
        ImGuiPopup::SetActive(false);
        callback(0, false);
        return false;
    }

    if (ImGuiPopup::IsCancelled())
    {
        ImGuiPopup::SetActive(false);
        callback(2, false);
        return false;
    }
    else
    {
        ImGuiPopup::SetContent("Compiling..."); // Todo: Consider getting each message and putting them here, or at least a simplified version from the message
        ImGuiPopup::SetProgress(50);
    }

    ConsoleLogger::InfoLog("Build - Running emmake", false);

    // Creating a pipe so I can get the output from MinGW Make
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hStdoutRead, hStdoutWrite;
    bool pipeCreated = true;

    if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &saAttr, 0))
    {
        ConsoleLogger::ErrorLog("Failed to create pipe for emmake");
        pipeCreated = false;
    }

    // Crating a process and running emmake make

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (pipeCreated)
    {
        SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);
        if (ConsoleLogger::showDebugMessages)
            si.hStdError = GetStdHandle(STD_ERROR_HANDLE); // This is needed to keep the colours on error messages
        else
            si.hStdError = hStdoutWrite;
        si.hStdOutput = hStdoutWrite;
        si.dwFlags |= STARTF_USESTDHANDLES;
    }

    //command = "mingw32-make -j" + std::to_string(static_cast<int>(std::round(Utilities::GetNumberOfCores() * 1))) + " PLATFORM=PLATFORM_DESKTOP"; // Todo: Make the number of cores configurable, and default at 75%. Also make sure its at least 1.
    command = "cmd.exe /C \"C:\\emsdk\\emsdk_env.bat && emmake make -j"  + std::to_string(static_cast<int>(std::round(Utilities::GetNumberOfCores() * 1))) + "\"";

    if (!CreateProcessA(NULL, const_cast<LPSTR>(command.c_str()), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
    {
        ConsoleLogger::ErrorLog("Build - Failed to run emmake");
        if (pipeCreated)
        {
            CloseHandle(hStdoutRead);
            CloseHandle(hStdoutWrite);
        }
        ImGuiPopup::SetActive(false);
        callback(0, false);
        return false;
    }

    if (pipeCreated)
        CloseHandle(hStdoutWrite);

    char buffer[4096];
    DWORD dwRead;

    while (true)
    {
        DWORD waitResult = WaitForSingleObject(pi.hProcess, 100);
        if (waitResult == WAIT_OBJECT_0)
            break;
        else if (waitResult != WAIT_TIMEOUT)
        {
            ConsoleLogger::ErrorLog("Build - Failed to compile game");
            ImGuiPopup::SetActive(false);
            callback(0, false);
            if (pipeCreated)
                CloseHandle(hStdoutRead);
            return false;
        }

        if (ImGuiPopup::IsCancelled())
        {
            Utilities::TerminateProcess(pi.dwProcessId, 1); // Using a custom TerminateProcess() function since window's TerminateProcess(), and CloseHandle() wouldn't work during MinGW make

            ConsoleLogger::InfoLog("emmake cancelled");

            ImGuiPopup::SetActive(false);
            callback(2, false);
            if (pipeCreated)
                CloseHandle(hStdoutRead);
            return false;
        }

        if (pipeCreated && ReadFile(hStdoutRead, buffer, sizeof(buffer) - 1, &dwRead, NULL) && dwRead > 0)
        {
            buffer[dwRead] = '\0';
            std::string line(buffer, dwRead);

            std::smatch match;

            if (std::regex_search(line, match, std::regex(R"(Linking CXX executable)")))
            {
                ImGuiPopup::SetContent("Linking html...");
                ImGuiPopup::SetProgress(90);
            }
            else if (std::regex_search(line, match, std::regex(R"(\[(\s?\d{1,3})%\])")))
                ImGuiPopup::SetProgress(50 + ((std::stoi(match[1]) * 40) / 100)); // emmake compile is only worth 40% of the total build progress which is why it multiplies by 40 and divides by 100

            if (ConsoleLogger::showDebugMessages)
                std::cout << buffer;
        }
    }

    if (pipeCreated)
        CloseHandle(hStdoutRead);
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode != 0)
    {
        ConsoleLogger::ErrorLog("Build - Failed to compile game");
        ImGuiPopup::SetActive(false);
        callback(0, false);
        return false;
    }

    // Resets the current path
    _chdir(originalPath.string().c_str());

    // Renames the .exe to the project name
    //std::filesystem::rename(buildPath / "Game.exe", buildPath / projectData.name + ".exe.); ------------------------ Fix this

    // Rename exe to replace underscores with spaces

    if (ImGuiPopup::IsCancelled())
    {
        ImGuiPopup::SetActive(false);
        callback(2, false);
        return false;
    }
    else
    {
        ImGuiPopup::SetContent("Finishing up...");
        ImGuiPopup::SetProgress(95);
    }

    //ConsoleLogger::InfoLog("Build - Renaming executable", false);
    //try {
    //    std::filesystem::rename(buildPath / std::string(projectName + ".exe"), buildPath / std::string(projectData.name + ".exe"));
    //}
    //catch (const std::filesystem::filesystem_error& ex)
    //{
    //    if (ConsoleLogger::showDebugMessages)
    //        ConsoleLogger::WarningLog(std::string("Build - The executable could not be renamed. Error: ") + ex.what());
    //    else
    //        ConsoleLogger::WarningLog("Build - The executable failed to be renamed");
    //}

    // Backing up CMake Files for incremental builds
    //BackupCMakeFiles(buildPath, buildPath / "CMakeFilesBackup");

    // Cleanup
    ConsoleLogger::InfoLog("Build - Cleaning up", false);
    CleanupBuildFolder(buildPath);

    // Move files to output directory
    try
    {
        if (std::filesystem::exists(outputPath))
            for (auto& file : std::filesystem::directory_iterator(outputPath))
                std::filesystem::remove_all(file);

        for (const auto& file : std::filesystem::directory_iterator(buildPath))
        {
            if (file.path().filename().string() != "CMakeFilesBackup" && file.path().filename().string() != "CMakeFiles" && file.path().filename().string() != "cmake_install.cmake" && file.path().filename().string() != "CMakeCache.txt" && file.path().filename().string() != "CMakeLists.txt" && file.path().filename().string() != "Makefile" && file.path().filename().string() != "Resources" && file.path().filename().string() != "Template.html")
            {
                //if (std::filesystem::exists(path / file.path().filename()))
                //    std::filesystem::remove_all(path / file.path().filename());
                std::filesystem::rename(file.path(), outputPath / file.path().filename());
            }
        }
    }
    catch (const std::filesystem::filesystem_error& ex)
    {
        if (ConsoleLogger::showDebugMessages)
            ConsoleLogger::ErrorLog(std::string("Build - There was an error moving build files to your selected build folder. Terminating build. Error: ") + ex.what());
        else
            ConsoleLogger::ErrorLog("Build - There was an error moving build files to your selected build folder. Terminating build.");

        try {
            for (const auto& file : std::filesystem::directory_iterator(buildPath))
                if (file.path().filename().string() != "CMakeFilesBackup" && file.path().filename().string() != "CMakeFiles" && file.path().filename().string() != "cmake_install.cmake" && file.path().filename().string() != "CMakeCache.txt" && file.path().filename().string() != "CMakeLists.txt" && file.path().filename().string() != "Makefile")
                    std::filesystem::remove_all(file.path());
        }
        catch (const std::filesystem::filesystem_error& ex) {}

        callback(0, false);
        ImGuiPopup::SetActive(false);
        return false;
    }

    try {
        for (const auto& file : std::filesystem::directory_iterator(buildPath))
            if (file.path().filename().string() != "CMakeFilesBackup" && file.path().filename().string() != "CMakeFiles" && file.path().filename().string() != "cmake_install.cmake" && file.path().filename().string() != "CMakeCache.txt" && file.path().filename().string() != "CMakeLists.txt" && file.path().filename().string() != "Makefile")
                std::filesystem::remove_all(file.path());
    }
    catch (const std::filesystem::filesystem_error& ex) {}


    ConsoleLogger::InfoLog("Build - Build complete", false);

    Utilities::OpenPathInExplorer(outputPath);

    callback(1, FALSE);
    ImGuiPopup::SetActive(false);
    return true;
}

ProjectData ProjectManager::LoadProject(std::filesystem::path path)
{
    ProjectData projectData = LoadProjectData(path);
    //projectData.name = path.stem().string();
    projectData.path = path.string();
    return projectData;
}

void ProjectManager::SaveProjectData(ProjectData projectData) // Todo: Encode File
{
    json projectDataJson;

    projectDataJson["name"] = projectData.name;
    projectDataJson["description"] = projectData.description;
    projectDataJson["company"] = projectData.company;
    projectDataJson["version"] = projectData.version;
    projectDataJson["copyright"] = projectData.copyright;
    projectDataJson["is3D"] = projectData.is3D;

    projectDataJson["defaultScene"] = projectData.defaultScenePath;
    projectDataJson["iconPath"] = projectData.iconPath;

    projectDataJson["resizableWindow"] = projectData.resizableWindow;
    projectDataJson["displayMode"] = projectData.displayMode;
    projectDataJson["minimumResolution"] = nlohmann::json::array();
    projectDataJson["minimumResolution"].push_back(projectData.minimumResolution.x);
    projectDataJson["minimumResolution"].push_back(projectData.minimumResolution.y);
    projectDataJson["windowResolution"] = nlohmann::json::array();
    projectDataJson["windowResolution"].push_back(projectData.windowResolution.x);
    projectDataJson["windowResolution"].push_back(projectData.windowResolution.y);
    projectDataJson["maxFPS"] = projectData.maxFPS;
    projectDataJson["runInBackground"] = projectData.runInBackground;
    projectDataJson["vsync"] = projectData.vsync;

    projectDataJson["antialiasing"] = projectData.antialiasing;
    projectDataJson["highDPI"] = projectData.highDPI;

    projectDataJson["physicsTimeStep"] = nlohmann::json::array();
    projectDataJson["physicsTimeStep"].push_back(projectData.physicsTimeStep.x);
    projectDataJson["physicsTimeStep"].push_back(projectData.physicsTimeStep.y);
    projectDataJson["velocityIterations"] = projectData.velocityIterations;
    projectDataJson["positionIterations"] = projectData.positionIterations;
    
    std::filesystem::path path = (projectData.path / "ProjectSettings.cry");

    std::string formattedPath = path.string();
    std::size_t found = formattedPath.find('\\'); // Todo: Check if I need to format the path
    while (found != std::string::npos) {
        formattedPath.replace(found, 1, "\\\\");
        found = formattedPath.find('\\', found + 2);
    }
    formattedPath.erase(std::remove_if(formattedPath.begin(), formattedPath.end(), [](char c) {
        return !std::isprint(static_cast<unsigned char>(c));
        }), formattedPath.end());

    std::ofstream file(formattedPath);
    file << std::setw(4) << projectDataJson << std::endl;
    file.close();

    if (file.fail() && file.bad())
    {
        if (file.eof())
        {
            ConsoleLogger::ErrorLog("End of file reached while project data at the path '" + path.string() + "'");
        }
        else if (file.fail())
        {
            ConsoleLogger::ErrorLog("The project data file at the path \"" + path.string() + "\" failed to open. The file can't be found or you have invalid permissions.");
        }
        else if (file.bad())
        {
            char errorMessage[256];
#ifdef WINDOWS
            strerror_s(errorMessage, sizeof(errorMessage), errno);
#else
            strerror_r(errno, errorMessage, sizeof(errorMessage));
#endif

            ConsoleLogger::WarningLog("The project data failed to save. Error: " + std::string(errorMessage));
        }
        else if (file.is_open())
        {
            ConsoleLogger::ErrorLog("The project data failed to save because the save file is open in another program");
        }
        else
        {
            char errorMessage[256];
#ifdef WINDOWS
            strerror_s(errorMessage, sizeof(errorMessage), errno);
#else
            strerror_r(errno, errorMessage, sizeof(errorMessage));
#endif
            ConsoleLogger::WarningLog("The project data failed to save. Error: " + std::string(errorMessage));
        }
        return;
    }
    ConsoleLogger::InfoLog("The project data has been saved");
}

ProjectData ProjectManager::LoadProjectData(std::filesystem::path projectPath) // Todo: Decode File
{
    if (!std::filesystem::exists(projectPath / "ProjectSettings.cry"))
    {
        ConsoleLogger::WarningLog("The project's data is missing, creating a new one.");
        // Todo: Create new Project Data. Check stuff like where Models or Sprites folder exists, etc to determine whether its a 2D or 3D game.
        throw std::runtime_error("Project data is missing at the path: " + (projectPath / "ProjectSettings.cry").string());
    }

    std::string filePathString = (projectPath / "ProjectSettings.cry").string();
    filePathString.erase(std::remove_if(filePathString.begin(), filePathString.end(),
        [](char c) { return !std::isprint(c); }), filePathString.end());

    std::ifstream file(filePathString);
    if (!file.is_open())
    {
        ConsoleLogger::WarningLog("Can not open the project data file.");
        // Todo: Handle this
        throw std::runtime_error("Failed to open the project data at the path: " + (projectPath / "ProjectSettings.cry").string());
    }
    std::stringstream fileStream;
    fileStream << file.rdbuf();
    file.close();

    json projectDataJson = json::parse(fileStream.str());

    ProjectData projectData;
    bool saveProjectData = false;

    try {
        projectData.name = projectDataJson.at("name").get<std::string>();
    }
    catch (const std::exception& e) {
        projectData.name = "Unknown";
        saveProjectData = true;
    }

    try {
        projectData.is3D = projectDataJson.at("is3D").get<bool>();
    }
    catch (const std::exception& e) {
        projectData.is3D = false;
        saveProjectData = true;
    }

    try {
        projectData.description = projectDataJson.at("description").get<std::string>();
    }
    catch (const std::exception& e) {
        projectData.description = "";
        saveProjectData = true;
    }

    try {
        projectData.company = projectDataJson.at("company").get<std::string>();
    }
    catch (const std::exception& e) {
        projectData.company = "";
        saveProjectData = true;
    }

    try {
        projectData.version = projectDataJson.at("version").get<std::string>();
    }
    catch (const std::exception& e) {
        projectData.version = "1.0";
        saveProjectData = true;
    }

    try {
        projectData.copyright = projectDataJson.at("copyright").get<std::string>();
    }
    catch (const std::exception& e) {
        projectData.copyright = "Copyright {year}, {company}. All rights reserved.";
        saveProjectData = true;
    }

    try {
        projectData.defaultScenePath = projectDataJson.at("defaultScene").get<std::string>();
    }
    catch (const std::exception& e) {
        projectData.defaultScenePath = "Scenes/Default.scene";
        saveProjectData = true;
    }

    try {
        projectData.iconPath = projectDataJson.at("iconPath").get<std::string>();
    }
    catch (const std::exception& e) {
        projectData.iconPath = "None";
        saveProjectData = true;
    }

    try {
        projectData.resizableWindow = projectDataJson.at("resizableWindow").get<bool>();
    }
    catch (const std::exception& e) {
        projectData.resizableWindow = false;
        saveProjectData = true;
    }

    try {
        projectData.displayMode = projectDataJson.at("displayMode").get<int>();
    }
    catch (const std::exception& e) {
        projectData.displayMode = 0;
        saveProjectData = true;
    }

    try {
        projectData.minimumResolution.x = projectDataJson.at("minimumResolution")[0].get<float>();
        projectData.minimumResolution.y = projectDataJson.at("minimumResolution")[1].get<float>();
    }
    catch (const std::exception& e) {
        projectData.minimumResolution = { 100, 100 };
        saveProjectData = true;
    }

    try {
        projectData.windowResolution.x = projectDataJson.at("windowResolution")[0].get<float>();
        projectData.windowResolution.y = projectDataJson.at("windowResolution")[1].get<float>();
    }
    catch (const std::exception& e) {
        projectData.windowResolution = { 1920, 1080 };
        saveProjectData = true;
    }

    try {
        projectData.maxFPS = projectDataJson.at("maxFPS").get<float>();
    }
    catch (const std::exception& e) {
        projectData.maxFPS = 60;
        saveProjectData = true;
    }

    try {
        projectData.runInBackground = projectDataJson.at("runInBackground").get<bool>();
    }
    catch (const std::exception& e) {
        projectData.runInBackground = false;
        saveProjectData = true;
    }

    try {
        projectData.vsync = projectDataJson.at("vsync").get<bool>();
    }
    catch (const std::exception& e) {
        projectData.vsync = false;
        saveProjectData = true;
    }

    try {
        projectData.antialiasing = projectDataJson.at("antialiasing").get<bool>();
    }
    catch (const std::exception& e) {
        projectData.antialiasing = false;
        saveProjectData = true;
    }

    try {
        projectData.highDPI = projectDataJson.at("highDPI").get<bool>();
    }
    catch (const std::exception& e) {
        projectData.highDPI = true;
        saveProjectData = true;
    }

    try {
        projectData.physicsTimeStep.x = projectDataJson.at("physicsTimeStep")[0].get<float>();
        projectData.physicsTimeStep.y = projectDataJson.at("physicsTimeStep")[1].get<float>();
    }
    catch (const std::exception& e) {
        projectData.physicsTimeStep = { 1, 60 };
        saveProjectData = true;
    }

    try {
        projectData.velocityIterations = projectDataJson.at("velocityIterations").get<int>();
    }
    catch (const std::exception& e) {
        projectData.velocityIterations = 8;
        saveProjectData = true;
    }

    try {
        projectData.positionIterations = projectDataJson.at("positionIterations").get<int>();
    }
    catch (const std::exception& e) {
        projectData.positionIterations = 3;
        saveProjectData = true;
    }

    if (saveProjectData)
    {
        projectData.path = projectPath;
        SaveProjectData(projectData);
    }

    ConsoleLogger::InfoLog("Project data has been loaded");

    return projectData;
}

void ProjectManager::GenerateExposedVariablesFunctions(std::filesystem::path path) // Todo: Move to ScriptingTools directory
{
    std::unordered_map<std::filesystem::path, std::unordered_map<int, nlohmann::json>> components;

    // Todo: Put components in relative paths to the Assets/, this will fix the issues with determining whether a component is internal or external
    // Todo: If a component in one scene as the same ID as a component in another scene, it will cause issues.
    for (Scene& scene : *SceneManager::GetScenes()) // Todo: Only iterate the scenes that are set to be built
    {
        for (GameObject* gameObject : scene.GetGameObjects())
        {
            for (Component* component : gameObject->GetComponents())
            {
                ScriptComponent* scriptComponent = dynamic_cast<ScriptComponent*>(component);
                if (scriptComponent)
                {
                    if (scriptComponent->exposedVariables.is_null())
                        continue;
                    components[scriptComponent->GetHeaderPath()][scriptComponent->id] = scriptComponent->exposedVariables;
                }
                else
                {
                    Component* _component = dynamic_cast<Component*>(component);
                    if (_component->exposedVariables.is_null())
                        continue;
                    components["Components/" + _component->name + ".h"][_component->id] = _component->exposedVariables;
                }
            }
        }
    }

    for (const auto& component : components)
    {
        if (!std::filesystem::exists(path / component.first.filename()) && !std::filesystem::exists(path / "Components" / component.first.filename())) // Todo: Should probably store whether the component is internal or not instead of doing this
        {
            // Todo: Do something if the file does not exist
            ConsoleLogger::ErrorLog("Failed to find \"" + (path / component.first.filename()).string() + "\".Error Code 700"); // Todo: Remove this warning, or maybe keep it for developer mode
            continue;
        }
        // Todo: This assume the .cpp is in the same location as the header. Consider getting the .cpp path from the Component and store it in the components map
        if (!std::filesystem::exists(path / (component.first.stem().string() + ".cpp")) && !std::filesystem::exists(path / "Components" / (component.first.stem().string() + ".cpp")))
        {
            // Todo: Do something if the file does not exist
            ConsoleLogger::ErrorLog("Failed to find \"" + (path / (component.first.stem().string() + ".cpp")).string() + "\". Error code 700"); // Todo: Remove this warning, or maybe keep it for developer mode
            continue;
        }

        // Todo: This will not work if a function has "{" before class declaration, such as in variables or in a comment

        bool internal = false;
        // Create function in header
        std::ifstream headerFile(path / component.first.filename());
        if (!headerFile.is_open())
        {
            // Todo: This is a horrible solution for opening internal components
            headerFile.open(path / "Components" / component.first.filename());
            if (!headerFile.is_open())
            {
                // Todo: Do something if file failed to open
                continue;
            }
            internal = true;
        }

        
        std::string line;
        //bool foundPublic = false;
        //while (std::getline(headerFile, line))
        //{
        //    if (line.find("public:") != std::string::npos)
        //    {
        //        foundPublic = true;
        //        break;
        //    }
        //}

        headerFile.clear();
        headerFile.seekg(0, std::ios::beg);

        line = "";
        bool placedFunction = false;
        //std::ofstream tempHeader;
        //if (internal)
        //    tempHeader.open(path / "Components" / (component.first.stem().string() + ".temp"));
        //else
        //    tempHeader.open(path / (component.first.stem().string() + ".temp"));

        // Reads the header file and puts all of the lines into a vector
        std::vector<std::string> lines;
        while (std::getline(headerFile, line))
        {
            // This code places it at the top of the class which caused scope issues when there were custom exposed variables like enums or other custom clases
            //if (foundPublic)
            //{
            //    if (!placedFunction && line.find("public:") != std::string::npos) // Todo: Should probably remove whitespaces incase someone wrote it like this, "public :". Same with abvoe when looking for public
            //    {
            //        tempHeader << line << std::endl;
            //        tempHeader << "void SetExposedVariables() override;\n";
            //        placedFunction = true;
            //    }
            //    else
            //        tempHeader << line << std::endl;
            //}
            //else // Todo: If the file does not include the public specifier, it is assumed to be private. This means none of the functions can be called on it and therefore the component does nothing. Should it be removed? If this ire moved, then theres no point to check if public exists before the actual loop.
            //{
            //    if (!placedFunction && line.find("{") != std::string::npos)
            //    {
            //        tempHeader << line << std::endl;
            //        tempHeader << "public:\nvoid SetExposedVariables() override;\nprivate:\n";
            //        placedFunction = true;
            //    }
            //    else
            //        tempHeader << line << std::endl;
            //}

            lines.push_back(line);
        }

        headerFile.close();

        std::ofstream tempHeader;
        // Todo: Find a better way to check if this is internal, like using full paths instead of file names. Same with tempCpp below
        if (internal)
            tempHeader.open(path / "Components" / (component.first.stem().string() + ".temp"));
        else
            tempHeader.open(path / (component.first.stem().string() + ".temp"));

        // Iterates the lines in reverse to find "};" and add SetExposedVariables() right before it
        for (auto it = lines.rbegin(); it != lines.rend(); ++it)
        {
            if (size_t pos = it->find("};"); pos != std::string::npos)
            {
                it->replace(pos, 2, "public:\nvoid SetExposedVariables() override;\n};");
                placedFunction = true;
                break;
            }
        }

        // Writes the lines from the vector into the tempHeader file
        for (const auto& l : lines)
            tempHeader << l << std::endl;

        tempHeader.close();

        // Todo: Find a better way to check if this is internal, like using full paths instead of file names
        if (internal)
        {
            std::filesystem::remove(path / "Components" / component.first.filename());
            std::filesystem::rename(path / "Components" / (component.first.stem().string() + ".temp"), path / "Components" / component.first.filename());
        }
        else
        {
            std::filesystem::remove(path / component.first.filename());
            std::filesystem::rename(path / (component.first.stem().string() + ".temp"), path / component.first.filename());
        }

        if (!placedFunction)
        {
            // Todo: Failed to find a place to put the SetExposedVariables() function
            ConsoleLogger::ErrorLog("Failed to find a place to put SetExposedVariables() in " + (path / component.first.filename()).string()); // Todo: Maek this warning for dev mode, and make a better message like failed to set exposed variables for x file
            continue;
        }

        // Create function in CPP
        std::ifstream cppFile(path / (component.first.stem().string() + ".cpp"));
        if (!cppFile.is_open())
        {
            // Todo: This is a horrible solution for opening internal components
            cppFile.open(path / "Components" / (component.first.stem().string() + ".cpp"));
            if (!cppFile.is_open())
            {
                // Todo: Do something if file failed to open
                continue;
            }
        }

        line = "";
        placedFunction = false;
        bool lastLineWasInclude = false;
        //std::ofstream tempCpp(path / (component.first.stem().string() + ".temp"));
        std::ofstream tempCpp;
        if (internal)
            tempCpp.open(path / "Components" / (component.first.stem().string() + ".temp"));
        else
            tempCpp.open(path / (component.first.stem().string() + ".temp"));
        while (std::getline(cppFile, line))
        {
            if (!placedFunction)
            {
                if (lastLineWasInclude && line.find("#include") == std::string::npos)
                {
                    tempCpp << ("void " + component.first.stem().string() + "::SetExposedVariables() {\n");

                    tempCpp << "switch (id) {\n";

                    for (const auto& data : component.second)
                    {
                        tempCpp << "case " + std::to_string(data.first) + std::string(":\n");
                        //ConsoleLogger::ErrorLog("Exposed Variables Json: " + data.second.dump(4));
                        for (auto variables = data.second[1].begin(); variables != data.second[1].end(); ++variables)
                        {
                            if ((*variables)[0] == "float")
                                tempCpp << (*variables)[1].get<std::string>() + " = " + (*variables)[2].dump() + "f;\n";
                            else if ((*variables)[0] == "Color")
                                tempCpp << (*variables)[1].get<std::string>() + " = {" + std::to_string((*variables)[2][0].get<int>()) + "," + std::to_string((*variables)[2][1].get<int>()) + "," + std::to_string((*variables)[2][2].get<int>()) + "," + std::to_string((*variables)[2][3].get<int>()) + "};\n";
                            else if ((*variables)[0] == "Vector2")
                                tempCpp << (*variables)[1].get<std::string>() + " = {" + std::to_string((*variables)[2][0].get<float>()) + "f," + std::to_string((*variables)[2][1].get<float>()) + "f};\n";
                            else if ((*variables)[0] == "Vector3")
                                tempCpp << (*variables)[1].get<std::string>() + " = {" + std::to_string((*variables)[2][0].get<float>()) + "f," + std::to_string((*variables)[2][1].get<float>()) + "f," + std::to_string((*variables)[2][2].get<float>()) + "f};\n";
                            else if ((*variables).size() > 4 && (*variables)[4].contains("Extensions"))
                            {
                                if ((*variables)[0].get<std::string>() == "string")
                                {
									if ((*variables)[2].get<std::string>() == "nullptr" || (*variables)[2].get<std::string>() == "Default")
										tempCpp << (*variables)[1].get<std::string>() + " = \"\";\n";
                                    else
                                        tempCpp << (*variables)[1].get<std::string>() + " = " + (*variables)[2].dump() + "; \n";
                                }
                                else if ((*variables)[2].get<std::string>() == "nullptr" || (*variables)[2].get<std::string>() == "Default")
                                    tempCpp << (*variables)[1].get<std::string>() + " = nullptr;\n";
                                else
                                    tempCpp << (*variables)[1].get<std::string>() + " = new " + (*variables)[0].get<std::string>() + "(" + (*variables)[2].dump() + ");\n";
                            }
                            else if ((*variables).size() > 4) // Todo: This is a horrible solution for checking if its an enum
                            {
                                std::string value = (*variables)[2].dump();
                                value = value.substr(1);
                                value = value.substr(0, value.size() - 1);
                                tempCpp << (*variables)[1].get<std::string>() + " = " + value + ";\n";
                            }
                            else
                                tempCpp << (*variables)[1].get<std::string>() + " = " + (*variables)[2].dump() + ";\n";
                        }
                        tempCpp << "break;\n";
                    }

                    tempCpp << "}\n}\n";
                    placedFunction = true;
                }

                tempCpp << line << std::endl;

                if (!placedFunction && line.find("#include") != std::string::npos)
                    lastLineWasInclude = true;
                else
                    lastLineWasInclude = false;
            }
            else
                tempCpp << line << std::endl;
        }

        cppFile.close();
        tempCpp.close();

        if (internal)
        {
            std::filesystem::remove(path / "Components" / (component.first.stem().string() + ".cpp"));
            std::filesystem::rename(path / "Components" / (component.first.stem().string() + ".temp"), path / "Components" / (component.first.stem().string() + ".cpp"));
        }
        else
        {
            std::filesystem::remove(path / (component.first.stem().string() + ".cpp"));
            std::filesystem::rename(path / (component.first.stem().string() + ".temp"), path / (component.first.stem().string() + ".cpp"));
        }

        if (!placedFunction)
        {
            // Todo: Failed to find a place to put the SetExposedVariables() function
            ConsoleLogger::ErrorLog("Failed to find a place to put SetExposedVariables() in " + (path / (component.first.stem().string() + ".cpp")).string()); // Todo: Remove this warning
            continue;
        }
        // Iterate component.first[1]
    }

    //for (const auto& file : std::filesystem::recursive_directory_iterator(path))
    //{
    //    if (!std::filesystem::is_directory(file))
    //    {
    //        if (file.path().extension() == ".h")
    //        {

    //        }
    //        else if (file.path().extension() == ".cpp")
    //        {

    //        }
    //    }
    //}
}