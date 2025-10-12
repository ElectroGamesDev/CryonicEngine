#include "Utilities.h"
#include "ConsoleLogger.h"
#include <ShlObj.h>
#include <fileapi.h>
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <array>
#include <vector>
#include <chrono>
#include <iomanip>
#include <json.hpp>
#include <algorithm>
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"
#include <tlhelp32.h>
#include "curl/curl.h"
#include <random>
#include "AnimationImporter.h"

void Utilities::HideFile(std::filesystem::path path) // Todo: Figure out why the file isn't being hidden
{
    SetFileAttributes(path.wstring().c_str(), FILE_ATTRIBUTE_HIDDEN);
}

void Utilities::OpenPathInExplorer(std::filesystem::path path)
{
	ShellExecute(NULL, L"open", path.wstring().c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

int Utilities::GetNumberOfCores()
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    return sysInfo.dwNumberOfProcessors;
}

bool Utilities::HasInternetConnection()
{
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    curl_easy_setopt(curl, CURLOPT_URL, "https://google.com");
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    // Todo: This disables the SSL Verification. For some reason curl_easy_perform fails when it is enabled
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);
    bool isConnected = (res == CURLE_OK);
    curl_easy_cleanup(curl);

    return isConnected;
}

bool Utilities::IsProgramInstalled(const char* program)
{
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(program, "r"), _pclose);

    if (!pipe)
    {
        // Todo: Handle error here
        return false;
    }

    if (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)  // If the command does not exist, then it doesnt have an output
        return true;

    return false;
}

std::filesystem::path Utilities::CreateUniqueFile(std::filesystem::path directory, std::string name, std::string extension)
{
    std::filesystem::path filePath(directory);
    if (extension != "") // If its a file
    {
        filePath /= name + "." + extension;

        int count = 0;
        while (std::filesystem::exists(filePath))
        {
            ++count;
            filePath = std::filesystem::path(directory) / (name + " (" + std::to_string(count) + ")." + extension);
        }

        std::ofstream file(filePath);
        if (file.is_open())
        {
            file.close();
            return filePath;
        }

        CreateDataFile(filePath);
    }
    else // If its a directory
    {
        filePath /= name;

        int count = 0;
        while (std::filesystem::exists(filePath))
        {
            ++count;
            filePath = std::filesystem::path(directory) / (name + " (" + std::to_string(count) + ")");
        }

        if (std::filesystem::create_directory(filePath))
            return filePath;
    }
    return "";
}

