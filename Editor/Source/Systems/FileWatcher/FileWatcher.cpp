#include "FileWatcher.h"
#include "Core/ProjectManager.h"
#include "Core/MainThreadQueue.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <filesystem>
#include "Resources/Sprite.h"
#include <shared_mutex>
#include <future>

namespace FileWatcher
{
    static std::unordered_map<std::string, std::filesystem::path> deletedFiles; // file name, old path
    static std::shared_mutex deletedFilesMutex; // Needed for so deletedFiles is thread safe and wont crash from race conditions
    static std::mutex texturesMutex;

    static std::unordered_map<std::string, std::function<void()>> modifyCallbacks;
    static std::unordered_map<std::string, std::function<void(const std::string oldPath, const std::string newPath)>> moveCallbacks;
    static std::unordered_map<std::string, std::function<void()>> deleteCallbacks;

    static std::unordered_map<std::string, std::function<void(const std::filesystem::path&)>> globalModifyCallbacks;
    static std::unordered_map<std::string, std::function<void(const std::filesystem::path&, const std::filesystem::path&)>> globalMoveCallbacks;
    static std::unordered_map<std::string, std::function<void(const std::filesystem::path&)>> globalDeleteCallbacks;

    void Init()
    {
        efsw::FileWatcher* fileWatcher = new efsw::FileWatcher();
        UpdateListener* listener = new UpdateListener();
        efsw::WatchID watchID3 = fileWatcher->addWatch(ProjectManager::projectData.path.string() + "\\Assets", listener, true, {{efsw::Option::WinBufferSize, 1024 * 1024}});
        fileWatcher->watch();
    }

