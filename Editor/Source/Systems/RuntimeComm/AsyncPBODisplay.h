#include "Raylib/RaylibWrapper.h"

class AsyncPBODisplay {
public:
    void* pBuf = nullptr;
    int width = 1920;
    int height = 1080;
    RaylibWrapper::Texture2D sharedTexture;
    bool initialized = false;

    bool ConnectToSharedMemory();
    void Disconnect();
    void Update();
};