std::string dataTypes[5] = { "int", "float", "bool", "char", "string" };
std::vector<std::pair<std::string, std::vector<std::string>>> enums; // Consider changing this to a unordered_set as it has random access
nlohmann::json Utilities::GetExposedVariables(std::filesystem::path path)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        ConsoleLogger::WarningLog("Failed to open script " + path.string() + " to get exposed variables.");
        return nullptr; // Empty
    }

    nlohmann::json json = nlohmann::json::array();

    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(std::filesystem::last_write_time(path) - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    std::time_t tt = std::chrono::system_clock::to_time_t(sctp);

    json.push_back(static_cast<int>(tt));


    nlohmann::json variablesJson = nlohmann::json::array();
    int accessModifier = 0; // 0 = private, 1 = public, 2 = protected
    std::string line;
    std::pair<std::string, std::vector<std::string>> definingEnum;
    while (std::getline(file, line))
    {
        //line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
        if (line.find("class ") != std::string::npos) // Todo: This is needed incase there are multiple classes in the header, but if a variable/function has "class" in the name, it will switch acess modifier to private ALTHOUGH having a space at the end may may this not an issue
            accessModifier = 0;
        if (line.find("public") != std::string::npos) // Not using else-if incase if this is same line as class
            accessModifier = 1;
        else if (line.find("private") != std::string::npos)
            accessModifier = 0;
        else if (line.find("public") != std::string::npos)
            accessModifier = 2;

        size_t enumPos = line.find("enum");
        if (enumPos != std::string::npos)
        {
            std::string tempLine = line;
            tempLine.erase(std::remove_if(tempLine.begin(), tempLine.end(), ::isspace), tempLine.end()); // Remove spaces
            tempLine = tempLine.substr(enumPos + 3); // Remove everything before enum name
            size_t bracePos = tempLine.find("{"); // Check if the line has a "{"
            if (bracePos != std::string::npos && tempLine != "{") // Checks if it has a "{" and it has values in it
            {
                definingEnum.first = tempLine.substr(0, bracePos); // Get the enum name
                tempLine = tempLine.substr(bracePos + 1); // Remove everything before the "{" so now its just the enum values and "}" and ";"

                size_t endBracePos = tempLine.find("}");
                if (bracePos != std::string::npos) // Has end brace on this line
                {
                    tempLine = tempLine.substr(0, endBracePos); // Get rid of everything after "}" so its only the enum values in the string
                    std::stringstream ss(tempLine);
                    while (ss.good())
                    {
                        std::string value;
                        std::getline(ss, value, ',');
                        definingEnum.second.push_back(value);
                    }
                    enums.push_back(definingEnum);
                    definingEnum.first = "";
                    definingEnum.second.clear();
                    continue;
                }
                else // There is no "}", so it will assume the rest of the charaters are enum values
                {
                    std::stringstream ss(tempLine);
                    while (ss.good())
                    {
                        std::string value;
                        std::getline(ss, value, ',');
                        if (value == "")
                            continue;
                        definingEnum.second.push_back(value);
                    }
                    continue;
                }
            }
            else if (tempLine != "")
            {
                definingEnum.first = tempLine; // If there is no "{", its safe to assume only the enum name is remaining on this line
                continue;
            }
        }
        else if (definingEnum.first != "") // Todo: Check if the code in the if above is even running as it seems even with single-lined enums it will still run this code when it shouldn't.
        {
            std::string tempLine = line;
            tempLine.erase(std::remove_if(tempLine.begin(), tempLine.end(), ::isspace), tempLine.end());
            if (tempLine == "" || tempLine == "{")
                continue;

            size_t endBracePos = tempLine.find("}");
            if (endBracePos != std::string::npos)
            {
                tempLine = tempLine.substr(0, endBracePos);
                enums.push_back(definingEnum);
                definingEnum.first = "";
                definingEnum.second.clear();
				continue;
            }
            std::stringstream ss(tempLine);
            while (ss.good())
            {
                std::string value;
                std::getline(ss, value, ',');
                if (value == "")
                    continue;
                definingEnum.second.push_back(value);
            }
            continue;
        }

        // Todo: this won't work if variables are declared among multiple tnes. Instead go through each line until ";" is found
        else if (line.find("(") == std::string::npos && line.find(")") == std::string::npos) // Todo: need to check if the line is commented. Support both single line comment, and multi-line comment. If single-lien commented, make sure to check if the "//" is before the declaration
        {
            if (line.find("Hide") != std::string::npos)
                continue;
            if (line.find(" ") != std::string::npos)
            {
                int type = 0;
                std::string selectedDataType;

                for (const std::string& dataType : dataTypes)
                    if (line.find(dataType) != std::string::npos)
                    {
                        type = 1;
                        selectedDataType = dataType;
                        break;
                    }
                if (type == 0)
                    for (const auto& dataType : enums)
                        if (line.find(dataType.first) != std::string::npos)
                        {
                            type = 2;
                            selectedDataType = dataType.first;
                            break;
                        }
                if (type != 0)
                {
                    if (accessModifier == 1 || line.find("Expose") != std::string::npos)
                    {
                        size_t startPos = line.find(selectedDataType);
                        size_t semicolonPos = line.find(";", startPos);
                        if (semicolonPos != std::string::npos)
                        {
                            std::string variableDeclaration = line.substr(startPos, semicolonPos - startPos);
                            std::stringstream ss(variableDeclaration);
                            std::string word;
                            nlohmann::json jsonArray = nlohmann::json::array();

                            // Adds the variable data type to the json
                            while (ss >> word)
                            {
                                if (word == "=")
                                    continue;
                                jsonArray.push_back(word);
                            }

                            //ConsoleLogger::InfoLog(jsonArray.dump());

                            if (jsonArray.size() <= 2) // Check if variable declared a value
                            {
                                if (jsonArray[0] == "int")
                                    jsonArray.push_back(0);
                                else if (jsonArray[0] == "float")
                                    jsonArray.push_back(0.0f);
                                else if (jsonArray[0] == "bool")
                                    jsonArray.push_back(false);
                                else if (jsonArray[0] == "string" || jsonArray[0] == "char")
                                    jsonArray.push_back("");
                                else if (type == 2) // Check if its an enum
                                {
                                    for (const auto& pair : enums)
                                        if (pair.first == selectedDataType)
                                        {
                                            if (pair.second.size() > 0)
                                                jsonArray.push_back(pair.second[0]);
                                            break;
                                        }
                                }
                            }
                            else
                            {
                                if (jsonArray[0] == "float")
                                {
                                    std::string value = jsonArray[2].get<std::string>();
                                    value.erase(value.size() - 1);
                                    jsonArray[2] = std::stof(value);
                                }
                                else if (jsonArray[0] == "string" || jsonArray[0] == "char")
                                {
                                    std::string value = jsonArray[2].get<std::string>();
                                    value.erase(0, 1);
                                    value.erase(value.size() - 1);
                                    jsonArray[2] = value;
                                }
                                else if (jsonArray[0] == "bool")
                                    jsonArray[2] = (jsonArray[2] == "true") ? true : false;
                            }

                            // Adding another variable name, and making the first letter capitalized
                            jsonArray.push_back(jsonArray[1]);
                            std::string newName = jsonArray.back();
                            if (!newName.empty()) {
                                newName[0] = std::toupper(newName[0]);

                                // Adding spaces to new variable name
                                bool firstLetter = true;
                                std::string name = newName;
                                newName = "";
                                for (char c : name)
                                {
                                    if (c == '_')
                                        newName += ' ';
                                    else if (std::isupper(c))
                                    {
                                        if (!firstLetter)
                                            newName += ' ';
                                        newName += c;
                                    }
                                    else
                                        newName += c;
                                    firstLetter = false;
                                }

                                jsonArray.back() = newName;
                            }

                            if (type == 2)
                                for (const auto& pair : enums)
                                    if (pair.first == selectedDataType)
                                    {
                                        nlohmann::json enumValuesJson = nlohmann::json::array();
                                        for (const std::string& enumValues : pair.second)
                                            enumValuesJson.push_back(enumValues);
                                        jsonArray.push_back(enumValuesJson);
                                        break;
                                    }

                            // Adding another variable value, and making the first letter capitalized
                            //jsonArray.push_back(jsonArray[2]);
                            //if (jsonArray.back().is_string())
                            //{
                            //    std::string newValue = jsonArray.back();
                            //    if (!newValue.empty()) {
                            //        newValue[0] = std::toupper(newValue[0]);
                            //        jsonArray.back() = newValue;
                            //    }
                            //}

                            variablesJson.push_back(jsonArray);
                        }
                    }
                }
            }
        }
    }
    json.push_back(variablesJson);

    file.close();

    return json;
}

