#include "AssetManager.h"
#include "ThirdParty/imgui/IconsFontAwesome6.h"
#include <string>
#include "Utilities/ConsoleLogger.h"
#include "ThirdParty/curl/include/curl.h"
#include "ThirdParty/Misc/json.hpp"
#include <fstream>
#include <vector>
#include "Raylib/RaylibWrapper.h"
#include <filesystem>
#include "Core/ProjectManager.h"
#include "ThirdParty/imgui/ImGuiNotify.hpp"

namespace AssetManager
{
    bool open = false;
    ImGuiWindowClass* windowClass;
    
    struct AssetData
    {
        std::string name;
        std::string description;
        std::string author;
        std::string category;
        std::string url;
        RaylibWrapper::Texture2D icon;
    };
    std::vector<AssetData> cachedAssets;
    nlohmann::json installedAssets;
    bool assetsCached = false;
    bool connectedToInternet = true;

    size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data)
    {
        data->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    size_t WriteImageDataToMemory(void* contents, size_t size, size_t nmemb, std::vector<unsigned char>* buffer) {
        size_t totalSize = size * nmemb;
        buffer->insert(buffer->end(), (unsigned char*)contents, (unsigned char*)contents + totalSize);
        return totalSize;
    }

    void RenderWindow()
    {
        if (!open)
        {
            if (assetsCached)
                Cleanup();

            return;
        }

        // Todo: Add pages

        if (!assetsCached && connectedToInternet) // Using connectedToInternet since it prevents the program from being stuck constantly checking if it has an internet connection
        {
            if (Utilities::HasInternetConnection())
                FetchAssets();
            else
                connectedToInternet = false;
        }

        ImGui::SetNextWindowClass(windowClass);
        if (ImGui::Begin((ICON_FA_BOX_ARCHIVE + std::string(" Asset Manager")).c_str(), &open, ImGuiWindowFlags_NoCollapse))
        {
            ImVec2 cursorPos = { 20, 40 };
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            for (AssetData& assetData : cachedAssets)
            {
                ImGui::SetCursorPos(cursorPos);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8f, 0.8f, 0.8f, 0.8f));
                // Todo: This is probably not best, but i'm not sure what else I could use
                ImGui::BeginChild(("##" + assetData.url).c_str(), { 150, 150 }, ImGuiChildFlags_Border, ImGuiWindowFlags_NoScrollbar);

                ImGui::SetCursorPos({ 32, 5 });
                RaylibWrapper::rlImGuiImageSizeV(&assetData.icon, { 85, 85 });

                ImGui::SetCursorPos({ (ImGui::GetWindowWidth() - ImGui::CalcTextSize(assetData.name.c_str()).x) * 0.5f, 95});
                ImGui::Text(assetData.name.c_str());

                ImGui::SetCursorPos({ 37, 120 });

                // Todo: Add Update button
                // Todo: Display both Update and Uninstall button if there is an update

                bool installed = false;
                for (auto& asset : installedAssets)
                {
                    if (asset["url"] == assetData.url)
                    {
                        installed = true;
                        break;
                    }
                }

                if (!installed)
                {
                    if (ImGui::Button(("Install##" + assetData.url).c_str(), { 75, 25 }))
                        InstallAsset(assetData.url, assetData.name);
                }
                else if (ImGui::Button(("Uninstall##" + assetData.url).c_str(), { 75, 25 }))
                    UninstallAsset(assetData.url);

                ImGui::EndChild();
                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);

                cursorPos = { ImGui::GetCursorPosX() + 180, cursorPos.y };
                if (cursorPos.x > ImGui::GetWindowWidth() - 180) // Todo: Will this cause an infinite loop if the window is really small?
                    cursorPos = { 5, ImGui::GetCursorPosY() + 180 };
            }
        }
        ImGui::End();
    }

    void CheckForUpdates()
    {
        if (!Utilities::HasInternetConnection())
            return;
    }

    bool InstallAsset(std::string url, std::string name) // TODO:  name variable is different than github repo such as name uses space while repo doesn't which causes issues. Maybe create in a temp folder, then rename, then move
    {
        // If delete asset within filesystem, check if its a third party asset from manager
        // When starting up editor, check if any third party assets have been removed

        if (!Utilities::HasInternetConnection())
            return false;

        // Todo: Check version

        // Check if Git is installed
        if (!Utilities::IsProgramInstalled("git --version"))
        {
            ConsoleLogger::ErrorLog("Package Manager - Failed to download asset due to Git not being installed. Try again after installing it.");
            ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
            if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                window->DockNode->TabBar->NextSelectedTabId = window->TabId;

            ImGuiToast toast(ImGuiToastType::Error, 2500, true);
            toast.setTitle("Package Manager", "");
            toast.setContent("Failed to download asset, please install Git first.");
            ImGui::InsertNotification(toast);

            return false;
        }

        std::filesystem::path tempPath = Utilities::CreateTempFolder(ProjectManager::projectData.path);

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        std::string command = "cmd.exe /C \"cd /d \"" + std::filesystem::path(tempPath).string() + "\" && git clone " + url + "\"";

        if (!CreateProcessA(NULL, const_cast<LPSTR>(command.c_str()), NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi))
        {
            ConsoleLogger::ErrorLog("Package Manager - Failed to install asset. Error code 9000");

            ImGuiToast toast(ImGuiToastType::Error, 2500, true);
            toast.setTitle("Package Manager", "");
            toast.setContent("Failed to install asset.");
            ImGui::InsertNotification(toast);
            return false;
        }

        while (true)
        {
            DWORD waitResult = WaitForSingleObject(pi.hProcess, 100);
            if (waitResult == WAIT_OBJECT_0)
                break;
            else if (waitResult != WAIT_TIMEOUT)
            {
                ConsoleLogger::ErrorLog("Package Manager - Failed to install asset. Error code 9001");

                ImGuiToast toast(ImGuiToastType::Error, 2500, true);
                toast.setTitle("Package Manager", "");
                toast.setContent("Failed to install asset.");
                ImGui::InsertNotification(toast);
                return false;
            }
        }

        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (exitCode != 0)
        {
            std::filesystem::remove_all(tempPath);
            ConsoleLogger::ErrorLog("Package Manager - Failed to install asset. Error code 9002");

            ImGuiToast toast(ImGuiToastType::Error, 2500, true);
            toast.setTitle("Package Manager", "");
            toast.setContent("Failed to install asset.");
            ImGui::InsertNotification(toast);
            return false;
        }

        // Install dependancies
        auto tempAssetPath = std::filesystem::directory_iterator(tempPath);
        if (tempAssetPath == std::filesystem::end(tempAssetPath))
        {
            std::filesystem::remove_all(tempPath);
            ConsoleLogger::ErrorLog("Package Manager - Failed to install asset. Error code 90013");

            ImGuiToast toast(ImGuiToastType::Error, 2500, true);
            toast.setTitle("Package Manager", "");
            toast.setContent("Failed to install asset.");
            ImGui::InsertNotification(toast);
            return false;
        }

        std::ifstream assetFile(tempAssetPath->path() / "manifest.json");
        if (!assetFile.is_open())
        {
            std::filesystem::remove_all(tempPath);
            ConsoleLogger::ErrorLog("Package Manager - Failed to install asset. Error code 9003");
            ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
            if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                window->DockNode->TabBar->NextSelectedTabId = window->TabId;

            ImGuiToast toast(ImGuiToastType::Error, 2500, true);
            toast.setTitle("Package Manager", "");
            toast.setContent("Failed to install asset.");
            ImGui::InsertNotification(toast);
            return false;
        }

        nlohmann::json manifestJson;
        manifestJson = nlohmann::json::parse(assetFile);
        assetFile.close();

        if (manifestJson.contains("dependencies"))
        {
            // Todo: Popup saying the addon requires dependencies to be installed.
            for (auto& dependency : manifestJson["dependencies"])
                InstallAsset(dependency["name"].get<std::string>(), dependency["url"].get<std::string>());
        }

        // Move the asset to the assets/Third Party
        std::filesystem::path thirdPartyPath = ProjectManager::projectData.path / "Assets" / "Third Party";

        if (!std::filesystem::exists(thirdPartyPath))
            std::filesystem::create_directory(thirdPartyPath);

        if (std::filesystem::exists(thirdPartyPath / name))
            std::filesystem::remove_all(thirdPartyPath / name);

        // Rename the asset folder
        std::filesystem::rename(*tempAssetPath, thirdPartyPath / name);
        std::filesystem::remove_all(tempPath);

        // Add to the assets.json
        if (!std::filesystem::exists(ProjectManager::projectData.path / "assets.json"))
        {
            std::ofstream file(ProjectManager::projectData.path / "assets.json");
            if (file.is_open())
            {
                file << nlohmann::json::array().dump(4);
                file.close();
            }
        }

        std::ifstream file(ProjectManager::projectData.path / "assets.json");
        if (!file.is_open())
        {
            ConsoleLogger::ErrorLog("Package Manager - Failed to install asset. Error code 9005");
            ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
            if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                window->DockNode->TabBar->NextSelectedTabId = window->TabId;

            ImGuiToast toast(ImGuiToastType::Error, 2500, true);
            toast.setTitle("Package Manager", "");
            toast.setContent("Failed to install asset.");
            ImGui::InsertNotification(toast);
            return false;
        }

        try {
            installedAssets = nlohmann::json::parse(file);
            file.close();

            // If the asset is already in the assets.json, remove it
            for (auto it = installedAssets.begin(); it != installedAssets.end(); )
            {
                if (it->contains("url") && it->at("url").get<std::string>() == url)
                {
                    it = installedAssets.erase(it);
                    break;
                }
                else 
                    ++it;
            }

            manifestJson["path"] = thirdPartyPath / name;
            installedAssets.push_back(manifestJson);

            std::ofstream outFile(ProjectManager::projectData.path / "assets.json");
            if (!outFile.is_open())
            {
                ConsoleLogger::ErrorLog("Package Manager - Failed to install asset. Error code 9006");
                ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
                if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                    window->DockNode->TabBar->NextSelectedTabId = window->TabId;

                ImGuiToast toast(ImGuiToastType::Error, 2500, true);
                toast.setTitle("Package Manager", "");
                toast.setContent("Failed to install asset.");
                ImGui::InsertNotification(toast);

                return false;
            }

            outFile << installedAssets.dump(4);
            outFile.close();
        }
        catch (const std::exception& e)
        {
            // Todo: Handle this
            //ConsoleLogger::ErrorLog("Error parsing JSON: " + std::string(e.what()));

            ConsoleLogger::ErrorLog("Package Manager - Failed to install asset. Error code 9007");
            ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
            if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                window->DockNode->TabBar->NextSelectedTabId = window->TabId;

            ImGuiToast toast(ImGuiToastType::Error, 2500, true);
            toast.setTitle("Package Manager", "");
            toast.setContent("Failed to update asset.");
            ImGui::InsertNotification(toast);

            return false;
        }

        return true;
    }

    bool UpdateAsset(std::string name, std::string url)
    {
        // Todo: Dont assume path
        // Todo: Add support for new/missing dependancies

        if (!Utilities::HasInternetConnection())
            return false;

        // Todo: Check version

        // Check if Git is installed
        if (!Utilities::IsProgramInstalled("git --version"))
        {
            ConsoleLogger::ErrorLog("Package Manager - Failed to update asset due to Git not being installed. Try again after installing it.");
            ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
            if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                window->DockNode->TabBar->NextSelectedTabId = window->TabId;

            ImGuiToast toast(ImGuiToastType::Error, 2500, true);
            toast.setTitle("Package Manager", "");
            toast.setContent("Failed to update asset, please install Git first.");
            ImGui::InsertNotification(toast);

            return false;
        }

        std::filesystem::path path = ProjectManager::projectData.path / "Assets" / "Third Party" / name;

        if (!std::filesystem::exists(path))
            std::filesystem::create_directory(path);

        // If its a git repository, then try to pull it
        if (std::filesystem::exists(path / ".git"))
        {
            STARTUPINFOA si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));
            std::string command = "cmd.exe /C \"cd /d \"" + std::filesystem::path(ProjectManager::projectData.path / "Assets" / "Third Party").string() + "\" && git pull\"";

            if (!CreateProcessA(NULL, const_cast<LPSTR>(command.c_str()), NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi))
                return InstallAsset(name, url);

            while (true)
            {
                DWORD waitResult = WaitForSingleObject(pi.hProcess, 100);
                if (waitResult == WAIT_OBJECT_0)
                    break;
                else if (waitResult != WAIT_TIMEOUT)
                    return InstallAsset(name, url);
            }

            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            if (exitCode != 0)
                return InstallAsset(name, url);
        }
        else // If the asset directory is not a repository, then reinstall
        {
            // Todo: Download to a temp folder, then move it to the correct directory instead of deleting the old files, and hope it installs with no problems
            // It will also still appear in the Assets.json so it will think its installed
            return InstallAsset(name, url);
        }

        std::ifstream file(ProjectManager::projectData.path / "assets.json");
        if (!file.is_open())
        {
            ConsoleLogger::ErrorLog("Package Manager - Failed to update asset.");
            ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
            if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                window->DockNode->TabBar->NextSelectedTabId = window->TabId;

            ImGuiToast toast(ImGuiToastType::Error, 2500, true);
            toast.setTitle("Package Manager", "");
            toast.setContent("Failed to update asset.");
            ImGui::InsertNotification(toast);
            return false;
        }

        // Update the assets.json
        if (!std::filesystem::exists(ProjectManager::projectData.path / "assets.json")) // This should already exist, but just in-case
        {
            std::ofstream file(ProjectManager::projectData.path / "assets.json");
            if (file.is_open())
            {
                file << nlohmann::json::array().dump(4);
                file.close();
            }
        }

        try {
            installedAssets = nlohmann::json::parse(file);
            file.close();

            for (nlohmann::json asset : installedAssets)
            {
                if (!asset.contains("url") || asset["url"].get<std::string>() != url)
                    continue;

                std::ifstream assetFile(path / "manifest.json");
                if (!assetFile.is_open())
                {
                    ConsoleLogger::ErrorLog("Package Manager - Failed to update asset. Error code 9008");
                    ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
                    if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                        window->DockNode->TabBar->NextSelectedTabId = window->TabId;

                    ImGuiToast toast(ImGuiToastType::Error, 2500, true);
                    toast.setTitle("Package Manager", "");
                    toast.setContent("Failed to update asset.");
                    ImGui::InsertNotification(toast);
                    return false;
                }

                nlohmann::json manifestJson;
                manifestJson = nlohmann::json::parse(assetFile);
                assetFile.close();

                manifestJson["path"] = path;
                asset = manifestJson;

                break;
            }

            std::ofstream outFile(ProjectManager::projectData.path / "Assets.json");
            if (!outFile.is_open())
            {
                ConsoleLogger::ErrorLog("Package Manager - Failed to update asset. Error code 9009");
                ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
                if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                    window->DockNode->TabBar->NextSelectedTabId = window->TabId;

                ImGuiToast toast(ImGuiToastType::Error, 2500, true);
                toast.setTitle("Package Manager", "");
                toast.setContent("Failed to update asset.");
                ImGui::InsertNotification(toast);                
                
                return false;
            }

            outFile << installedAssets.dump(4);
            outFile.close();
        }
        catch (const std::exception& e)
        {
            // Todo: Handle this
            //ConsoleLogger::ErrorLog("Error parsing JSON: " + std::string(e.what()));\

            ConsoleLogger::ErrorLog("Package Manager - Failed to update asset. Error code 9010");
            ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
            if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                window->DockNode->TabBar->NextSelectedTabId = window->TabId;

            ImGuiToast toast(ImGuiToastType::Error, 2500, true);
            toast.setTitle("Package Manager", "");
            toast.setContent("Failed to update asset.");
            ImGui::InsertNotification(toast);

            return false;
        }

        return true;
    }

    bool UninstallAsset(std::string url)
    {
        std::ifstream file(ProjectManager::projectData.path / "assets.json");
        if (!file.is_open())
        {
            ConsoleLogger::ErrorLog("Package Manager - Failed to uninstall asset. Error code 9011");
            ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
            if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                window->DockNode->TabBar->NextSelectedTabId = window->TabId;

            ImGuiToast toast(ImGuiToastType::Error, 2500, true);
            toast.setTitle("Package Manager", "");
            toast.setContent("Failed to install asset.");
            ImGui::InsertNotification(toast);
            return false;
        }

        installedAssets = nlohmann::json::parse(file);
        file.close();
        installedAssets = installedAssets;

        std::string name;
        std::string path;

        for (auto it = installedAssets.begin(); it != installedAssets.end(); )
        {
            if (it->contains("url") && it->at("url").get<std::string>() == url)
            {
                name = it->at("name").get<std::string>();
                if (it->contains("path"))
                    path = it->at("path").get<std::string>();

                it = installedAssets.erase(it);

                std::ofstream file(ProjectManager::projectData.path / "assets.json");
                if (file.is_open())
                {
                    file << installedAssets.dump(4);
                    file.close();
                }

                break;
            }
            else
                ++it;
        }

        if (name.empty())
        {
            ConsoleLogger::ErrorLog("Package Manager - Failed to uninstall asset. Error code 9012");
            ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
            if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                window->DockNode->TabBar->NextSelectedTabId = window->TabId;

            ImGuiToast toast(ImGuiToastType::Error, 2500, true);
            toast.setTitle("Package Manager", "");
            toast.setContent("Failed to install asset.");
            ImGui::InsertNotification(toast);
            return false;
        }


        std::filesystem::path thirdPartyPath = ProjectManager::projectData.path / "Assets" / "Third Party" / name;

        if (path.empty() || !std::filesystem::exists(path)) // Checking the last known path
        {
            path = (ProjectManager::projectData.path / "Assets" / "Third Party" / name).string();
            if (!std::filesystem::exists(path)) // Checking the Third Party folder
            {
                // Checking Third Party directory first incase if the user moved it into another folder
                if (std::filesystem::exists(ProjectManager::projectData.path / "Assets" / "Third Party"))
                {
                    path = "";
                    for (auto it = std::filesystem::recursive_directory_iterator(ProjectManager::projectData.path / "Assets" / "Third Party"); it != std::filesystem::end(it); ++it)
                    {
                        if (!it->is_directory())
                            continue;
                        
                        if (std::filesystem::exists(it->path() / "manifest.json")) // Checking if its an asset and has the manifest.json
                        {
                            std::ifstream manifestFile(it->path() / "manifest.json");
                            if (!manifestFile.is_open())
                                continue;

                            nlohmann::json manifestJson;
                            manifestJson = nlohmann::json::parse(manifestFile);
                            manifestFile.close();

                            if (manifestJson.contains("url") && manifestJson["url"].get<std::string>() == url)
                            {
                                path = it->path().string();
                                break;
                            }
                            else
                                it.disable_recursion_pending(); // Makes it so it stops searching this directory since its assumed an asset wouldnt be within an asset
                        }
                    }
                }

                if (path.empty()) // Checks all directories within /Assets, except /Third Party
                {
                    for (auto it = std::filesystem::recursive_directory_iterator(ProjectManager::projectData.path / "Assets"); it != std::filesystem::end(it); ++it)
                    {
                        if (!it->is_directory())
                            continue;

                        if (it->path().filename() == "Third Party")
                        {
                            it.disable_recursion_pending();
                            continue;
                        }

                        if (std::filesystem::exists(it->path() / "manifest.json")) // Checking if its an asset and has the manifest.json
                        {
                            std::ifstream manifestFile(it->path() / "manifest.json");
                            if (!manifestFile.is_open())
                                continue;

                            nlohmann::json manifestJson;
                            manifestJson = nlohmann::json::parse(manifestFile);
                            manifestFile.close();

                            if (manifestJson.contains("url") && manifestJson["url"].get<std::string>() == url)
                            {
                                path = it->path().string();
                                break;
                            }
                            else
                                it.disable_recursion_pending(); // Makes it so it stops searching this directory since its assumed an asset wouldnt be within an asset
                        }
                    }
                }
            }
        }

        if (!path.empty())
            std::filesystem::remove_all(path);
        else
        {
            ConsoleLogger::ErrorLog("Package Manager - Failed to find asset to uninstall. The asset may already be uninstalled, if not, try to locate and delete it yourself.");
            ImGuiWindow* window = ImGui::FindWindowByName((ICON_FA_CODE + std::string(" Console")).c_str());
            if (window != NULL && window->DockNode != NULL && window->DockNode->TabBar != NULL)
                window->DockNode->TabBar->NextSelectedTabId = window->TabId;

            ImGuiToast toast(ImGuiToastType::Error, 2500, true);
            toast.setTitle("Package Manager", "");
            toast.setContent("Failed to uninstall asset.");
            ImGui::InsertNotification(toast);
            return false;
        }

        return true;
    }

    void Init(ImGuiWindowClass* winClass)
    {
        windowClass = winClass;
        curl_global_init(CURL_GLOBAL_ALL); // This would be called in Editor.cpp, but including Curl causes some issues

        std::ifstream file(ProjectManager::projectData.path / "assets.json");
        if (!file.is_open())
            return;

        try {
            installedAssets = nlohmann::json::parse(file);
            file.close();
        }
        catch (const std::exception& e) {
            return;
        }
    }

    void Cleanup()
    {
        for (AssetData& assetData : cachedAssets)
            RaylibWrapper::UnloadTexture(assetData.icon);

        cachedAssets.clear();
        assetsCached = false;
    }

    std::vector<unsigned char> DownloadImageToMemory(const std::string& url)
    {
        CURL* curl = curl_easy_init();
        std::vector<unsigned char> imageData;

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteImageDataToMemory);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imageData);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

            // Todo: This disables the SSL Verification. For some reason curl_easy_perform fails when it is enabled
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                ConsoleLogger::ErrorLog("Image download failed : " + std::string(curl_easy_strerror(res)));
                imageData.clear();
            }
            curl_easy_cleanup(curl);
        }
        return imageData;
    }

    AssetData FetchAsset(std::string url)
    {
        std::string readBuffer;
        CURL* curl = curl_easy_init();
        if (curl)
        {
            //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

            // Todo: This disables the SSL Verification. For some reason curl_easy_perform fails when it is enabled
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                curl_easy_cleanup(curl);
                return AssetData();
            }

            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code != 200)
            {
                ConsoleLogger::ErrorLog("HTTP request failed. Error: " + std::string(curl_easy_strerror(res)), true);
                return AssetData();
            }

            curl_easy_cleanup(curl);
        }
        else
            return AssetData();

        try
        {
            nlohmann::json assets = nlohmann::json::parse(readBuffer);

            for (const auto& asset : assets)
            {
                // Add this
            }
        }
        catch (nlohmann::json::parse_error& e)
        {
            ConsoleLogger::ErrorLog("JSON parsing error: " + std::string(e.what()), true);
            return AssetData();
        }
    }

    bool FetchAssets()
    {
        std::string readBuffer;
        CURL* curl = curl_easy_init();
        if (curl)
        {
            //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            curl_easy_setopt(curl, CURLOPT_URL, "https://assetregistry.cryonicengine.com/assets.json");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

            // Todo: This disables the SSL Verification. For some reason curl_easy_perform fails when it is enabled
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                curl_easy_cleanup(curl);
                return false;
            }

            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code != 200)
            {
                ConsoleLogger::ErrorLog("HTTP request failed. Response code: " + response_code, true);
                return false;
            }

            curl_easy_cleanup(curl);
        }
        else
            return false;

        try
        {
            nlohmann::json assets = nlohmann::json::parse(readBuffer);

            for (const auto& asset : assets)
            {
                if (asset.contains("name") && asset.contains("description") && asset.contains("author") && asset.contains("url") && asset.contains("category"))
                {
                    // Todo: Check if the icon page exists

                    std::string imageUrl = asset["url"].get<std::string>();
                    size_t pos = imageUrl.find("github.com");
                    if (pos != std::string::npos)
                    {
                        imageUrl.replace(pos, 10, "raw.githubusercontent.com");
                        imageUrl += "/refs/heads/main/icon.png";
                    }

                    AssetData assetData;
                    assetData.name = asset["name"].get<std::string>();
                    assetData.description = asset["description"].get<std::string>();
                    assetData.author = asset["author"].get<std::string>();
                    assetData.category = asset["category"].get<std::string>();
                    assetData.url = asset["url"].get<std::string>();

                    // Github only supports HTTPS, this curl lib only works with HTTP
                    // Once HTTPS is supported, change the website in HasInternetConnection() in Utilities.
                    
                    std::vector<unsigned char> imageData = DownloadImageToMemory(imageUrl);
                    RaylibWrapper::Image image = RaylibWrapper::LoadImageFromMemory(".png", imageData.data(), imageData.size());
                    assetData.icon = RaylibWrapper::LoadTextureFromImage(image);
                    RaylibWrapper::UnloadImage(image);

                    cachedAssets.push_back(assetData);
                }
            }
        }
        catch (nlohmann::json::parse_error& e)
        {
            ConsoleLogger::ErrorLog("JSON parsing error: " + std::string(e.what()), true);
            return false;
        }

        assetsCached = true;
        return true;
    }
}