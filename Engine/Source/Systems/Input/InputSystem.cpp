#include "InputSystem.h"
#include "Raylib/RaylibInputWrapper.h"

// Keyboard
bool Keyboard::IsKeyPressed(KeyboardKey key)
{
	return IsKeyPressedWrapper(static_cast<int>(key));
}

bool Keyboard::IsKeyReleased(KeyboardKey key)
{
	return IsKeyReleasedWrapper(static_cast<int>(key));
}

bool Keyboard::IsKeyDown(KeyboardKey key)
{
	return IsKeyDownWrapper(static_cast<int>(key));
}


// Mouse

// This is used because mouse inputs don't work on web if the input happens between BeginDrawing() and EndDrawing(). Edit: This has been commented out because it appears to work without it, and the Raylib wiki may be out of date
//#ifdef WEB
//std::unordered_set<MouseButton> Mouse::buttonsPressed;
//std::unordered_set<MouseButton> Mouse::buttonsReleased;
//std::unordered_set<MouseButton> Mouse::buttonsDown;
//#endif

bool Mouse::IsButtonPressed(MouseButton button)
{
//#ifdef WEB
//	return buttonsPressed.find(button) != buttonsPressed.end();
//#else
	return IsMouseButtonPressedWrapper(static_cast<int>(button));
//#endif
}

bool Mouse::IsButtonReleased(MouseButton button)
{
//#ifdef WEB
//	return buttonsReleased.find(button) != buttonsReleased.end();
//#else
	return IsMouseButtonReleasedWrapper(static_cast<int>(button));
//#endif
}

bool Mouse::IsButtonDown(MouseButton button)
{
//#ifdef WEB
//	return buttonsDown.find(button) != buttonsDown.end();
//#else
	return IsMouseButtonDownWrapper(static_cast<int>(button));
//#endif
}


// Gamepad
bool Gamepad::IsButtonPressed(GamepadButton button)
{
	return IsGamepadButtonPressedWrapper(static_cast<int>(button));
}

bool Gamepad::IsButtonReleased(GamepadButton button)
{
	return IsGamepadButtonReleasedWrapper(static_cast<int>(button));
}

bool Gamepad::IsButtonDown(GamepadButton button)
{
	return IsGamepadButtonDownWrapper(static_cast<int>(button));
}