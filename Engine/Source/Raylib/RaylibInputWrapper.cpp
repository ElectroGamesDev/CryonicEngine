#include "RaylibInputWrapper.h"
#include "ThirdParty/Raylib/include/raylib.h"

// Keyboard
bool IsKeyPressedWrapper(int key)
{
	return IsKeyPressed(key);
}

bool IsKeyReleasedWrapper(int key)
{
	return IsKeyReleased(key);
}

bool IsKeyDownWrapper(int key)
{
	return IsKeyDown(key);
}


// Mouse
bool IsMouseButtonPressedWrapper(int button)
{
	return IsMouseButtonPressed(button);
}

bool IsMouseButtonReleasedWrapper(int button)
{
	return IsMouseButtonReleased(button);
}

bool IsMouseButtonDownWrapper(int button)
{
	return IsMouseButtonDown(button);
}


// Gamepad
bool IsGamepadButtonPressedWrapper(int button)
{
	return IsGamepadButtonPressed(0, button);
}

bool IsGamepadButtonReleasedWrapper(int button)
{
	return IsGamepadButtonReleased(0, button);
}

bool IsGamepadButtonDownWrapper(int button)
{
	return IsGamepadButtonDown(0, button);
}