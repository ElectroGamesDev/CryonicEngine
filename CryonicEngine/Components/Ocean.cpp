#include "Ocean.h"
#if defined(EDITOR)
#include "../ProjectManager.h"
#include "../Editor.h"
#else
#include "../Game.h"
#endif

void Ocean::Awake()
{
	// Create or get water material
	if (!waterMaterial)
	{
		waterMaterial = new Material("WaterDefault");
		ownMaterial = true;

		waterMaterial->SetAlbedoColor({ 13, 3, 77, 255 }); // Base dark blue, overridden by shader

		std::pair<unsigned int, int*> waterShaderPair = ShaderManager::GetShader(ShaderManager::Shaders::Water);
		waterMaterial->SetShader({ waterShaderPair.first, waterShaderPair.second });
	}
}

void Ocean::Start()
{
	modelSet = raylibModel.Create(ModelType::Plane, "Plane", ShaderManager::Shaders::Water, "", { planeSize, static_cast<float>(planeRes) });
	raylibModel.SetMaterials({ waterMaterial->GetRaylibMaterial() });
}

void Ocean::Update()
{
	if (infiniteOcean)
	{
		// Todo: This will have issues if the main camera is changed. Use events to know if the main camera changes
		Vector3 pos;
#if defined(EDITOR)
		pos = { Editor::camera.position.x, gameObject->transform.GetPosition().y, Editor::camera.position.z };
#else
		if (!mainCamera)
			mainCamera = CameraComponent::main;

		if (!mainCamera)
			pos = { gameObject->transform.GetPosition().x, gameObject->transform.GetPosition().y, gameObject->transform.GetPosition().z };
		else
			pos = { mainCamera->GetGameObject()->GetPosition().x, gameObject->transform.GetPosition().y, mainCamera->GetGameObject()->GetPosition().z };
#endif

		gameObject->transform.SetPosition(pos);
	}
}

