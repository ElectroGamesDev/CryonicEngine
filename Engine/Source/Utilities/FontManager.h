#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
//#include "rlImGui.h"
#include "ThirdParty/imgui/imgui.h"

namespace FontManager
{
    ImFont* LoadFont(std::string font, int size, bool iconFont = false, bool checkIfExists = false);
    ImFont* GetFont(std::string font, int size, bool checkIfExists = true);
    void LoadFonts(std::string font, std::vector<int> sizes);
    void UpdateFonts();
    void InitFontManager();
};