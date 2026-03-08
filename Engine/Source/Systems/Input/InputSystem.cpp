#include "InputSystem.h"
#include "Raylib/RaylibInputWrapper.h"
#if !defined(EDITOR)
#include "Game.h";
#endif
#include <unordered_map>
#include <algorithm>

// Keyboard
std::unordered_set<KeyboardKey> Keyboard::keyPressed;
std::unordered_set<KeyboardKey> Keyboard::keyReleased;
std::unordered_set<KeyboardKey> Keyboard::keyDown;

bool Keyboard::AnyKeyPressed()
{
	// Todo: Implement this
	return false;
}

bool Keyboard::AnyKeyReleased()
{
	// Todo: Implement this
	return false;
}

bool Keyboard::IsKeyPressed(KeyboardKey key)
{
#if !defined(EDITOR)
	if (isPlayMode)
	{
		if (keyPressed.find(key) != keyPressed.end())
			return true;

		return false;
	}
#endif

	return IsKeyPressedWrapper(static_cast<int>(key));
}

bool Keyboard::IsKeyReleased(KeyboardKey key)
{
#if !defined(EDITOR)
	if (isPlayMode)
	{
		if (keyReleased.find(key) != keyReleased.end())
			return true;

		return false;
	}
#endif

	return IsKeyReleasedWrapper(static_cast<int>(key));
}

bool Keyboard::IsKeyDown(KeyboardKey key)
{
#if !defined(EDITOR)
	if (isPlayMode)
	{
		if (keyDown.find(key) != keyDown.end())
			return true;

		return false;
	}
#endif

	return IsKeyDownWrapper(static_cast<int>(key));
}

KeyboardKey Keyboard::StringToKey(std::string key)
{
	static const std::unordered_map<std::string, KeyboardKey> map = {
		// Letters
		{"A", KEY_A}, {"B", KEY_B}, {"C", KEY_C}, {"D", KEY_D}, {"E", KEY_E},
		{"F", KEY_F}, {"G", KEY_G}, {"H", KEY_H}, {"I", KEY_I}, {"J", KEY_J},
		{"K", KEY_K}, {"L", KEY_L}, {"M", KEY_M}, {"N", KEY_N}, {"O", KEY_O},
		{"P", KEY_P}, {"Q", KEY_Q}, {"R", KEY_R}, {"S", KEY_S}, {"T", KEY_T},
		{"U", KEY_U}, {"V", KEY_V}, {"W", KEY_W}, {"X", KEY_X}, {"Y", KEY_Y},
		{"Z", KEY_Z},

		// Numbers
		{"0", KEY_ZERO}, {"1", KEY_ONE}, {"2", KEY_TWO}, {"3", KEY_THREE},
		{"4", KEY_FOUR}, {"5", KEY_FIVE}, {"6", KEY_SIX}, {"7", KEY_SEVEN},
		{"8", KEY_EIGHT}, {"9", KEY_NINE},

		// Function keys
		{"F1", KEY_F1}, {"F2", KEY_F2}, {"F3", KEY_F3}, {"F4", KEY_F4},
		{"F5", KEY_F5}, {"F6", KEY_F6}, {"F7", KEY_F7}, {"F8", KEY_F8},
		{"F9", KEY_F9}, {"F10", KEY_F10}, {"F11", KEY_F11}, {"F12", KEY_F12},

		// Modifier keys
		{"SHIFT", KEY_LEFT_SHIFT},
		{"LEFT_SHIFT", KEY_LEFT_SHIFT},
		{"RIGHT_SHIFT", KEY_RIGHT_SHIFT},

		{"CTRL", KEY_LEFT_CONTROL},
		{"LEFT_CTRL", KEY_LEFT_CONTROL},
		{"RIGHT_CTRL", KEY_RIGHT_CONTROL},
		{"CONTROL", KEY_LEFT_CONTROL},

		{"ALT", KEY_LEFT_ALT},
		{"LEFT_ALT", KEY_LEFT_ALT},
		{"RIGHT_ALT", KEY_RIGHT_ALT},

		{"CAPSLOCK", KEY_CAPS_LOCK},
		{"TAB", KEY_TAB},

		// Arrow keys
		{"UP", KEY_UP},
		{"DOWN", KEY_DOWN},
		{"LEFT", KEY_LEFT},
		{"RIGHT", KEY_RIGHT},

		// Control keys
		{"ENTER", KEY_ENTER},
		{"ESCAPE", KEY_ESCAPE},
		{"SPACE", KEY_SPACE},
		{"BACKSPACE", KEY_BACKSPACE},
		{"DELETE", KEY_DELETE},
		{"INSERT", KEY_INSERT},
		{"HOME", KEY_HOME},
		{"END", KEY_END},
		{"PAGEUP", KEY_PAGE_UP},
		{"PAGEDOWN", KEY_PAGE_DOWN},
		{"PRINTSCREEN", KEY_PRINT_SCREEN},
		{"PAUSE", KEY_PAUSE},
		{"NUMLOCK", KEY_NUM_LOCK},
		{"SCROLLLOCK", KEY_SCROLL_LOCK},

		// Numpad
		{"NUMPAD_0", KEY_KP_0}, {"NUMPAD_1", KEY_KP_1},
		{"NUMPAD_2", KEY_KP_2}, {"NUMPAD_3", KEY_KP_3},
		{"NUMPAD_4", KEY_KP_4}, {"NUMPAD_5", KEY_KP_5},
		{"NUMPAD_6", KEY_KP_6}, {"NUMPAD_7", KEY_KP_7},
		{"NUMPAD_8", KEY_KP_8}, {"NUMPAD_9", KEY_KP_9},
		{"NUMPAD_ADD", KEY_KP_ADD},
		{"NUMPAD_SUBTRACT", KEY_KP_SUBTRACT},
		{"NUMPAD_MULTIPLY", KEY_KP_MULTIPLY},
		{"NUMPAD_DIVIDE", KEY_KP_DIVIDE},
		{"NUMPAD_DECIMAL", KEY_KP_DECIMAL},
		{"NUMPAD_ENTER", KEY_KP_ENTER},

		// Symbols
		{"`", KEY_GRAVE},
		{"-", KEY_MINUS},
		{"=", KEY_EQUAL},
		{"[", KEY_LEFT_BRACKET},
		{"]", KEY_RIGHT_BRACKET},
		{"\\", KEY_BACKSLASH},
		{";", KEY_SEMICOLON},
		{"'", KEY_APOSTROPHE},
		{",", KEY_COMMA},
		{".", KEY_PERIOD},
		{"/", KEY_SLASH},
	};

	std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return (char)std::toupper(c); });

	auto it = map.find(key);
	if (it != map.end())
		return it->second;

	return (KeyboardKey)0;
}


