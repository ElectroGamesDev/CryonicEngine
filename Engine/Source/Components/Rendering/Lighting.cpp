#include "Lighting.h"
#include "Core/CryonicCore.h"
#include "Raylib/RaylibLightWrapper.h"
#include "Core/GameObject.h"
#if defined(EDITOR)
#include "Core/Editor.h"
#include "Utilities/IconManager.h"
#else
#include "Components/Misc/CameraComponent.h"
#endif

std::deque<Lighting*> Lighting::lights;
int Lighting::nextId = 0;

void Lighting::Awake()
{
	lightId = nextId;
	nextId++;

	shadowManager.Init(lightId);
	shadowManager.lightType = static_cast<int>(type);

	lights.push_back(this);

	needsShaderUpdate = true;
}

void Lighting::Destroy()
{
	auto it = std::find(lights.begin(), lights.end(), this);
	if (it != lights.end())
		lights.erase(it);
}

void Lighting::Enable()
{
	UpdateShaderProperties();
}

void Lighting::Disable()
{
	// Disable light by setting alpha to 0
	RaylibWrapper::Vector4 lightColorNormalized = { 0.0f, 0.0f, 0.0f, 0.0f };
	RaylibWrapper::SetShaderValue(shadowManager.shader, shadowManager.lightColLoc,
		&lightColorNormalized, RaylibWrapper::SHADER_UNIFORM_VEC4);
}

void Lighting::UpdateShaderProperties()
{
	bool isActive = active && gameObject->globalActive && gameObject->active;

	unsigned char alpha = isActive ? 255 : 0;

	// Update color with intensity
	RaylibWrapper::Vector4 lightColorNormalized = RaylibWrapper::ColorNormalize(
		{ color.r, color.g, color.b, alpha }
	);
	lightColorNormalized.x *= intensity;
	lightColorNormalized.y *= intensity;
	lightColorNormalized.z *= intensity;

	RaylibWrapper::SetShaderValue(shadowManager.shader, shadowManager.lightColLoc,
		&lightColorNormalized, RaylibWrapper::SHADER_UNIFORM_VEC4);

	// Update light type
	int typeValue = static_cast<int>(type);
	RaylibWrapper::SetShaderValue(shadowManager.shader, shadowManager.lightTypeLoc,
		&typeValue, RaylibWrapper::SHADER_UNIFORM_INT);
	shadowManager.lightType = typeValue;

	// Update spotlight properties (converted to cosine for shader)
	if (type == Spot)
	{
		float innerCos = cosf(spotInnerAngle * DEG2RAD);
		float outerCos = cosf(spotOuterAngle * DEG2RAD);

		 RaylibWrapper::SetShaderValue(shadowManager.shader, shadowManager.spotInnerLoc, &innerCos, RaylibWrapper::SHADER_UNIFORM_FLOAT);
		 RaylibWrapper::SetShaderValue(shadowManager.shader, shadowManager.spotOuterLoc, &outerCos, RaylibWrapper::SHADER_UNIFORM_FLOAT);
	}

	lastColor = color;
	lastType = type;
	lastIntensity = intensity;
	lastRange = range;
	lastSpotInnerAngle = spotInnerAngle;
	lastSpotOuterAngle = spotOuterAngle;
}

