#include "ShadowManager.h"
#include "Raylib/RaylibModelWrapper.h"
#if !defined(EDITOR)
#include "Game.h"
#endif
//#include "Components/Lighting.h"

//RaylibWrapper::Camera ShadowManager::camera = {0};
RaylibWrapper::Shader ShadowManager::shader;
RaylibWrapper::Shader ShadowManager::materialPreviewShader;
//RaylibWrapper::RenderTexture2D ShadowManager::shadowMapTexture;
//RaylibWrapper::Vector3 ShadowManager::lightDir;
//int ShadowManager::lightDirLoc;
//int ShadowManager::lightVPLoc;
//int ShadowManager::shadowMapLoc;

// Most of the commented code is for a better shadowmapping method that I couldnt get working

void ShadowManager::Init(int id, int width, int height)
{
    size = { (float)width, (float)height };

    //shader.locs[RaylibWrapper::SHADER_LOC_VECTOR_VIEW] = RaylibWrapper::GetShaderLocation(shader, "viewPos");
    //lightDir = RaylibWrapper::Vector3Normalize({ 0.35f, -1.0f, -0.35f });
    //RaylibWrapper::Color lightColor = { 255, 255, 255, 255 };
    //RaylibWrapper::Vector4 lightColorNormalized = RaylibWrapper::ColorNormalize(lightColor);
    //lightDirLoc = RaylibWrapper::GetShaderLocation(shader, "lightDir");
    //int lightColLoc = RaylibWrapper::GetShaderLocation(shader, "lightColor");
    //RaylibWrapper::SetShaderValue(shader, lightDirLoc, &lightDir, RaylibWrapper::SHADER_UNIFORM_VEC3);
    //RaylibWrapper::SetShaderValue(shader, lightColLoc, &lightColorNormalized, RaylibWrapper::SHADER_UNIFORM_VEC4);
    //int ambientLoc = RaylibWrapper::GetShaderLocation(shader, "ambient");
    //float ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    //RaylibWrapper::SetShaderValue(shader, ambientLoc, ambient, RaylibWrapper::SHADER_UNIFORM_VEC4);
    //lightVPLoc = RaylibWrapper::GetShaderLocation(shader, "lightVP");
    //shadowMapLoc = RaylibWrapper::GetShaderLocation(shader, "shadowMap");
    //int shadowMapResolution = width;
    //RaylibWrapper::SetShaderValue(shader, RaylibWrapper::GetShaderLocation(shader, "shadowMapResolution"), &shadowMapResolution, RaylibWrapper::SHADER_UNIFORM_INT);

    shader.locs[RaylibWrapper::SHADER_LOC_VECTOR_VIEW] = RaylibWrapper::GetShaderLocation(shader, "viewPos");

    lightPosLoc = RaylibWrapper::GetShaderLocation(shader, ("lightPos[" + std::to_string(id - 1) + "]").c_str());
    lightDirLoc = RaylibWrapper::GetShaderLocation(shader, ("lightDir[" + std::to_string(id - 1) + "]").c_str());
    lightColLoc = RaylibWrapper::GetShaderLocation(shader, ("lightColor[" + std::to_string(id - 1) + "]").c_str());
    lightTypeLoc = RaylibWrapper::GetShaderLocation(shader, ("lightType[" + std::to_string(id - 1) + "]").c_str());
    lightVPLoc = RaylibWrapper::GetShaderLocation(shader, ("lightVP[" + std::to_string(id - 1) + "]").c_str());
    spotInnerLoc = RaylibWrapper::GetShaderLocation(shader, ("spotInnerCutoff[" + std::to_string(id - 1) + "]").c_str());
    spotOuterLoc = RaylibWrapper::GetShaderLocation(shader, ("spotOuterCutoff[" + std::to_string(id - 1) + "]").c_str());
    shadowMapLoc = RaylibWrapper::GetShaderLocation(shader, ("shadowMap[" + std::to_string(id - 1) + "]").c_str());

    RaylibWrapper::SetShaderValue(shader, shadowMapLoc, &id, RaylibWrapper::SHADER_UNIFORM_INT);

    // Set light properties
    lightDir = RaylibWrapper::Vector3Normalize({ 0.35f, -1.0f, -0.35f });
    lightPos = { 0.0f, 0.0f, 0.0f }; // Default position for point/spotlights
    lightType = 0; // 0 = Point, 1 = Spot, 2 = Directional

    RaylibWrapper::Vector4 lightColorNormalized = RaylibWrapper::ColorNormalize({ 255, 255, 255, 0 });

    // Todo: Do I need to set spotInnerLoc and spotOuterLoc here?
    RaylibWrapper::SetShaderValue(shader, lightDirLoc, &lightDir, RaylibWrapper::SHADER_UNIFORM_VEC3);
    RaylibWrapper::SetShaderValue(shader, lightPosLoc, &lightPos, RaylibWrapper::SHADER_UNIFORM_VEC3);
    RaylibWrapper::SetShaderValue(shader, lightColLoc, &lightColorNormalized, RaylibWrapper::SHADER_UNIFORM_VEC4);
    RaylibWrapper::SetShaderValue(shader, lightTypeLoc, &lightType, RaylibWrapper::SHADER_UNIFORM_INT);

    int ambientLoc = RaylibWrapper::GetShaderLocation(shader, "ambient");
    float ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    RaylibWrapper::SetShaderValue(shader, ambientLoc, ambient, RaylibWrapper::SHADER_UNIFORM_VEC4);

    RaylibWrapper::SetShaderValue(shader, RaylibWrapper::GetShaderLocation(shader, "shadowMapResolution"), &width, RaylibWrapper::SHADER_UNIFORM_INT);

    shadowMapTexture = LoadShadowMapRenderTexture();

    //RaylibWrapper::SetShaderValue(shader, shadowMapLoc + (id - 1), &shadowMapTexture, RaylibWrapper::SHADER_UNIFORM_SAMPLER2D); // The example doesnt use this, but shouldnt it be set?

    SetCamera();
}

