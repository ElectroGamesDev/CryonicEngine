#pragma once
#include <vector>
#include "Systems/RuntimeComm/SharedInput.h"
#include "Systems/Input/InputSystem.h"

class SharedInputWriter
{
public:
	SharedInputState* sharedState = nullptr;

	bool Connect();
	void Disconnect();

	//void AddKeyDown(KeyboardKey key);
	//void AddKeyPressed(KeyboardKey key);
	//void AddKeyReleased(KeyboardKey key);

	//void AddMouseButtonDown(MouseButton button);
	//void AddMouseButtonPressed(MouseButton button);
	//void AddMouseButtonReleased(MouseButton button);

	//void AddGamepadButtonDown(GamepadButton button);
	//void AddGamepadButtonPressed(GamepadButton button);
	//void AddGamepadButtonReleased(GamepadButton button);

	//void SetMousePosition(float x, float y);
	//void SetMouseDelta(float dx, float dy);
	//void SetMouseWheelMove(float move);

	void SendInputFrame();

private:
	//std::vector<KeyboardKey> accKeyDown;
	//std::vector<KeyboardKey> accKeyPressed;
	//std::vector<KeyboardKey> accKeyReleased;

	//std::vector<MouseButton> accMouseDown;
	//std::vector<MouseButton> accMousePressed;
	//std::vector<MouseButton> accMouseReleased;

	//std::vector<GamepadButton> accGamepadDown;
	//std::vector<GamepadButton> accGamepadPressed;
	//std::vector<GamepadButton> accGamepadReleased;

	//float accMouseX = 0.0f;
	//float accMouseY = 0.0f;
	//float accMouseDeltaX = 0.0f;
	//float accMouseDeltaY = 0.0f;
	//float accMouseWheelMove = 0.0f;

	//bool mousePosSet = false;
	//bool mouseDeltaSet = false;
	//bool mouseWheelSet = false;

	//void ClearAccumulators();
};