void Lighting::UpdateShadowCamera()
{
	Vector3 position = gameObject->transform.GetPosition();

	if (type == Directional)
	{
		// Directional lights don't use position, only direction
		// Position the shadow camera based on scene center or main camera
#if defined(EDITOR)
		RaylibWrapper::Vector3 focusPoint = Editor::camera.target;
#else
		RaylibWrapper::Vector3 focusPoint = { 0, 0, 0 };

		if (CameraComponent::main)
		{
			Vector3 camPos = CameraComponent::main->gameObject->transform.GetPosition();
			focusPoint = { camPos.x, camPos.y, camPos.z };
		}
#endif

		// Position shadow camera along light direction, away from focus point
		shadowManager.camera.position = RaylibWrapper::Vector3Add(
			{ focusPoint.x, focusPoint.y, focusPoint.z },
			RaylibWrapper::Vector3Scale(shadowManager.lightDir, -50.0f)
		);
		shadowManager.camera.target = { focusPoint.x, focusPoint.y, focusPoint.z };

		// Directional lights use orthographic projection
		shadowManager.camera.projection = RaylibWrapper::CAMERA_ORTHOGRAPHIC;
		shadowManager.camera.fovy = 40.0f; // Ortho size
	}
	else if (type == Point)
	{
		// Point lights need position but not direction offset
		shadowManager.lightPos = { position.x, position.y, position.z };
		shadowManager.camera.position = { position.x, position.y, position.z };

		// Point lights use perspective projection
		shadowManager.camera.projection = RaylibWrapper::CAMERA_PERSPECTIVE;
		shadowManager.camera.fovy = 90.0f;
	}
	else if (type == Spot)
	{
		// Spotlights use both position and direction
		shadowManager.lightPos = { position.x, position.y, position.z };
		shadowManager.camera.position = { position.x, position.y, position.z };

		RaylibWrapper::Vector3 targetPos = RaylibWrapper::Vector3Add(
			{ position.x, position.y, position.z },
			shadowManager.lightDir
		);
		shadowManager.camera.target = targetPos;

		// Spotlight uses perspective with FOV matching outer angle
		shadowManager.camera.projection = RaylibWrapper::CAMERA_PERSPECTIVE;
		shadowManager.camera.fovy = spotOuterAngle * 2.0f;
	}
}

void Lighting::RenderLight(int index)
{
	if (index >= 15) // Maximum 15 lights
		return;

	bool isActive = active && gameObject->globalActive && gameObject->active;
	if (!isActive)
		return;

	// Set view position for shader
#if defined(EDITOR)
	RaylibWrapper::SetShaderValue(ShadowManager::shader,
		ShadowManager::shader.locs[RaylibWrapper::SHADER_LOC_VECTOR_VIEW],
		&Editor::camera.position, RaylibWrapper::SHADER_UNIFORM_VEC3);
#else
	if (!CameraComponent::main)
		return;

	Vector3 camPos = CameraComponent::main->gameObject->transform.GetPosition();
	RaylibWrapper::SetShaderValue(ShadowManager::shader,
		ShadowManager::shader.locs[RaylibWrapper::SHADER_LOC_VECTOR_VIEW],
		&camPos, RaylibWrapper::SHADER_UNIFORM_VEC3);
#endif

	// Check if we need to update shader properties
	if (needsShaderUpdate ||
		color.r != lastColor.r || color.g != lastColor.g ||
		color.b != lastColor.b || color.a != lastColor.a ||
		type != lastType || intensity != lastIntensity ||
		range != lastRange || spotInnerAngle != lastSpotInnerAngle ||
		spotOuterAngle != lastSpotOuterAngle)
	{
		UpdateShaderProperties();
		needsShaderUpdate = false;
	}

	// Update rotation (affects directional and spot lights)
	if (type != Point)
	{
		Quaternion currentRotation = gameObject->transform.GetRotation();
		if (lastRotation != currentRotation)
		{
			Vector3 rotation = gameObject->transform.GetRotationEuler() * DEG2RAD;
			RaylibWrapper::Matrix rotationMatrix = RaylibWrapper::MatrixRotateXYZ(
				{ rotation.x, rotation.y, rotation.z }
			);
			shadowManager.lightDir = RaylibWrapper::Vector3Normalize(
				RaylibWrapper::Vector3Transform({ 0, 0, -1 }, rotationMatrix)
			);
			lastRotation = currentRotation;

			RaylibWrapper::SetShaderValue(ShadowManager::shader, shadowManager.lightDirLoc,
				&shadowManager.lightDir, RaylibWrapper::SHADER_UNIFORM_VEC3);
		}
	}

	// Update position (affects point and spot lights, directional uses it for shadow camera positioning)
	Vector3 currentPosition = gameObject->transform.GetPosition();
	if (lastPosition != currentPosition || type != lastType)
	{
		lastPosition = currentPosition;
		UpdateShadowCamera();

		// Only set light position for point and spot lights
		if (type != Directional)
		{
			RaylibWrapper::SetShaderValue(shadowManager.shader, shadowManager.lightPosLoc,
				&shadowManager.lightPos, RaylibWrapper::SHADER_UNIFORM_VEC3);
		}
	}

	// Render shadows if enabled
	if (!castShadows)
		return;

	// Begin shadow map rendering
	RaylibWrapper::BeginTextureMode(shadowManager.shadowMapTexture);
	RaylibWrapper::ClearBackground({ 255, 255, 255, 255 });
	RaylibWrapper::BeginMode3D(shadowManager.camera);

	RaylibWrapper::Matrix lightView = RaylibWrapper::rlGetMatrixModelview();
	RaylibWrapper::Matrix lightProj = RaylibWrapper::rlGetMatrixProjection();

	// Render scene from light's perspective
	for (GameObject* obj : SceneManager::GetActiveScene()->GetGameObjects())
	{
		if (!obj->IsActive())
			continue;

		// Optional: Frustum culling for performance
		// if (!IsInLightFrustum(obj)) continue;

		for (Component* component : obj->GetComponents())
		{
			if (component->IsActive())
				component->Render(true);
		}
	}

	RaylibWrapper::EndMode3D();
	RaylibWrapper::EndTextureMode();

	// Set light-view-projection matrix
	RaylibWrapper::SetShaderValueMatrix(ShadowManager::shader, shadowManager.lightVPLoc,
		RaylibWrapper::MatrixMultiply(lightView, lightProj));

	// Bind shadow map texture
	RaylibWrapper::rlActiveTextureSlot(lightId);
	RaylibWrapper::rlEnableTexture(shadowManager.shadowMapTexture.depth.id);

	RaylibWrapper::SetShaderValue(shadowManager.shader, shadowManager.shadowMapLoc,
		&lightId, RaylibWrapper::SHADER_UNIFORM_INT);
}