ShadowManager::~ShadowManager()
{
    if (shadowMapTexture.id > 0)
        RaylibWrapper::rlUnloadFramebuffer(shadowMapTexture.id);

    // rlUnloadFramebuffer should unload the render texture?
    //RaylibWrapper::UnloadRenderTexture(shadowMapTexture);
}

RaylibWrapper::Matrix ShadowManager::RenderShadowPass()
{
    //RaylibWrapper::BeginTextureMode(shadowMapTexture);

    //RaylibWrapper::ClearBackground({ 0, 0, 0, 0 });  // Clear depth buffer

    //RaylibWrapper::BeginMode3D(camera);

    //double rres = 1024; // render resolution, 2048 will cover 4x the area of the 1024 framebuffer at 1/2 the resolution
    //int nearplane = 1, farplane = 10000;
    //RaylibWrapper::rlSetMatrixProjection(RaylibWrapper::MatrixOrtho(-rres / 2, rres / 2, -rres / 2, rres / 2, nearplane, farplane));

    //for (GameObject* gameObject : SceneManager::GetActiveScene()->GetGameObjects())
    //{
    //    if (!gameObject->IsActive()) continue;

    //    for (Component* component : gameObject->GetComponents())
    //    {
    //        if (component->IsActive())
    //            component->RenderShadow();
    //    }
    //}

    //RaylibWrapper::Matrix matLightVP = RaylibWrapper::MatrixMultiply(RaylibWrapper::rlGetMatrixModelview(), RaylibWrapper::rlGetMatrixProjection());

    //RaylibWrapper::EndMode3D();
    //RaylibWrapper::EndTextureMode();

    //return matLightVP;

    return {};
}

