#include "InputSystem.h"
#include "Raylib/RaylibInputWrapper.h"
#if !defined(EDITOR)
#include "Game.h";
#endif

// Keyboard
std::unordered_set<KeyboardKey> Keyboard::keyPressed;
std::unordered_set<KeyboardKey> Keyboard::keyReleased;
std::unordered_set<KeyboardKey> Keyboard::keyDown;

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
		if (buttonPressed.find(key) != buttonPressed.end())
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
		if (buttonReleased.find(key) != buttonReleased.end())
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
		if (buttonDown.find(key) != buttonDown.end())
			return true;

		return false;
	}
#endif

	return IsGamepadButtonDownWrapper(static_cast<int>(button));
}