std::vector<std::string> Utilities::GetGltfAnimationNames(std::filesystem::path path)
{
	std::vector<std::string> names;
	auto animations = AnimationImporter::GetAnimationInfo(path);

	for (const auto& anim : animations)
		names.push_back(anim.name);

	return names;
}

bool Utilities::CreateDataFile(std::filesystem::path path)
{
    if (!std::filesystem::exists(path) || !path.has_extension())
        return false;

    if (path.extension() == ".mp3" || path.extension() == ".wav" || path.extension() == ".ogg" || path.extension() == ".flac" || path.extension() == ".qoa" || path.extension() == ".xm" || path.extension() == ".mod")
    {
        std::filesystem::path newPath = path.parent_path() / (path.filename().string() + ".data");

        if (std::filesystem::exists(newPath))
            return false;

        std::ofstream file(newPath);

        bool loadInMemory = false;
        try
        {
            // If the audio file is less than 1MB, set it to load in memory on default
            if (std::filesystem::file_size(path) < 1048576)
                loadInMemory = true;
        }
        catch (std::filesystem::filesystem_error& error)
        {
            ConsoleLogger::WarningLog("Failed to check the size of the audio file \"" + path.stem().string() + "\" when creating the data file. Error: " + error.what(), true);
        }
        nlohmann::json jsonData = {
            {"public", {
                {"loadInMemory", loadInMemory}
            }},
            {"private", {
                {"version", 1.0f}
            }}
        };
        file << std::setw(4) << jsonData << std::endl;

        return true;
    }
    else if (path.extension() == ".mat")
    {
        std::ifstream infile(path);
        nlohmann::json existingData;

        if (infile.is_open()) {
            try {
                infile >> existingData;
            }
            catch (const std::exception& e) {
                existingData.clear();
            }
        }

        if (existingData.contains("public") && existingData.contains("private")) // Checks if the file has some integrity
            return false;

        std::ofstream file(path, std::ios::trunc);

        nlohmann::json jsonData = {
            {"public", {
                {"albedoTexture", {"file", {".png", ".jpg", ".jpeg"}, ""}},
                {"albedoColor", {255, 255, 255, 255}},
                {"normalTexture", {"file", {".png", ".jpg", ".jpeg"}, ""}},
                {"metallicTexture", {"file", {".png", ".jpg", ".jpeg"}, ""}},
                {"metallic", 0.0f},
                {"roughnessTexture", {"file", {".png", ".jpg", ".jpeg"}, ""}},
                {"roughness", 0.5f},
                {"emissionTexture", {"file", {".png", ".jpg", ".jpeg"}, ""}},
                {"emission", 0.0f}
            }},
            {"private", {
                {"version", 1.0f}
            }}
        };

        file << std::setw(4) << jsonData << std::endl;

        return true;
    }

    return false;
}