RaylibWrapper::RenderTexture2D ShadowManager::LoadShadowMapRenderTexture()
{
    RaylibWrapper::RenderTexture2D target = { 0 };

    target.id = RaylibWrapper::rlLoadFramebuffer();
    target.texture.width = size.x;
    target.texture.height = size.y;

    if (target.id > 0) {
        RaylibWrapper::rlEnableFramebuffer(target.id);

        //target.texture.id = RaylibWrapper::rlLoadTexture(NULL, size.x, size.y, RaylibWrapper::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
        //target.texture.width = size.x;
        //target.texture.height = size.y;
        //target.texture.format = RaylibWrapper::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8; // encode colour as 4 8 bit floats
        //target.texture.mipmaps = 1;

        target.depth.id = RaylibWrapper::rlLoadTextureDepth(size.x, size.y, false);
        target.depth.width = size.x;
        target.depth.height = size.y;
        //target.depth.format = RaylibWrapper::PIXELFORMAT_UNCOMPRESSED_R32; // encode depth as a single 32bit float
        target.depth.format = 19;
        target.depth.mipmaps = 1;

        // bind textures to framebuffer
        //RaylibWrapper::rlFramebufferAttach(target.id, target.texture.id, RaylibWrapper::RL_ATTACHMENT_COLOR_CHANNEL0, RaylibWrapper::RL_ATTACHMENT_TEXTURE2D, 0);
        RaylibWrapper::rlFramebufferAttach(target.id, target.depth.id, RaylibWrapper::RL_ATTACHMENT_DEPTH, RaylibWrapper::RL_ATTACHMENT_TEXTURE2D, 0);

        if (!RaylibWrapper::rlFramebufferComplete(target.id))
        {
            ConsoleLogger::ErrorLog("Framebuffer for shadows failed to be created"); // This will just spam errors
            return target; // Todo: Would this cause errors since I'm not running RaylibWrapper::rlDisableFramebuffer()?
        }


        RaylibWrapper::rlDisableFramebuffer();
    }
    else
        ConsoleLogger::ErrorLog("Framebuffer for shadows failed to be created");

    return target;
}

void ShadowManager::SetCamera() // Todo: Set the correct position and target. take in a parameter
{
    //camera.target = { mainCamera.GetPosition()[0], mainCamera.GetPosition()[1], mainCamera.GetPosition()[2] };
    //camera.position = { mainCamera.GetPosition()[0] + Lighting::main->gameObject->transform.GetPosition().x,
    //    mainCamera.GetPosition()[1] + Lighting::main->gameObject->transform.GetPosition().y,
    //    mainCamera.GetPosition()[2] + Lighting::main->gameObject->transform.GetPosition().z };

    camera.position = RaylibWrapper::Vector3Scale(lightDir, -15.0f); // Todo: This should set the position and target using the camera
    camera.target = {0, 0, 0};

    // Normalize the position
    //Vector3 lightDir = Lighting::main->gameObject->transform.GetPosition();
    //float magnitude = sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z);
    //lightDir.x /= magnitude;
    //lightDir.y /= magnitude;
    //lightDir.z /= magnitude;
    //Vector3 pos = Vector3{ mainCamera.GetPosition()[0], mainCamera.GetPosition()[1], mainCamera.GetPosition()[2] } + (lightDir * 100.0f); // Todo: Shouldn't multily unless its the sun / directional light
    //camera.position = { pos.x, pos.y, pos.z };

    camera.up = {0,1,0};
    camera.projection = RaylibWrapper::CAMERA_ORTHOGRAPHIC; // Todo: Use CAMERA_PERSPECTIVE for point lights and spotlights. Maybe even look into using it for sun. Apparently they recommend using it for the sun since the sun should be far away so perspective is irrelevant
    camera.fovy = 20;
}

void ShadowManager::LoadShaders()
{
#if defined (EDITOR)
    shader = RaylibWrapper::LoadShader("resources/Shaders/glsl330/shadowmap.vs", "resources/Shaders/glsl330/shadowmap.fs");

    materialPreviewShader = RaylibWrapper::LoadShader("resources/Shaders/glsl330/shadowmap.vs", "resources/Shaders/glsl330/shadowmap.fs");
    RaylibModel::SetMaterialPreviewShader(shader.id, shader.locs);
#else
    //shader = RaylibWrapper::LoadShader((path + ".vs").c_str(), (path + ".fs").c_str());
    if (exeParent.empty())
        shader = RaylibWrapper::LoadShader("resources/Shaders/glsl330/shadowmap.vs", "Resources/shaders/glsl330/shadowmap.fs");
    else
        shader = RaylibWrapper::LoadShader((std::filesystem::path(exeParent) / "resources/Shaders/glsl330/shadowmap.vs").string().c_str(), (std::filesystem::path(exeParent) / "resources/Shaders/glsl330/shadowmap.fs").string().c_str());
#endif

    RaylibModel::SetShadowShader(shader.id, shader.locs);
}

void ShadowManager::UnloadShaders()
{
    RaylibWrapper::UnloadShader(shader);
    RaylibWrapper::UnloadShader(materialPreviewShader);
}