void Ocean::Render(bool renderShadows)
{
	if (!modelSet || renderShadows)
		return;

	// Enable alpha blending for transparency (see below water)
	RaylibWrapper::rlSetBlendMode(RaylibWrapper::RL_BLEND_ALPHA_PREMULTIPLY);  // Use raylib's SetBlendMode for transparency to see below

	// Set custom shader uniforms
	RaylibWrapper::Shader shader = waterMaterial->GetRaylibMaterial()->shader;
	float timeVal = RaylibWrapper::GetTime();

	// Time uniform
	int locTime = RaylibWrapper::GetShaderLocation(shader, "time");
	RaylibWrapper::SetShaderValue(shader, locTime, &timeVal, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	// Wave params for wave 1
	int locAmp1 = RaylibWrapper::GetShaderLocation(shader, "waveAmp1");
	RaylibWrapper::SetShaderValue(shader, locAmp1, &waveAmp1, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locLambda1 = RaylibWrapper::GetShaderLocation(shader, "waveLambda1");
	RaylibWrapper::SetShaderValue(shader, locLambda1, &waveLambda1, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locSpeed1 = RaylibWrapper::GetShaderLocation(shader, "waveSpeed1");
	RaylibWrapper::SetShaderValue(shader, locSpeed1, &waveSpeed1, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	float dir1Arr[2] = { waveDir1.x, waveDir1.y };
	int locDir1 = RaylibWrapper::GetShaderLocation(shader, "waveDir1");
	RaylibWrapper::SetShaderValue(shader, locDir1, dir1Arr, RaylibWrapper::SHADER_UNIFORM_VEC2);

	// Wave params for wave 2
	int locAmp2 = RaylibWrapper::GetShaderLocation(shader, "waveAmp2");
	RaylibWrapper::SetShaderValue(shader, locAmp2, &waveAmp2, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locLambda2 = RaylibWrapper::GetShaderLocation(shader, "waveLambda2");
	RaylibWrapper::SetShaderValue(shader, locLambda2, &waveLambda2, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locSpeed2 = RaylibWrapper::GetShaderLocation(shader, "waveSpeed2");
	RaylibWrapper::SetShaderValue(shader, locSpeed2, &waveSpeed2, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	float dir2Arr[2] = { waveDir2.x, waveDir2.y };
	int locDir2 = RaylibWrapper::GetShaderLocation(shader, "waveDir2");
	RaylibWrapper::SetShaderValue(shader, locDir2, dir2Arr, RaylibWrapper::SHADER_UNIFORM_VEC2);

	// Wave params for wave 3
	int locAmp3 = RaylibWrapper::GetShaderLocation(shader, "waveAmp3");
	RaylibWrapper::SetShaderValue(shader, locAmp3, &waveAmp3, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locLambda3 = RaylibWrapper::GetShaderLocation(shader, "waveLambda3");
	RaylibWrapper::SetShaderValue(shader, locLambda3, &waveLambda3, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locSpeed3 = RaylibWrapper::GetShaderLocation(shader, "waveSpeed3");
	RaylibWrapper::SetShaderValue(shader, locSpeed3, &waveSpeed3, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	float dir3Arr[2] = { waveDir3.x, waveDir3.y };
	int locDir3 = RaylibWrapper::GetShaderLocation(shader, "waveDir3");
	RaylibWrapper::SetShaderValue(shader, locDir3, dir3Arr, RaylibWrapper::SHADER_UNIFORM_VEC2);

	// Wave params for wave 4
	int locAmp4 = RaylibWrapper::GetShaderLocation(shader, "waveAmp4");
	RaylibWrapper::SetShaderValue(shader, locAmp4, &waveAmp4, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locLambda4 = RaylibWrapper::GetShaderLocation(shader, "waveLambda4");
	RaylibWrapper::SetShaderValue(shader, locLambda4, &waveLambda4, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locSpeed4 = RaylibWrapper::GetShaderLocation(shader, "waveSpeed4");
	RaylibWrapper::SetShaderValue(shader, locSpeed4, &waveSpeed4, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	float dir4Arr[2] = { waveDir4.x, waveDir4.y };
	int locDir4 = RaylibWrapper::GetShaderLocation(shader, "waveDir4");
	RaylibWrapper::SetShaderValue(shader, locDir4, dir4Arr, RaylibWrapper::SHADER_UNIFORM_VEC2);

	// Steepness
	int locSteepness = RaylibWrapper::GetShaderLocation(shader, "steepness");
	RaylibWrapper::SetShaderValue(shader, locSteepness, &steepness, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locWaterColor = RaylibWrapper::GetShaderLocation(shader, "waterColor");
	RaylibWrapper::SetShaderValue(shader, locWaterColor, &waterColor, RaylibWrapper::SHADER_UNIFORM_VEC4);

	// Foam and transparency
	int locFoam = RaylibWrapper::GetShaderLocation(shader, "foamThreshold");
	RaylibWrapper::SetShaderValue(shader, locFoam, &foamThreshold, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locTransparency = RaylibWrapper::GetShaderLocation(shader, "waterTransparency");
	RaylibWrapper::SetShaderValue(shader, locTransparency, &waterTransparency, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	// Ripples:
	int locRippleScale = RaylibWrapper::GetShaderLocation(shader, "rippleScale");
	if (locRippleScale != -1)
		RaylibWrapper::SetShaderValue(shader, locRippleScale, &rippleScale, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	int locRippleAmp = RaylibWrapper::GetShaderLocation(shader, "rippleAmp");
	if (locRippleAmp != -1)
		RaylibWrapper::SetShaderValue(shader, locRippleAmp, &rippleAmp, RaylibWrapper::SHADER_UNIFORM_FLOAT);

	// Set colDiffuse (tinted version of waterColor for base material - required for shader)
	Vector4 colDiffuseVal = { waterColor.x * 0.5f, waterColor.y * 0.5f, waterColor.z * 1.2f, 1.0f };
	int locColDiffuse = RaylibWrapper::GetShaderLocation(shader, "colDiffuse");
	if (locColDiffuse != -1) RaylibWrapper::SetShaderValue(shader, locColDiffuse, &colDiffuseVal, RaylibWrapper::SHADER_UNIFORM_VEC4);

	// Render using MeshRenderer logic, but without shadows
	Vector3 position = gameObject->transform.GetPosition();
	Quaternion rotation = gameObject->transform.GetRotation();
	Vector3 scale = gameObject->transform.GetScale();

	// Material handling
	// Todo: This material is being set each frame
	if ((!waterMaterial && raylibModel.GetMaterialID(0) > 1) || (waterMaterial && !raylibModel.CompareMaterials({ waterMaterial->GetID() })))
	{
		if (!waterMaterial)
		{
			Awake(); // Todo: This needs to have its own function
		}
		else
			raylibModel.SetMaterials({ waterMaterial->GetRaylibMaterial() });
	}

	// Draw
	raylibModel.DrawModelWrapper(position.x, position.y, position.z,
		1, 1, 1,
		rotation.x, rotation.y, rotation.z, rotation.w,
		255, 255, 255, 255);

	// Reset blend mode after drawing
	RaylibWrapper::rlSetBlendMode(RaylibWrapper::RL_BLEND_ALPHA);
}

#if defined(EDITOR)
void Ocean::EditorUpdate()
{
	if (!editorSetup)
	{
		Awake();
		Start(); // Todo: The start code should be in Awake(), but currently I'm calling Awake() when setting materials
		editorSetup = true;
	}

	// Update exposed variables
	infiniteOcean = exposedVariables[1][1][2].get<bool>();
	waveAmp1 = exposedVariables[1][2][2].get<float>();
	waveLambda1 = exposedVariables[1][3][2].get<float>();
	waveSpeed1 = exposedVariables[1][4][2].get<float>();
	waveDir1 = { exposedVariables[1][5][2][0].get<float>(), exposedVariables[1][5][2][1].get<float>() };
	waveAmp2 = exposedVariables[1][6][2].get<float>();
	waveLambda2 = exposedVariables[1][7][2].get<float>();
	waveSpeed2 = exposedVariables[1][8][2].get<float>();
	waveDir2 = { exposedVariables[1][9][2][0].get<float>(), exposedVariables[1][9][2][1].get<float>() };
	waveAmp3 = exposedVariables[1][10][2].get<float>();
	waveLambda3 = exposedVariables[1][11][2].get<float>();
	waveSpeed3 = exposedVariables[1][12][2].get<float>();
	waveDir3 = { exposedVariables[1][13][2][0].get<float>(), exposedVariables[1][13][2][1].get<float>() };
	waveAmp4 = exposedVariables[1][14][2].get<float>();
	waveLambda4 = exposedVariables[1][15][2].get<float>();
	waveSpeed4 = exposedVariables[1][16][2].get<float>();
	waveDir4 = { exposedVariables[1][17][2][0].get<float>(), exposedVariables[1][17][2][1].get<float>() };
	steepness = exposedVariables[1][18][2].get<float>();
	planeSize = exposedVariables[1][19][2].get<float>();
	planeRes = exposedVariables[1][20][2].get<int>();
	waterColor = { exposedVariables[1][21][2][0].get<float>(),
				   exposedVariables[1][21][2][1].get<float>(),
				   exposedVariables[1][21][2][2].get<float>(),
				   exposedVariables[1][21][2][3].get<float>() };
	foamThreshold = exposedVariables[1][22][2].get<float>();
	waterTransparency = exposedVariables[1][23][2].get<float>();

	// Material handling
	if (exposedVariables[1][0][2] == "nullptr")
		exposedVariables[1][0][2] = "WaterDefault";

	if ((!waterMaterial && !(exposedVariables[1][0][2] == "WaterDefault")) || (waterMaterial && waterMaterial->GetPath() != exposedVariables[1][0][2]))
	{
		if (exposedVariables[1][0][2] == "WaterDefault")
		{
			// Todo: Make a function to call instead of copy pasting the same code

			if (waterMaterial && ownMaterial)
				delete waterMaterial;

			waterMaterial = new Material("WaterDefault");
			ownMaterial = true;

			waterMaterial->SetAlbedoColor({ 13, 3, 77, 255 }); // Todo: Make this color exposed

			std::pair<unsigned int, int*> waterShaderPair = ShaderManager::GetShader(ShaderManager::Shaders::Water);
			waterMaterial->SetShader({ waterShaderPair.first, waterShaderPair.second });
		}
		else
			waterMaterial = Material::GetMaterial(exposedVariables[1][0][2]);
	}

	// If variables changed, regenerate mesh if in editor
	if (modelSet && (exposedVariables[1][19][2] != planeSize || exposedVariables[1][20][2] != planeRes))
	{
		planeSize = exposedVariables[1][19][2].get<float>();
		planeRes = exposedVariables[1][20][2].get<int>();

		// Regenerate mesh
		Start(); // It would be best to have a function dedicated to mesh generation
	}
}
#endif

void Ocean::Destroy()
{
	if (modelSet)
		raylibModel.DeleteInstance();

	if (waterMaterial && waterMaterial->GetPath() == "WaterDefault")
		delete waterMaterial;
}