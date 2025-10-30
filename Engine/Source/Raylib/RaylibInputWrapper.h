#include <array>

#pragma once

bool IsKeyPressedWrapper(int key);
bool IsKeyReleasedWrapper(int key);
bool IsKeyDownWrapper(int key);

bool IsMouseButtonPressedWrapper(int button);
bool IsMouseButtonReleasedWrapper(int button);
bool IsMouseButtonDownWrapper(int button);
void SetMousePositionWrapper(int x, int y);
std::array<int, 2> GetMousePositionWrapper();
std::array<int, 2> GetMouseDeltaWrapper();
void SetMouseScaleWrapper(float x, float y);
void SetMouseCursorWrapper(int cursor);
float GetMouseWheelMoveWrapper();

bool IsGamepadButtonPressedWrapper(int button);
bool IsGamepadButtonReleasedWrapper(int button);
bool IsGamepadButtonDownWrapper(int button);
