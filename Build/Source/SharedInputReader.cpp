#include "SharedInputReader.h"
#include "Systems/Input/InputSystem.h"
#include <Windows.h>

namespace InputReader {
	HANDLE hMapFile = nullptr;
}

bool SharedInputReader::Connect()
{
	InputReader::hMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, L"Local\\GameInputSharedMemory");
	if (!InputReader::hMapFile)
		return false;

	sharedState = (SharedInputState*)MapViewOfFile(InputReader::hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedInputState));
	if (!sharedState)
	{
		CloseHandle(InputReader::hMapFile);
		return false;
	}

	return true;
}

void SharedInputReader::Disconnect()
{
	if (sharedState)
		UnmapViewOfFile(sharedState);
	if (InputReader::hMapFile)
		CloseHandle(InputReader::hMapFile);
}

void SharedInputReader::SyncToInputSystem()
{
	if (!sharedState || !sharedState->newFrameAvailable.load(std::memory_order_acquire))
		return;

	// Clear existing input sets
	Keyboard::keyDown.clear();
	Keyboard::keyPressed.clear();
	Keyboard::keyReleased.clear();

	Mouse::buttonDown.clear();
	Mouse::buttonPressed.clear();
	Mouse::buttonReleased.clear();

	Gamepad::buttonDown.clear();
	Gamepad::buttonPressed.clear();
	Gamepad::buttonReleased.clear();

	// Populate keyboard sets from lists
	for (uint32_t i = 0; i < sharedState->numKeyDown; ++i)
		Keyboard::keyDown.insert(sharedState->keyDownList[i]);

	for (uint32_t i = 0; i < sharedState->numKeyPressed; ++i)
		Keyboard::keyPressed.insert(sharedState->keyPressedList[i]);

	for (uint32_t i = 0; i < sharedState->numKeyReleased; ++i)
		Keyboard::keyReleased.insert(sharedState->keyReleasedList[i]);

	// Populate mouse button sets from lists
	for (uint32_t i = 0; i < sharedState->numMouseDown; ++i)
		Mouse::buttonDown.insert(sharedState->mouseDownList[i]);

	for (uint32_t i = 0; i < sharedState->numMousePressed; ++i)
		Mouse::buttonPressed.insert(sharedState->mousePressedList[i]);

	for (uint32_t i = 0; i < sharedState->numMouseReleased; ++i)
		Mouse::buttonReleased.insert(sharedState->mouseReleasedList[i]);

	// Populate gamepad button sets from lists
	for (uint32_t i = 0; i < sharedState->numGamepadDown; ++i)
		Gamepad::buttonDown.insert(sharedState->gamepadDownList[i]);

	for (uint32_t i = 0; i < sharedState->numGamepadPressed; ++i)
		Gamepad::buttonPressed.insert(sharedState->gamepadPressedList[i]);

	for (uint32_t i = 0; i < sharedState->numGamepadReleased; ++i)
		Gamepad::buttonReleased.insert(sharedState->gamepadReleasedList[i]);

	// Set mouse positions and wheel
	Mouse::mousePos = { sharedState->mouseX, sharedState->mouseY };
	Mouse::deltaPos = { sharedState->mouseDeltaX, sharedState->mouseDeltaY };
	Mouse::mouseWheelMove = sharedState->mouseWheelMove;

	// Reset the flag
	sharedState->newFrameAvailable.store(false, std::memory_order_release);
}