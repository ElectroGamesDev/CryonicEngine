#pragma once

#include "ThirdParty/efsw/include/efsw.hpp"
#include <filesystem>
#include <functional>

namespace FileWatcher
{
    class UpdateListener : public efsw::FileWatchListener
    {
        void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename) override;
    };

    void Init();
    void FileMoved(std::filesystem::path oldPath, std::filesystem::path newPath);
    void FileModified(std::filesystem::path filePath);
    void CheckIfFileMoved(std::filesystem::path path);

    void AddFileModifyCallback(std::string relativePath, std::function<void()> callback);
    void RemoveFileModifyCallback(std::string relativePath);

    void AddFileMoveCallback(std::string relativePath, std::function<void(const std::string oldPath, const std::string newPath)>  callback);
    void RemoveFileMoveCallback(std::string relativePath);

    void AddFileDeletedCallback(std::string relativePath, std::function<void()> callback);
    void RemoveFileDeletedCallback(std::string relativePath);

    void AddGlobalModifyCallback(std::string id, std::function<void(const std::filesystem::path path)> callback);
    void RemoveGlobalModifyCallback(std::string id);

    void AddGlobalMoveCallback(std::string id, std::function<void(const std::filesystem::path oldPath, const std::filesystem::path newPath)> callback);
    void RemoveGlobalMoveCallback(std::string id);

    void AddGlobalDeletedCallback(std::string id, std::function<void(const std::filesystem::path path)> callback);
    void RemoveGlobalDeletedCallback(std::string id);
};