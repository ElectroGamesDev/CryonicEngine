#include "FontManager.h"
#include <vector>
#include <iostream>
#include <unordered_map> 
#include <utility>
//#include "ThirdParty/imgui/imgui_impl_raylib.h"
#if defined(EDITOR)
#include "ThirdParty/imgui/IconsFontAwesome6.h"
#include "Core/ProjectManager.h"
#else
#include "Game.h"
#endif
#include "Raylib/RaylibWrapper.h"

namespace FontManager
{
	static ImFont* defaultFont;
	static std::vector<std::pair<std::string, int>> unloadedFonts;
	static std::unordered_map<std::string, std::unordered_map<int, ImFont*>> fonts;
	static bool updateFonts = false;

	void InitFontManager()
	{
#if defined(EDITOR)
		defaultFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/fonts/Familiar-Pro-Bold.ttf", 16);
#else
		//defaultFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/fonts/Roboto-Medium.ttf", 16); // A default font shouldn't be needed
#endif
		RaylibWrapper::rlImGuiReloadFonts();
	}

	ImFont* LoadFont(std::string font, int size, bool iconFont, bool checkIfExists)
	{
		if (checkIfExists)
		{
			auto fontIter = fonts.find(font);
			if (fontIter != fonts.end())
			{
				auto& sizeMap = fontIter->second;
				auto sizeIter = sizeMap.find(size);

				if (sizeIter != sizeMap.end())
					return sizeIter->second;
			}
		}

		if (iconFont)
		{
#if defined(EDITOR)
			ImFontConfig config;
			config.MergeMode = true;
			static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
			ImGuiIO& io = ImGui::GetIO();
			io.Fonts->AddFontFromFileTTF(("resources/fonts/" + font + ".ttf").c_str(), size, &config, icon_ranges);
			//io.Fonts->Build();
			RaylibWrapper::rlImGuiReloadFonts();
			//ConsoleLogger::InfoLog(font + " font created at size " + std::to_string(size));
#endif
		}
		else
		{
			//fonts[font][size] = io.Fonts->AddFontFromFileTTF(("resources/fonts/" + font +".ttf").c_str(), size);
			//rlImGuiReloadFonts();
			unloadedFonts.push_back(std::make_pair(font, size));
			//ConsoleLogger::InfoLog(font + " font created at size " + std::to_string(size));
			//return fonts[font][size];
			updateFonts = true;
		}
		return defaultFont; // Returns the default font since the new font can't be used until the next frame
	}

	void LoadFonts(std::string font, std::vector<int> sizes)
	{
		for (auto& size : sizes)
			LoadFont(font, size, false);
	}

	ImFont* GetFont(std::string font, int size, bool checkIfExists)
	{
		//for (auto& pair : fonts)
		//{
		//	if (pair.first != font) continue;
		//	for (auto& fontSize: pair.second)
		//	{
		//		if (fontSize.first != size) continue;
		//		return fontSize.second;
		//	}
		//}

		if (!checkIfExists)
			return fonts[font][size];

		auto fontIter = fonts.find(font);
		if (fontIter != fonts.end()) {
			auto& sizeMap = fontIter->second;
			auto sizeIter = sizeMap.find(size);

			if (sizeIter != sizeMap.end())
				return sizeIter->second;
		}

		return LoadFont(font, size, false, false);
	}

	void UpdateFonts()
	{
		if (!updateFonts) return;
		updateFonts = false;
		ImGuiIO& io = ImGui::GetIO();

		for (auto& font : unloadedFonts)
		{
#if defined(EDITOR)
			bool updatedFont = false;
			if (font.first.length() > 3)
			{
				std::string extension = font.first.substr(font.first.length() - 4);
				if (extension == ".ttf" || extension == ".otf")
				{
					fonts[font.first][font.second] = io.Fonts->AddFontFromFileTTF((ProjectManager::projectData.path.string() + "/Assets/" + font.first).c_str(), font.second);
					updatedFont = true;
				}
			}

			if (!updatedFont)
				fonts[font.first][font.second] = io.Fonts->AddFontFromFileTTF(("resources/fonts/" + font.first + ".ttf").c_str(), font.second);
#else
			if (exeParent.empty())
				fonts[font.first][font.second] = io.Fonts->AddFontFromFileTTF(("resources/Assets/" + font.first).c_str(), font.second);
			else
				fonts[font.first][font.second] = io.Fonts->AddFontFromFileTTF((exeParent.string() + "/resources/Assets/" + font.first).c_str(), font.second);
#endif
		}
		unloadedFonts.clear();

		RaylibWrapper::rlImGuiReloadFonts();
	}
}