bool Utilities::ImportFile(std::filesystem::path filePath, std::filesystem::path importPath)
{
    try
    {
        if (std::filesystem::is_directory(filePath))
        {
            if (!std::filesystem::exists(importPath))
                std::filesystem::create_directories(importPath);

            for (const auto& entry : std::filesystem::directory_iterator(filePath))
                ImportFile(entry.path(), importPath / entry.path().filename());
        }
        else
        {
            if (std::filesystem::exists(importPath))
            {
                // Todo: Popup a window asking if the user would like to replace the file
                std::filesystem::remove(importPath);
            }
            std::filesystem::copy_file(filePath, importPath);

            if (!std::filesystem::exists(importPath.string() + ".data"))
                Utilities::CreateDataFile(importPath.string());
        }
    }
    catch (const std::filesystem::filesystem_error& error)
    {
        ConsoleLogger::WarningLog("Failed to import file. Error: " + std::string(error.what()));
        return false;
    }

    return true;
}

std::string Utilities::SelectFolderDialog(const std::filesystem::path& projectPath)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    std::string path;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* pFileOpen;

        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr))
        {
            DWORD dwOptions;
            hr = pFileOpen->GetOptions(&dwOptions);
            if (SUCCEEDED(hr))
            {
                pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);  // Use FOS_PICKFOLDERS for selecting folders

                IShellItem* pDefaultFolder;
                hr = SHCreateItemFromParsingName(projectPath.c_str(), NULL, IID_IShellItem, reinterpret_cast<void**>(&pDefaultFolder));

                if (SUCCEEDED(hr))
                {
                    pFileOpen->SetFolder(pDefaultFolder);
                    pDefaultFolder->Release();
                }
            }

            hr = pFileOpen->Show(NULL);

            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    if (SUCCEEDED(hr))
                    {
                        int len = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, NULL, 0, NULL, NULL);
                        std::string str(len, '\0');
                        WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, &str[0], len, NULL, NULL);
                        path = str;

                        CoTaskMemFree(pszFilePath);
                    }
                    else
                    {
                        path = "";
                    }
                    pItem->Release();
                }
                else
                {
                    path = "";
                }
            }
            else
            {
                path = "";
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }

    path.erase(std::remove_if(path.begin(), path.end(), [](char c) {
        return !std::isprint(static_cast<unsigned char>(c));
        }), path.end());
    return path;
}

std::pair<std::string, Utilities::JobHandle> Utilities::LaunchProcess(std::string startCommand)
{
    HANDLE hJob = CreateJobObject(nullptr, nullptr);

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));

    STARTUPINFOA si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    BOOL success = CreateProcessA(
        nullptr,
        startCommand.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_SUSPENDED,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    if (!success)
        return { std::to_string(GetLastError()), nullptr };

    AssignProcessToJobObject(hJob, pi.hProcess);

    ResumeThread(pi.hThread);

    CloseHandle(pi.hThread);

    return {"", (JobHandle)hJob};
}

void Utilities::TerminateProcess(int dwProcessId, int uExitCode)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Kill the child processes
    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            if (pe32.th32ParentProcessID == dwProcessId)
            {
                //TerminateProcess(pe32.th32ProcessID, uExitCode);
                HANDLE hChild = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                if (hChild)
                {
                    ::TerminateProcess(hChild, uExitCode);
                    CloseHandle(hChild);
                }
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
    if (hProcess != NULL)
    {
        ::TerminateProcess(hProcess, uExitCode);
        CloseHandle(hProcess);
    }
}

void Utilities::TerminateJob(JobHandle jobHandle)
{
    if (!jobHandle)
        return;

    CloseHandle((HANDLE)jobHandle);
}

std::filesystem::path Utilities::CreateTempFolder(std::filesystem::path projectPath)
{
    const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 generator(rd());  
    std::uniform_int_distribution<> dist(0, characters.size() - 1);

    std::string folderName;
    for (size_t i = 0; i < 20; ++i)
        folderName += characters[dist(generator)];

    std::filesystem::create_directories(projectPath / "Internal" / "Temp" / folderName);

    return projectPath / "Internal" / "Temp" / folderName;
}

std::filesystem::path Utilities::GetExePath()
{
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule != NULL)
    {
        char exePath[MAX_PATH];
        GetModuleFileNameA(hModule, exePath, (sizeof(exePath)));
        return exePath;
    }
    else
    {
        ConsoleLogger::WarningLog("Failed to get the exe path. There may be issues.", false);
        return std::filesystem::current_path();
    }
}