#pragma once
#include <atomic>
#include <cstdint>
#include "Systems/Input/InputSystem.h"

constexpr int MAX_INPUT_EVENTS = 64;
constexpr int MAX_KEYS = 512;
constexpr int MAX_MOUSE_BUTTONS = 16;
constexpr int MAX_GAMEPAD_BUTTONS = 32;

struct SharedInputState
{
	std::atomic<bool> newFrameAvailable{ false };

	// Keyboard
	uint32_t numKeyDown;
	uint32_t numKeyPressed;
	uint32_t numKeyReleased;
	KeyboardKey keyDownList[MAX_INPUT_EVENTS];
	KeyboardKey keyPressedList[MAX_INPUT_EVENTS];
	KeyboardKey keyReleasedList[MAX_INPUT_EVENTS];

	// Mouse buttons
	uint32_t numMouseDown;
	uint32_t numMousePressed;
	uint32_t numMouseReleased;
	MouseButton mouseDownList[MAX_INPUT_EVENTS];
	MouseButton mousePressedList[MAX_INPUT_EVENTS];
	MouseButton mouseReleasedList[MAX_INPUT_EVENTS];

	// Gamepad buttons
	uint32_t numGamepadDown;
	uint32_t numGamepadPressed;
	uint32_t numGamepadReleased;
	GamepadButton gamepadDownList[MAX_INPUT_EVENTS];
	GamepadButton gamepadPressedList[MAX_INPUT_EVENTS];
	GamepadButton gamepadReleasedList[MAX_INPUT_EVENTS];

	// Mouse position, delta, wheel
	float mouseX;
	float mouseY;
	float mouseDeltaX;
	float mouseDeltaY;
	float mouseWheelMove;
};