// Mouse
std::unordered_set<MouseButton> Mouse::buttonPressed;
std::unordered_set<MouseButton> Mouse::buttonReleased;
std::unordered_set<MouseButton> Mouse::buttonDown;
Vector2 Mouse::mousePos = 0;
Vector2 Mouse::deltaPos = 0;
float Mouse::mouseWheelMove = 0;

// This is used because mouse inputs don't work on web if the input happens between BeginDrawing() and EndDrawing(). Edit: This has been commented out because it appears to work without it, and the Raylib wiki may be out of date
//#ifdef WEB
//std::unordered_set<MouseButton> Mouse::buttonsPressed;
//std::unordered_set<MouseButton> Mouse::buttonsReleased;
//std::unordered_set<MouseButton> Mouse::buttonsDown;
//#endif

bool Mouse::IsButtonPressed(MouseButton button)
{
#if !defined(EDITOR)
	if (isPlayMode)
	{
		if (buttonPressed.find(button) != buttonPressed.end())
			return true;

		return false;
	}
#endif

//#ifdef WEB
//	return buttonsPressed.find(button) != buttonsPressed.end();
//#else
	return IsMouseButtonPressedWrapper(static_cast<int>(button));
//#endif
}

bool Mouse::IsButtonReleased(MouseButton button)
{
#if !defined(EDITOR)
	if (isPlayMode)
	{
		if (buttonReleased.find(button) != buttonReleased.end())
			return true;

		return false;
	}
#endif

//#ifdef WEB
//	return buttonsReleased.find(button) != buttonsReleased.end();
//#else
	return IsMouseButtonReleasedWrapper(static_cast<int>(button));
//#endif
}

bool Mouse::IsButtonDown(MouseButton button)
{
#if !defined(EDITOR)
	if (isPlayMode)
	{
		if (buttonDown.find(button) != buttonDown.end())
			return true;

		return false;
	}
#endif

	//#ifdef WEB
	//	return buttonsDown.find(button) != buttonsDown.end();
	//#else
	return IsMouseButtonDownWrapper(static_cast<int>(button));
	//#endif
}

void Mouse::SetPosition(Vector2 pos)
{
	SetMousePositionWrapper(pos.x, pos.y);
}

Vector2 Mouse::GetPosition()
{
#if !defined(EDITOR)
	if (isPlayMode)
		return mousePos;
#endif

	std::array<float, 2> pos = GetMousePositionWrapper();
	return { pos[0], pos[1] };

}

Vector2 Mouse::GetDelta()
{
#if !defined(EDITOR)
	if (isPlayMode)
		return deltaPos;
#endif

	std::array<float, 2> pos = GetMouseDeltaWrapper();
	return { pos[0], pos[1] };
}

void Mouse::SetMouseScale(Vector2 scale)
{
	SetMouseScaleWrapper(scale.x, scale.y);
}

void Mouse::SetMouseCursor(MouseCursor cursor)
{
	SetMouseCursorWrapper((int)cursor);
}

float Mouse::GetMouseWheelMove()
{
#if !defined(EDITOR)
	if (isPlayMode)
		return mouseWheelMove;
#endif

	return GetMouseWheelMoveWrapper();
}

// Gamepad
std::unordered_set<GamepadButton> Gamepad::buttonPressed;
std::unordered_set<GamepadButton> Gamepad::buttonReleased;
std::unordered_set<GamepadButton> Gamepad::buttonDown;

bool Gamepad::IsButtonPressed(GamepadButton button)
{
#if !defined(EDITOR)
	if (isPlayMode)
	{
		if (buttonPressed.find(button) != buttonPressed.end())
			return true;

		return false;
	}
#endif

	return IsGamepadButtonPressedWrapper(static_cast<int>(button));
}

bool Gamepad::IsButtonReleased(GamepadButton button)
{
#if !defined(EDITOR)
	if (isPlayMode)
	{
		if (buttonReleased.find(button) != buttonReleased.end())
			return true;

		return false;
	}
#endif

	return IsGamepadButtonReleasedWrapper(static_cast<int>(button));
}

bool Gamepad::IsButtonDown(GamepadButton button)
{
#if !defined(EDITOR)
	if (isPlayMode)
	{
		if (buttonDown.find(button) != buttonDown.end())
			return true;

		return false;
	}
#endif

	return IsGamepadButtonDownWrapper(static_cast<int>(button));
}