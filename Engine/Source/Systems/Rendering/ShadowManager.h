#pragma once

#include "Raylib/RaylibWrapper.h"
#include "Core/GameObject.h"
#include "Components/Component.h"
#include "Systems/Scene/SceneManager.h"
#include "Raylib/RaylibCameraWrapper.h"

class ShadowManager
{
public:
    void Init(int id, int width = 1024, int height = 1024);
    ~ShadowManager();
    RaylibWrapper::Matrix RenderShadowPass();
    void SetCamera();
    static void LoadShaders();
    static void UnloadShaders();

    RaylibWrapper::RenderTexture2D shadowMapTexture;
    static RaylibWrapper::Shader shader;
    static RaylibWrapper::Shader materialPreviewShader;
    RaylibWrapper::Camera camera;
    RaylibWrapper::Vector3 lightDir;
    RaylibWrapper::Vector3 lightPos;
    int lightType;
    int lightDirLoc;
    int lightColLoc;
    int lightPosLoc;
    int lightTypeLoc;
    int lightVPLoc;
    int spotInnerLoc;
	int spotOuterLoc;
    int shadowMapLoc;
private:
    Vector2 size;

    RaylibWrapper::RenderTexture2D LoadShadowMapRenderTexture();
    RaylibWrapper::RenderTexture2D shadowmap;
};