    void UpdateListener::handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename)
    {
        // Add and Delete is used for file moving.
        switch (action) {
        case efsw::Actions::Add:
        {
            std::unique_lock<std::shared_mutex> lock(deletedFilesMutex);
            auto it = deletedFiles.find(filename);
            if (it != deletedFiles.end())
            {
                FileMoved(it->second, dir + "\\" + filename);
                deletedFiles.erase(it);
            }
            break;
        }
        case efsw::Actions::Delete:
        {
            std::unique_lock<std::shared_mutex> writeLock(deletedFilesMutex);
            deletedFiles[filename] = std::filesystem::path(dir + "\\" + filename);
            //std::thread(CheckIfFileMoved, std::filesystem::path(dir + "\\" + filename)).detach();
            std::async(std::launch::async, CheckIfFileMoved, std::filesystem::path(dir + "\\" + filename)); // The commented code above could result in a race condition, this should fix it
            break;
        }
        case efsw::Actions::Modified: // Gets called when a file is modifiued, or something gets changed in a folder
            FileModified(dir + "\\" + filename);
            break;
        case efsw::Actions::Moved: // Gets called when something is renamed
            FileMoved(oldFilename, filename);
            break;
        default:
            break;
        }
    }

    void CheckIfFileMoved(std::filesystem::path path) //  This doesn't work well
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::unique_lock<std::shared_mutex> lock(deletedFilesMutex);
        auto it = deletedFiles.find(path.filename().string());
        if (it != deletedFiles.end())
        {
            // File was not added, and therefore assumed to not have been moved. Handle deleted files here
            deletedFiles.erase(it);
            std::cout << path.string() + " Has been deleted" << std::endl;

            // Callbacks
            std::string relativePath = std::filesystem::relative(path, ProjectManager::projectData.path / "Assets").string();
            if (deleteCallbacks.count(relativePath))
            {
                deleteCallbacks[relativePath]();
                deleteCallbacks.erase(relativePath);
            }

            for (auto& [id, callback] : globalDeleteCallbacks)
                callback(path);
        }
    }

    void FileMoved(std::filesystem::path oldPath, std::filesystem::path newPath)
    {
        // Update paths of everything using the old path
        std::cout << oldPath.string() + " Has been moved to " + newPath.string() << std::endl;

        // When moving a folder, it only runs Add and Delete on folder, not files within

        if (std::filesystem::exists(oldPath.string() + ".data"))
            std::filesystem::rename(oldPath.string() + ".data", (newPath.parent_path() / (newPath.stem().string() + newPath.extension().string() + ".data")));

        // Callbacks
        std::string relativeOldPath = std::filesystem::relative(oldPath, ProjectManager::projectData.path / "Assets").string();
        if (moveCallbacks.count(relativeOldPath))
        {
            moveCallbacks[relativeOldPath](oldPath.string(), newPath.string());

            // Change the callback path
            moveCallbacks[std::filesystem::relative(newPath, ProjectManager::projectData.path / "Assets").string()] = moveCallbacks[oldPath.string()];
            moveCallbacks.erase(relativeOldPath);
        }

        for (auto& [id, callback] : globalMoveCallbacks)
            callback(oldPath, newPath);
    }

    void FileModified(std::filesystem::path filePath) // Todo: Since we have sleeps, after the sleep, we should check if there are any newer modifications. Applications like Gimp when saving/overwriting, it modifies the file 3 times. Having this would prevent it from being loaded and unloaded 3 times.
    {
        if (std::filesystem::is_directory(filePath) || filePath.extension() == ".data") // file types like .animgraph shouldn't be added here?
            return;

        // Todo: Add meshes
        // Todo: Add fonts

        if (filePath.extension() == ".png" || filePath.extension() == ".jpg" || filePath.extension() == ".jpeg") // Update sprites
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Its possible that the file is still being written and unable to be read. I assume this is because programs like gimp modify the files 3 times, or it just takes a bit to be readable.

            std::string relativePath = std::filesystem::relative(filePath, ProjectManager::projectData.path / "Assets").string();

            std::unique_lock<std::mutex> lock(texturesMutex);
            auto it = Sprite::textures.find(relativePath);
            if (it == Sprite::textures.end())
                return;

            MainThreadQueue::Add([it]() {
                RaylibWrapper::UnloadTexture(*it->second.first); // This doesn't seem to be actually unloading the texture since I can still use it?
                });
            //delete it->second.first;
            //it->second.first = new RaylibWrapper::Texture2D(RaylibWrapper::LoadTexture(filePath.string().c_str()));
        }

        // Callbacks
        std::string relativePath = std::filesystem::relative(filePath, ProjectManager::projectData.path / "Assets").string();
        if (modifyCallbacks.count(relativePath))
            modifyCallbacks[relativePath]();

        for (auto& [id, callback] : globalModifyCallbacks)
            callback(filePath);
    }

    // Specific file Callbacks
    void AddFileModifyCallback(std::string relativePath, std::function<void()> callback)
    {
        modifyCallbacks[relativePath] = callback;
    }

    void RemoveFileModifyCallback(std::string relativePath)
    {
        modifyCallbacks.erase(relativePath);
    }

    void AddFileMoveCallback(std::string relativePath, std::function<void(const std::string oldPath, const std::string newPath)> callback)
    {
        moveCallbacks[relativePath] = callback;
    }

    void RemoveFileMoveCallback(std::string relativePath)
    {
        moveCallbacks.erase(relativePath);
    }

    void AddFileDeletedCallback(std::string relativePath, std::function<void()> callback)
    {
        deleteCallbacks[relativePath] = callback;
    }

    void RemoveFileDeletedCallback(std::string relativePath)
    {
        deleteCallbacks.erase(relativePath);
    }

    // Global 
    void AddGlobalModifyCallback(std::string id, std::function<void(const std::filesystem::path path)> callback)
    {
        globalModifyCallbacks[id] = callback;
    }

    void RemoveGlobalModifyCallback(std::string relativePath)
    {
        globalModifyCallbacks.erase(relativePath);
    }

    void AddGlobalMoveCallback(std::string id, std::function<void(std::filesystem::path oldPath, const std::filesystem::path newPath)> callback)
    {
        globalMoveCallbacks[id] = callback;
    }

    void RemoveGlobalMoveCallback(std::string id)
    {
        globalMoveCallbacks.erase(id);
    }

    void AddGlobalDeletedCallback(std::string id, std::function<void(const std::filesystem::path path)> callback)
    {
        globalDeleteCallbacks[id] = callback;
    }

    void RemoveGlobalDeletedCallback(std::string id)
    {
        globalDeleteCallbacks.erase(id);
    }
};