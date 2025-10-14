#include "Game.h"
#include <iostream>
#include "ConsoleLogger.h"
#include "Scenes/SceneManager.h"
#include "Components/Component.h"
#include "Components/CameraComponent.h"
#include "Components/SpriteRenderer.h"
#include "Components/Lighting.h"
#include "Components/Skybox.h"
#include "Components/Clouds.h"
#include "Components/Ocean.h"
#include "AudioClip.h"
#include "ShaderManager.h"
#include "RaylibModelWrapper.h"
#include "RaylibWrapper.h"
#include "RenderableTexture.h"
#include "ShadowManager.h"
#include "Material.h"
#include "MenuManager.h"
#ifdef WINDOWS
// Prevent Windows from defining conflicting functions
#define NOGDI
#define NOUSER
#include <windows.h>
#include "WindowsHelper.h"
#include "AsyncPBOCapture.h"
#elif WEB
#include <emscripten/emscripten.h>
#include <unordered_set>
#include "InputSystem.h"
#endif
#include "imgui_internal.h"
#include "FontManager.h"
#include <vector>

#include "CollisionListener2D.h"
#include "Physics2DDebugDraw.h"

#ifdef IS3D
#define JPH_DEBUG_RENDERER
#include <thread>
#include "Jolt/Jolt.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Renderer/DebugRenderer.h"
#include "Jolt/Physics/Body/BodyManager.h"
#include "Components/Rigidbody3D.h"
#include "Physics3DDebugDraw.h"
#include "CollisionListener3D.h"
JPH_SUPPRESS_WARNINGS

JPH::PhysicsSystem physicsSystem;
CollisionListener3D collisionListener3D;
JPH::TempAllocatorMalloc* tempAllocator;
Physics3DDebugDraw* debugRenderer;
JPH::JobSystemThreadPool jobSystem;
JPH::BodyManager::DrawSettings bodyDrawSettings;
#endif

b2World* world = nullptr;
CollisionListener2D collisionListener;

std::filesystem::path exeParent;

bool isPlayMode = false;
RaylibWrapper::RenderTexture playModeRenderTexture;
#ifdef WINDOWS
AsyncPBOCapture pboCapture;
#endif

// Todo: Get this from project settings
// These are global so the MainLoop() can access them
float timeStep = 1.0f / 60.0f;
int32 velocityIterations = 8; // For 2D physics
int32 positionIterations = 3; // For 2D physics
int physicsIterations = 5; // For 3D physics
float timeSinceLastUpdate = 0.0f;

