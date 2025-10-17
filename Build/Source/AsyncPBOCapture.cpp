#include "AsyncPBOCapture.h"

AsyncPBOCapture::~AsyncPBOCapture() {
    Stop();
    for (auto buf : cpuBuffers)
        delete[] buf;

    if (sharedMemory)
        UnmapViewOfFile(sharedMemory);

    if (hMapFile)
        CloseHandle(hMapFile);

    glDeleteBuffers(pboCount, pboIds.data());
}

void AsyncPBOCapture::Init(int width, int height)
{
    this->width = width;
    this->height = height;

    bufferSize = width * height * 4; // RGBA8
    pboCount = 3; // triple buffering
    pboIds.resize(pboCount);
    glGenBuffers(pboCount, pboIds.data());

    for (int i = 0; i < pboCount; i++) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, bufferSize, nullptr, GL_STREAM_READ);
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    // Allocate CPU buffers for copying from PBO
    cpuBuffers.resize(pboCount);
    for (int i = 0; i < pboCount; i++) {
        cpuBuffers[i] = new uint8_t[bufferSize];
        memset(cpuBuffers[i], 0, bufferSize);
    }

    // Setup shared memory 
    hMapFile = CreateFileMappingW(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
        bufferSize,
        L"Local\\GameFrameBuffer");
    if (hMapFile) {
        sharedMemory = (uint8_t*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, bufferSize);
    }
    else {
        sharedMemory = nullptr;
    }

    newFrameAvailable = false;
}

void AsyncPBOCapture::Start()
{
    running = true;
    captureThread = std::thread(&AsyncPBOCapture::CaptureLoop, this);
}

void AsyncPBOCapture::Stop()
{
    running = false;
    if (captureThread.joinable())
        captureThread.join();
}

void AsyncPBOCapture::AsyncReadback(RaylibWrapper::RenderTexture2D& renderTexture)
{
    RaylibWrapper::rlDrawRenderBatchActive(); // flush Raylib batch before raw GL

    int nextIndex = (pboIndex + 1) % pboCount;

    // Bind current PBO and issue glReadPixels asynchronously
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboIndex]);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // Map the PBO from previous frame (nextIndex) to get data (may be null if not ready)
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[nextIndex]);
    void* ptr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, bufferSize, GL_MAP_READ_BIT);
    if (ptr) {
        memcpy(cpuBuffers[nextIndex], ptr, bufferSize);
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

        // Copy to shared memory for Editor
        if (sharedMemory) {
            memcpy(sharedMemory, cpuBuffers[nextIndex], bufferSize);
            newFrameAvailable.store(true, std::memory_order_release);
        }
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    pboIndex = nextIndex;
}

void AsyncPBOCapture::CaptureLoop()
{
    // This thread could handle shared memory sync or processing if needed
    // But main thread issues ReadPixels because OpenGL contexts are usually single-threaded
    while (running)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
