#include "SharedInputWriter.h"
#include <algorithm>
#define NOMINMAX // Needed for the min function
#include <Windows.h>

namespace InputWriter {
	HANDLE hMapFile = nullptr;
}

bool SharedInputWriter::Connect()
{
	InputWriter::hMapFile = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
		sizeof(SharedInputState),
		L"Local\\GameInputSharedMemory");
	if (!InputWriter::hMapFile)
		return false;

	sharedState = (SharedInputState*)MapViewOfFile(InputWriter::hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedInputState));
	if (!sharedState)
	{
		CloseHandle(InputWriter::hMapFile);
		return false;
	}

	ZeroMemory(sharedState, sizeof(SharedInputState));
	return true;
}

void SharedInputWriter::Disconnect()
{
	if (sharedState)
		UnmapViewOfFile(sharedState);
	if (InputWriter::hMapFile)
		CloseHandle(InputWriter::hMapFile);
}

//void SharedInputWriter::AddKeyDown(KeyboardKey key)
//{
//	if (key >= 0 && key < MAX_KEYS)
//		accKeyDown.push_back(key);
//}
//
//void SharedInputWriter::AddKeyPressed(KeyboardKey key)
//{
//	if (key >= 0 && key < MAX_KEYS)
//		accKeyPressed.push_back(key);
//}
//
//void SharedInputWriter::AddKeyReleased(KeyboardKey key)
//{
//	if (key >= 0 && key < MAX_KEYS)
//		accKeyReleased.push_back(key);
//}
//
//void SharedInputWriter::AddMouseButtonDown(MouseButton button)
//{
//	if (button >= 0 && button < MAX_MOUSE_BUTTONS)
//		accMouseDown.push_back(button);
//}
//
//void SharedInputWriter::AddMouseButtonPressed(MouseButton button)
//{
//	if (button >= 0 && button < MAX_MOUSE_BUTTONS)
//		accMousePressed.push_back(button);
//}
//
//void SharedInputWriter::AddMouseButtonReleased(MouseButton button)
//{
//	if (button >= 0 && button < MAX_MOUSE_BUTTONS)
//		accMouseReleased.push_back(button);
//}
//
//void SharedInputWriter::AddGamepadButtonDown(GamepadButton button)
//{
//	if (button >= 0 && button < MAX_GAMEPAD_BUTTONS)
//		accGamepadDown.push_back(button);
//}
//
//void SharedInputWriter::AddGamepadButtonPressed(GamepadButton button)
//{
//	if (button >= 0 && button < MAX_GAMEPAD_BUTTONS)
//		accGamepadPressed.push_back(button);
//}
//
//void SharedInputWriter::AddGamepadButtonReleased(GamepadButton button)
//{
//	if (button >= 0 && button < MAX_GAMEPAD_BUTTONS)
//		accGamepadReleased.push_back(button);
//}
//
//void SharedInputWriter::SetMousePosition(float x, float y)
//{
//	accMouseX = x;
//	accMouseY = y;
//	mousePosSet = true;
//}
//
//void SharedInputWriter::SetMouseDelta(float dx, float dy)
//{
//	accMouseDeltaX = dx;
//	accMouseDeltaY = dy;
//	mouseDeltaSet = true;
//}
//
//void SharedInputWriter::SetMouseWheelMove(float move)
//{
//	accMouseWheelMove = move;
//	mouseWheelSet = true;
//}

