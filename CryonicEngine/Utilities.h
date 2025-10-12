#pragma once

#include <filesystem>
#include <json.hpp>
#include <vector>

namespace Utilities
{
    using JobHandle = void*;

    void OpenPathInExplorer(std::filesystem::path path);
    std::string SelectFolderDialog(const std::filesystem::path& projectPath);
    void HideFile(std::filesystem::path path);
    int GetNumberOfCores();
    bool HasInternetConnection();
    bool IsProgramInstalled(const char* program);
    std::filesystem::path CreateUniqueFile(std::filesystem::path path, std::string name, std::string extension);
    nlohmann::json GetExposedVariables(std::filesystem::path path);
    std::vector<std::string> GetGltfAnimationNames(std::filesystem::path path);
    bool CreateDataFile(std::filesystem::path path);
    bool ImportFile(std::filesystem::path filePath, std::filesystem::path importPath);
    // Todo: This is used for running the compiled game in debug mode. This should be moved eventually.
    std::pair<std::string, JobHandle> LaunchProcess(std::string startCommand);
    void TerminateProcess(int dwProcessId, int uExitCode); // A TerminateProcess function that actually works unlike Window's TermianteProcess()
    void TerminateJob(JobHandle jobHandle);
    std::filesystem::path CreateTempFolder(std::filesystem::path projectPath);
    std::filesystem::path GetExePath();
    uint32_t HashString(const std::string& str);
};