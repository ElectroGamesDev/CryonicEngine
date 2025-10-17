#include "AsyncPBODisplay.h"
#include <Windows.h>

HANDLE hMapFile = NULL;

bool AsyncPBODisplay::ConnectToSharedMemory()
{
    hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, TEXT("Local\\GameFrameBuffer"));
    if (hMapFile == NULL) {
        return false;
    }
    pBuf = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, width * height * 4);
    if (pBuf == nullptr) {
        CloseHandle(hMapFile);
        hMapFile = NULL;
        return false;
    }

    // Create an empty texture on GPU once
    RaylibWrapper::Image tempImg = RaylibWrapper::GenImageColor(width, height, {0,0,0,0});
    sharedTexture = RaylibWrapper::LoadTextureFromImage(tempImg);
    RaylibWrapper::UnloadImage(tempImg);

    initialized = true;
    return true;
}

void AsyncPBODisplay::Disconnect()
{
    if (pBuf) {
        UnmapViewOfFile(pBuf);
        pBuf = nullptr;
    }
    if (hMapFile) {
        CloseHandle(hMapFile);
        hMapFile = NULL;
    }
    if (initialized) {
        UnloadTexture(sharedTexture);
        initialized = false;
    }
}

void AsyncPBODisplay::Update()
{
    if (!initialized || pBuf == nullptr)
        return;

    // Update texture with shared memory data (RGBA8)
    RaylibWrapper::UpdateTexture(sharedTexture, pBuf);
}