void SharedInputWriter::SendInputFrame()
{
	if (!sharedState)
		return;

	// Keyboard
	uint32_t keyDownCount = 0;
	uint32_t keyPressedCount = 0;
	uint32_t keyReleasedCount = 0;

	constexpr std::pair<int, int> keyboardRanges[] = {
		{39, 39},
		{44, 57},
		{59, 59},
		{61, 61},
		{65, 90},
		{91, 93},
		{96, 96},
		{32, 32},
		{256, 269},
		{280, 284},
		{290, 301},
		{340, 348},
		{320, 336}
	};

	for (const auto& [start, end] : keyboardRanges)
	{
		for (int key = start; key <= end; ++key)
		{
			auto k = static_cast<KeyboardKey>(key);

			if (Keyboard::IsKeyDown(k) && keyDownCount < MAX_INPUT_EVENTS)
				sharedState->keyDownList[keyDownCount++] = k;

			if (Keyboard::IsKeyPressed(k) && keyPressedCount < MAX_INPUT_EVENTS)
				sharedState->keyPressedList[keyPressedCount++] = k;

			if (Keyboard::IsKeyReleased(k) && keyReleasedCount < MAX_INPUT_EVENTS)
				sharedState->keyReleasedList[keyReleasedCount++] = k;

			if (keyDownCount >= MAX_INPUT_EVENTS && keyPressedCount >= MAX_INPUT_EVENTS && keyReleasedCount >= MAX_INPUT_EVENTS)
				break;
		}
	}

	sharedState->numKeyDown = keyDownCount;
	sharedState->numKeyPressed = keyPressedCount;
	sharedState->numKeyReleased = keyReleasedCount;

	// Mouse
	uint32_t mouseDownCount = 0;
	uint32_t mousePressedCount = 0;
	uint32_t mouseReleasedCount = 0;

	for (int button = 0; button <= MOUSE_BUTTON_BACK; ++button)
	{
		if (Mouse::IsButtonDown((MouseButton)button) && mouseDownCount < MAX_INPUT_EVENTS)
			sharedState->mouseDownList[mouseDownCount++] = (MouseButton)button;

		if (Mouse::IsButtonPressed((MouseButton)button) && mousePressedCount < MAX_INPUT_EVENTS)
			sharedState->mousePressedList[mousePressedCount++] = (MouseButton)button;

		if (Mouse::IsButtonReleased((MouseButton)button) && mouseReleasedCount < MAX_INPUT_EVENTS)
			sharedState->mouseReleasedList[mouseReleasedCount++] = (MouseButton)button;
	}

	sharedState->numMouseDown = mouseDownCount;
	sharedState->numMousePressed = mousePressedCount;
	sharedState->numMouseReleased = mouseReleasedCount;

	// Gamepad
	uint32_t gamepadDownCount = 0;
	uint32_t gamepadPressedCount = 0;
	uint32_t gamepadReleasedCount = 0;

	for (int button = 0; button <= GAMEPAD_BUTTON_RIGHT_THUMB; ++button)
	{
		if (Gamepad::IsButtonDown((GamepadButton)button) && gamepadDownCount < MAX_INPUT_EVENTS)
			sharedState->gamepadDownList[gamepadDownCount++] = (GamepadButton)button;

		if (Gamepad::IsButtonPressed((GamepadButton)button) && gamepadPressedCount < MAX_INPUT_EVENTS)
			sharedState->gamepadPressedList[gamepadPressedCount++] = (GamepadButton)button;

		if (Gamepad::IsButtonReleased((GamepadButton)button) && gamepadReleasedCount < MAX_INPUT_EVENTS)
			sharedState->gamepadReleasedList[gamepadReleasedCount++] = (GamepadButton)button;
	}

	sharedState->numGamepadDown = gamepadDownCount;
	sharedState->numGamepadPressed = gamepadPressedCount;
	sharedState->numGamepadReleased = gamepadReleasedCount;

	// Move movement
	// Todo: Need to implement these, but the game view window position and size needs to be taken into consideration.
	//sharedState->mouseX = input.GetMouseX();
	//sharedState->mouseY = input.GetMouseY();
	//sharedState->mouseDeltaX = input.GetMouseDeltaX();
	//sharedState->mouseDeltaY = input.GetMouseDeltaY();
	sharedState->mouseWheelMove = Mouse::GetMouseWheelMove();

	//// Copy keyboard lists
	//sharedState->numKeyDown = std::min((uint32_t)accKeyDown.size(), (uint32_t)MAX_INPUT_EVENTS);
	//for (uint32_t i = 0; i < sharedState->numKeyDown; ++i)
	//	sharedState->keyDownList[i] = accKeyDown[i];

	//sharedState->numKeyPressed = std::min((uint32_t)accKeyPressed.size(), (uint32_t)MAX_INPUT_EVENTS);
	//for (uint32_t i = 0; i < sharedState->numKeyPressed; ++i)
	//	sharedState->keyPressedList[i] = accKeyPressed[i];

	//sharedState->numKeyReleased = std::min((uint32_t)accKeyReleased.size(), (uint32_t)MAX_INPUT_EVENTS);
	//for (uint32_t i = 0; i < sharedState->numKeyReleased; ++i)
	//	sharedState->keyReleasedList[i] = accKeyReleased[i];

	//// Copy mouse button lists
	//sharedState->numMouseDown = std::min((uint32_t)accMouseDown.size(), (uint32_t)MAX_INPUT_EVENTS);
	//for (uint32_t i = 0; i < sharedState->numMouseDown; ++i)
	//	sharedState->mouseDownList[i] = accMouseDown[i];

	//sharedState->numMousePressed = std::min((uint32_t)accMousePressed.size(), (uint32_t)MAX_INPUT_EVENTS);
	//for (uint32_t i = 0; i < sharedState->numMousePressed; ++i)
	//	sharedState->mousePressedList[i] = accMousePressed[i];

	//sharedState->numMouseReleased = std::min((uint32_t)accMouseReleased.size(), (uint32_t)MAX_INPUT_EVENTS);
	//for (uint32_t i = 0; i < sharedState->numMouseReleased; ++i)
	//	sharedState->mouseReleasedList[i] = accMouseReleased[i];

	//// Copy gamepad button lists
	//sharedState->numGamepadDown = std::min((uint32_t)accGamepadDown.size(), (uint32_t)MAX_INPUT_EVENTS);
	//for (uint32_t i = 0; i < sharedState->numGamepadDown; ++i)
	//	sharedState->gamepadDownList[i] = accGamepadDown[i];

	//sharedState->numGamepadPressed = std::min((uint32_t)accGamepadPressed.size(), (uint32_t)MAX_INPUT_EVENTS);
	//for (uint32_t i = 0; i < sharedState->numGamepadPressed; ++i)
	//	sharedState->gamepadPressedList[i] = accGamepadPressed[i];

	//sharedState->numGamepadReleased = std::min((uint32_t)accGamepadReleased.size(), (uint32_t)MAX_INPUT_EVENTS);
	//for (uint32_t i = 0; i < sharedState->numGamepadReleased; ++i)
	//	sharedState->gamepadReleasedList[i] = accGamepadReleased[i];

	//// Copy mouse values if set
	//if (mousePosSet) {
	//	sharedState->mouseX = accMouseX;
	//	sharedState->mouseY = accMouseY;
	//}
	//if (mouseDeltaSet) {
	//	sharedState->mouseDeltaX = accMouseDeltaX;
	//	sharedState->mouseDeltaY = accMouseDeltaY;
	//}
	//if (mouseWheelSet) {
	//	sharedState->mouseWheelMove = accMouseWheelMove;
	//}

	// Signal new frame
	sharedState->newFrameAvailable.store(true, std::memory_order_release);

	// Clear for next frame
	//ClearAccumulators();
}

//void SharedInputWriter::ClearAccumulators()
//{
//	accKeyDown.clear();
//	accKeyPressed.clear();
//	accKeyReleased.clear();
//
//	accMouseDown.clear();
//	accMousePressed.clear();
//	accMouseReleased.clear();
//
//	accGamepadDown.clear();
//	accGamepadPressed.clear();
//	accGamepadReleased.clear();
//
//	mousePosSet = false;
//	mouseDeltaSet = false;
//	mouseWheelSet = false;
//
//	accMouseX = 0.0f;
//	accMouseY = 0.0f;
//	accMouseDeltaX = 0.0f;
//	accMouseDeltaY = 0.0f;
//	accMouseWheelMove = 0.0f;
//}