#ifdef IS3D
/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
	virtual bool					ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
	{
		switch (inObject1)
		{
		case Layers::NON_MOVING:
			return inObject2 == Layers::MOVING; // Non moving only collides with moving
		case Layers::MOVING:
			return true; // Moving collides with everything
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
namespace BroadPhaseLayers
{
	static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
	static constexpr JPH::BroadPhaseLayer MOVING(1);
	static constexpr unsigned int NUM_LAYERS(2);
};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	virtual unsigned int					GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual JPH::BroadPhaseLayer			GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
	{
		switch ((JPH::BroadPhaseLayer::Type)inLayer)
		{
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
		default:													JPH_ASSERT(false); return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	JPH::BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool				ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
	{
		switch (inLayer1)
		{
		case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};
#endif

void MainLoop();

int main(int argc, char* argv[])
{
	// TODO: Init() shadow manager, and make sure to pass in parameters
	// Todo: and UnloadShader()
	// Make sure to check if its 3d before rendering, init, and cleanup shadows

	// Todo: Is this even running? There was an error in the code and it didn't give off any errors. I might have fixed it with the ninja command by using "-DCMAKE_CXX_FLAGS=-DWINDOWS "?
#ifdef WINDOWS
	// Sets executable path to a variable
	HMODULE hModule = GetModuleHandle(NULL);
	if (hModule != NULL)
	{
		char exePath[MAX_PATH];
		GetModuleFileNameA(hModule, exePath, (sizeof(exePath)));
		std::string temp = exePath;
		exeParent = temp;
		exeParent = exeParent.parent_path();
	}
	else
	{
		ConsoleLogger::WarningLog("Failed to get module handle. There may be game-breaking issues.", false);
		exeParent = std::filesystem::current_path();
	}
#endif

	// Creates the window
	RaylibWrapper::SetConfigFlags(0);
	RaylibWrapper::InitWindow(RaylibWrapper::GetScreenWidth(), RaylibWrapper::GetScreenHeight(), (NAME));
	//RaylibWrapper::ToggleFullscreen();
	//RaylibWrapper::ToggleBorderlessWindowed();
	//if (RaylibWrapper::GetScreenWidth() == RaylibWrapper::GetMonitorWidth(RaylibWrapper::GetCurrentMonitor()) && RaylibWrapper::GetScreenHeight() == RaylibWrapper::GetMonitorHeight(RaylibWrapper::GetCurrentMonitor())) RaylibWrapper::MaximizeWindow();
	RaylibWrapper::SetWindowMinSize(100, 100);
	RaylibWrapper::RaylibWrapper::SetTargetFPS(60);
	RaylibWrapper::SetExitKey(0);
	
	RaylibWrapper::InitAudioDevice();

	// ImGui Setup
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enabled Multi-Viewports
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	ImGui::StyleColorsDark();
	RaylibWrapper::ImGui_ImplRaylib_Init();

	// Setup playmode
	if (argc > 1) {
		for (int i = 1; i < argc; ++i) {
			if (std::strcmp(argv[i], "playmode") == 0)
			{
				isPlayMode = true;
				playModeRenderTexture = RaylibWrapper::LoadRenderTexture(RaylibWrapper::GetScreenWidth(), RaylibWrapper::GetScreenHeight());

#ifdef WINDOWS
				pboCapture.Init(RaylibWrapper::GetScreenWidth(), RaylibWrapper::GetScreenHeight());
				pboCapture.Start();
				WindowsHelper::HideWindow();
#endif
			}
		}
	}

	FontManager::InitFontManager();

	// Physics setup. Must go before scene loading
#ifdef IS3D
	JPH::RegisterDefaultAllocator();
	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterTypes();

	const unsigned int cMaxBodies = 65536;
	const unsigned int cNumBodyMutexes = 0;
	const unsigned int cMaxBodyPairs = 65536;
	const unsigned int cMaxContactConstraints = 20480;

	BPLayerInterfaceImpl broadPhaseLayerInterface;
	ObjectVsBroadPhaseLayerFilterImpl objectVsBroadphaseLayerFilter;
	ObjectLayerPairFilterImpl objectVsObjectLayerFilter;

	physicsSystem.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broadPhaseLayerInterface, objectVsBroadphaseLayerFilter, objectVsObjectLayerFilter);

	//MyBodyActivationListener bodyActivationListener;
	//physicsSystem.SetBodyActivationListener(&body_activationListener);

	physicsSystem.SetContactListener(&collisionListener3D);

	JPH::BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();
	Rigidbody3D::bodyInterface = &physicsSystem.GetBodyInterface();

	Rigidbody3D::bodyLockInterface = &physicsSystem.GetBodyLockInterface();

	debugRenderer = new Physics3DDebugDraw();
	//JPH::DebugRenderer::sInstance = debugRenderer;
	bodyDrawSettings.mDrawGetSupportFunction = true;
	bodyDrawSettings.mDrawShape = true;
	bodyDrawSettings.mDrawShapeWireframe = true;
	bodyDrawSettings.mDrawBoundingBox = true;
	jobSystem.Init(2048, 16, std::thread::hardware_concurrency());
	//JPH::TempAllocatorImpl tempAllocator(100 * 1024 * 1024);
	tempAllocator = new JPH::TempAllocatorMalloc();
#endif

	b2Vec2 gravity(0.0f, -9.8f);
	world = new b2World(gravity);
	world->SetContactListener(&collisionListener);
	//Physics2DDebugDraw debugDraw;
	//debugDraw.SetFlags(b2Draw::e_shapeBit);
	//world->SetDebugDraw(&debugDraw);

	// Shaders must be initiated before scenes/gameobjects
	ShaderManager::Init();
	ShadowManager::LoadShaders();

	Material::LoadWhiteTexture();
	Material::LoadDefaultMaterial();
	MenuManager::Init();

	// Todo: This assumes the default scene path and name
	if (exeParent.empty())
		SceneManager::LoadScene("Resources/Assets/Scenes/Default.scene");
	else
		SceneManager::LoadScene(exeParent / "Resources" / "Assets" / "Scenes" / "Default.scene");
	SceneManager::SetActiveScene(&SceneManager::GetScenes()->back());

#ifdef WEB
	emscripten_set_main_loop(MainLoop, 60, 1);
#else
	while (!RaylibWrapper::WindowShouldClose())
		MainLoop();
#endif

#ifdef WINDOWS
	pboCapture.Stop();
	RaylibWrapper::UnloadRenderTexture(playModeRenderTexture);
#endif

	// Todo: There may be other scenes loaded. Make sure to also unload them.

	SceneManager::UnloadScene(SceneManager::GetActiveScene());

	// Cleanup sounds (AudioPlayer cleans up streamed sounds)
	for (auto& pair : AudioClip::sounds)
		Raylib::UnloadSound(pair.second);

	RaylibWrapper::CloseAudioDevice();
	RaylibWrapper::ImGui_ImplRaylib_Shutdown();
	ImGui::DestroyContext();

	Material::UnloadWhiteTexture();
	Material::UnloadDefaultMaterial();

	ShaderManager::Cleanup();
	ShadowManager::UnloadShaders();
    RaylibWrapper::CloseWindow();
#ifdef IS3D
	delete debugRenderer;
	delete tempAllocator;
	JPH::UnregisterTypes();
	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;
#else
	delete world;
#endif
	return 0;
}

void MainLoop()
{
	timeSinceLastUpdate += RaylibWrapper::GetFrameTime();
	while (timeSinceLastUpdate >= timeStep)
	{
#ifdef IS3D
		physicsSystem.Update(timeStep, physicsIterations, tempAllocator, &jobSystem);
#else
		world->Step(timeStep, velocityIterations, positionIterations);
		collisionListener.ContinueContact(); // Todo: Should this go after the loop?
#endif
		timeSinceLastUpdate -= timeStep;

		fixedDeltaTime = timeStep;

		for (GameObject* gameObject : SceneManager::GetActiveScene()->GetGameObjects())
		{
			if (!gameObject->IsActive() || !gameObject->IsGlobalActive())
				continue;
			for (Component* component : gameObject->GetComponents())
			{
				if (!component->IsActive())
					continue;

				component->FixedUpdate();
				fixedDeltaTime = timeStep; // Setting this here and before the loop incase if a component changes the fixed delta time
			}
		}
	}

	// GUI
	FontManager::UpdateFonts();

	RaylibWrapper::ImGui_ImplRaylib_ProcessEvents();
	RaylibWrapper::ImGui_ImplRaylib_NewFrame();
	ImGui::NewFrame();

	if (CameraComponent::main != nullptr)
		ShaderManager::UpdateShaders(CameraComponent::main->gameObject->transform.GetPosition().x, CameraComponent::main->gameObject->transform.GetPosition().y, CameraComponent::main->gameObject->transform.GetPosition().z);

	// This is used because mouse inputs don't work on web if the input happens between BeginDrawing() and EndDrawing(). Edit: This has been commented out because it appears to work without it, and the Raylib wiki may be out of date
//#ifdef WEB
//	Mouse::buttonsPressed.clear();
//	Mouse::buttonsReleased.clear();
//	Mouse::buttonsDown.clear();
//
//	for (int button = 0; button <= 6; button++)
//	{
//		MouseButton mouseButton = static_cast<MouseButton>(button);
//
//		if (RaylibWrapper::IsMouseButtonDown(static_cast<RaylibWrapper::MouseButton>(button)))
//			Mouse::buttonsDown.insert(mouseButton);
//
//		if (RaylibWrapper::IsMouseButtonPressed(static_cast<RaylibWrapper::MouseButton>(button)))
//			Mouse::buttonsPressed.insert(mouseButton);
//
//		if (RaylibWrapper::IsMouseButtonReleased(static_cast<RaylibWrapper::MouseButton>(button)))
//			Mouse::buttonsReleased.insert(mouseButton);
//	}
//#endif

	// Shadows
	//if (CameraComponent::main != nullptr)
	//{
	//	Vector3 mainCameraPos = CameraComponent::main->gameObject->transform.GetPosition();
	//	shadowManager.camera.position = { mainCameraPos.x + Lighting::main->gameObject->transform.GetPosition().x,
	//		mainCameraPos.y + Lighting::main->gameObject->transform.GetPosition().y,
	//		mainCameraPos.z + Lighting::main->gameObject->transform.GetPosition().z };
	//}
	//RaylibWrapper::Matrix matLightVP = shadowManager.RenderShadowPass();

	if (!isPlayMode)
		RaylibWrapper::BeginDrawing();


#ifdef IS3D
	RaylibWrapper::rlEnableShader(ShadowManager::shader.id);

	int index = 1;
	for (Lighting* light : Lighting::lights)
	{
		if (light->IsActive() && light->gameObject->IsGlobalActive() && light->gameObject->IsActive())
		{
			light->RenderLight(index);
			index++;
		}
	}
#endif

	if (isPlayMode) // Must go after RenderLight
		RaylibWrapper::BeginTextureMode(playModeRenderTexture);

#ifdef IS3D
	RaylibWrapper::ClearBackground({ 135, 206, 235, 255 });
#else
	RaylibWrapper::ClearBackground({ 128, 128, 128, 255 });
#endif

	CameraComponent::main->raylibCamera.BeginMode3D();

	// Shadows
	//RaylibWrapper::rlEnableShader(shadowManager.shader.id);
	//RaylibWrapper::rlSetUniformMatrix(RaylibWrapper::GetShaderLocation(shadowManager.shader, "matLightVP"), matLightVP);
	//RaylibWrapper::SetShaderValueTexture(shadowManager.shader, RaylibWrapper::GetShaderLocation(shadowManager.shader, "texture_shadowmap"), shadowManager.shadowMapTexture.texture);
	// or RaylibWrapper::rlSetUniformSampler(GetShaderLocation(shader_shadowmap, "texture_shadowmap"), shadowmap.texture.id);
	// RaylibWrapper::rlActiveTextureSlot(1);
	// RaylibWrapper::rlEnableTexture(shadowManager.shadowMapTexture.id);

	// Update CollisionSystem

	deltaTime = RaylibWrapper::GetFrameTime();
	float tempDelaTime = deltaTime;

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(RaylibWrapper::GetScreenWidth(), RaylibWrapper::GetScreenHeight()));
	ImGui::Begin("##Game", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

	// Call components Update()

	// Skyboxes must be rendered first
	Skybox::RenderSkyboxes();

	GameObject::markForDeletion = true;
	auto& gameObjects = SceneManager::GetActiveScene()->GetGameObjects();
	for (size_t i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* gameObject = gameObjects[i];

		if (!gameObject->IsActive() || !gameObject->IsGlobalActive())
			continue;

		for (Component* component : gameObject->GetComponents())
		{
			if (!component->IsActive())
				continue;

			component->Update();
			deltaTime = tempDelaTime; // Setting this here and before the loop incase if a component changes the delta time

			if (gameObject && component) // Component/Gameobject may get deleted in the Update(). I could also check if the gameobject and component is still active here although most likely useless overhead
				component->RenderGui();

#ifdef IS3D
			component->Render();
#endif
		}
	}
	GameObject::markForDeletion = false;

	for (GameObject* gameObject : GameObject::markedForDeletion)
	{
		if (gameObject)
			SceneManager::GetActiveScene()->RemoveGameObject(gameObject);
	}
	GameObject::markedForDeletion.clear();

	for (Component* component : Component::markedForDeletion)
		if (component && component->gameObject)
			component->gameObject->RemoveComponent(component);
	Component::markedForDeletion.clear();

	for (RenderableTexture* texture : RenderableTexture::textures) // Renders Sprites and Tilemaps
		if (texture)
			texture->Render();

	// Clouds must be rendered after opaque geometry, but before transparent ones
	Clouds::RenderClouds();

	// Water must be rendered last
	Ocean::RenderOceans();

	ImGui::End();

#ifdef IS2D
	//world->DebugDraw();
#else
	if (CameraComponent::main != nullptr)
		debugRenderer->SetCameraPos(JPH::RVec3(
			CameraComponent::main->gameObject->transform.GetPosition().x,
			CameraComponent::main->gameObject->transform.GetPosition().y,
			CameraComponent::main->gameObject->transform.GetPosition().z
		));

	//physicsSystem.DrawBodies(bodyDrawSettings, debugRenderer);
#endif

#ifdef IS3D
	RaylibWrapper::rlDisableShader();
	RaylibWrapper::rlActiveTextureSlot(0);
#endif

	RaylibWrapper::EndMode3D();

	// Render GUI
	ImGui::Render();
	RaylibWrapper::ImGui_ImplRaylib_RenderDrawData(ImGui::GetDrawData());

	if (isPlayMode)
	{
		RaylibWrapper::EndTextureMode();
#ifdef WINDOWS
		pboCapture.AsyncReadback(playModeRenderTexture);
#endif

		// Todo: Copy render texture to shared memory

		// Draws the render texture to the screen
		// For some reason I still need to render this else it doesn't render to the Editor's Game view. I even tried scaling it down and making it invisible, but the Editor's game view copies it.
		RaylibWrapper::BeginDrawing();
		//RaylibWrapper::ClearBackground(RaylibWrapper::BLACK);

		RaylibWrapper::Rectangle sourceRec = { 0, (float)playModeRenderTexture.texture.height, (float)playModeRenderTexture.texture.width, -(float)playModeRenderTexture.texture.height };
		RaylibWrapper::Rectangle destRec = { 0, 0, (float)RaylibWrapper::GetScreenWidth(), (float)RaylibWrapper::GetScreenHeight() };
		RaylibWrapper::DrawTexturePro(playModeRenderTexture.texture, sourceRec, destRec, { 0,0 }, 0.0f, RaylibWrapper::WHITE);

		RaylibWrapper::EndDrawing();
	}
	else
		RaylibWrapper::EndDrawing();
}