#if defined(EDITOR)
void Lighting::EditorUpdate()
{
	bool isNowActive = active && gameObject->globalActive && gameObject->active;

	// Handle enable/disable
	if (isNowActive != wasLastActive)
	{
		if (isNowActive)
			Enable();
		else
			Disable();

		wasLastActive = isNowActive;
	}

	// Update color
	if (color.r != exposedVariables[1][0][2][0].get<int>() ||
		color.g != exposedVariables[1][0][2][1].get<int>() ||
		color.b != exposedVariables[1][0][2][2].get<int>() ||
		color.a != exposedVariables[1][0][2][3].get<int>())
	{
		color.r = exposedVariables[1][0][2][0].get<int>();
		color.g = exposedVariables[1][0][2][1].get<int>();
		color.b = exposedVariables[1][0][2][2].get<int>();
		color.a = exposedVariables[1][0][2][3].get<int>();
		needsShaderUpdate = true;
	}

	// Update type
	std::string currentTypeString = exposedVariables[1][1][2].get<std::string>();
	Type newType = type;

	if (currentTypeString == "Point")
		newType = Point;
	else if (currentTypeString == "Spot")
		newType = Spot;
	else if (currentTypeString == "Directional")
		newType = Directional;

	if (newType != type)
	{
		type = newType;
		needsShaderUpdate = true;
	}

	// Update intensity
	float newIntensity = exposedVariables[1][2][2].get<float>();
	if (abs(newIntensity - intensity) > 0.001f)
	{
		intensity = newIntensity;
		needsShaderUpdate = true;
	}

	// Update range
	float newRange = exposedVariables[1][3][2].get<float>();
	if (abs(newRange - range) > 0.001f)
	{
		range = newRange;
		needsShaderUpdate = true;
	}

	// Update spotlight angles
	if (type == Spot)
	{
		float newInnerAngle = exposedVariables[1][4][2].get<float>();
		float newOuterAngle = exposedVariables[1][5][2].get<float>();

		if (abs(newInnerAngle - spotInnerAngle) > 0.001f ||
			abs(newOuterAngle - spotOuterAngle) > 0.001f)
		{
			spotInnerAngle = newInnerAngle;
			spotOuterAngle = newOuterAngle;
			needsShaderUpdate = true;
		}
	}

	// Update cast shadows
	castShadows = exposedVariables[1][6][2].get<bool>();

	// Draw light gizmo in editor
	RaylibWrapper::Draw3DBillboard(Editor::camera,
		*IconManager::imageTextures["LightGizmoIcon"],
		{ gameObject->transform.GetPosition().x,
		 gameObject->transform.GetPosition().y,
		 gameObject->transform.GetPosition().z },
		0.5f, { 255, 255, 255, 150 });
}
#endif