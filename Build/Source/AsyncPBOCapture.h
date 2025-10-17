#if defined (EDITOR)
#include "../../RaylibWrapper.h"
#else
#include "RaylibWrapper.h"
#endif

#include "external/glad.h"
#include <thread>
#include <atomic>
#include <vector>
#include <Windows.h>

class AsyncPBOCapture {
public:
    // Let Editor check this atomic flag
    std::atomic<bool> newFrameAvailable;

    ~AsyncPBOCapture();
    void Init(int width, int height);
    void Start();
    void Stop();

    // Call this on main thread after rendering into renderTexture, before swapping buffers
    void AsyncReadback(RaylibWrapper::RenderTexture2D& renderTexture);
private:
    void CaptureLoop();

    int width, height;
    int pboCount = 0;
    size_t bufferSize;
    std::vector<GLuint> pboIds;
    std::vector<uint8_t*> cpuBuffers;
    int pboIndex;

    HANDLE hMapFile;
    uint8_t* sharedMemory;

    std::thread captureThread;
    std::atomic<bool> running = false;
};