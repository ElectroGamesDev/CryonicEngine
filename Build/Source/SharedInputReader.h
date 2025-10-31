#pragma once

#include "Systems/RuntimeComm/SharedInput.h"

class SharedInputReader
{
public:
	SharedInputState* sharedState = nullptr;

	bool Connect();
	void Disconnect();
	void SyncToInputSystem();
};