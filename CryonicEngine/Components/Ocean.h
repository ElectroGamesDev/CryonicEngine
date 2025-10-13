#pragma once

#include "Component.h"
#include "../RaylibWrapper.h"
#include "../RaylibModelWrapper.h"
#include "../Material.h"
#include "../ShaderManager.h"
#include <filesystem>
#include "CameraComponent.h"

class Ocean : public Component
{
public:
	Ocean(GameObject* obj, int id) : Component(obj, id)
	{
		runInEditor = true;
		name = "Ocean";
		iconUnicode = "\xef\x9d\xb3";

#if defined(EDITOR)
		std::string variables = R"(
        [
            0,
            [
                [
                    "Material",
                    "material",
                    "WaterDefault",
                    "Material",
                    {
                        "Extensions": [".mat"]
                    }
                ],
                [
                    "bool",
                    "infiniteOcean",
                    false,
                    "Infinite Ocean"
				],
				[
					"float",
					"waveAmp1",
					0.5,
					"Wave Amplitude 1",
					{ "min": 0.0, "max" : 5.0 }
				],
				[
					"float",
					"waveLambda1",
					15.0,
					"Wave Length 1",
					{ "min": 1.0, "max" : 100.0 }
				],
				[
					"float",
					"waveSpeed1",
					1.0,
					"Wave Speed 1",
					{ "min": 0.0, "max" : 10.0 }
				],
				[
					"Vector2",
					"waveDir1",
					[1.0, 0.6],
					"Wave Direction 1"
				],
				[
					"float",
					"waveAmp2",
					0.25,
					"Wave Amplitude 2",
					{ "min": 0.0, "max" : 5.0 }
				],
				[
					"float",
					"waveLambda2",
					8.0,
					"Wave Length 2",
					{ "min": 1.0, "max" : 100.0 }
				],
				[
					"float",
					"waveSpeed2",
					1.2,
					"Wave Speed 2",
					{ "min": 0.0, "max" : 10.0 }
				],
				[
					"Vector2",
					"waveDir2",
					[0.8, -0.6],
					"Wave Direction 2"
				],
				[
					"float",
					"planeSize",
					2000.0,
					"Plane Size",
					{ "min": 100.0, "max" : 10000.0 }
				],
				[
					"int",
					"planeRes",
					64,
					"Plane Resolution",
					{ "min": 10, "max" : 256 }
				],
				[
					"Vector4",
					"waterColor",
					[0.0, 0.1, 0.3, 1.0],
					"Water Color"
				]
			]
		]
		)";
		exposedVariables = nlohmann::json::parse(variables);
#endif
	}

	Ocean* Clone() override
	{
		return new Ocean(gameObject, -1);
	}

	void Awake() override;
	void Start() override;
	void Update() override;
	void Render(bool renderShadows) override;
#if defined(EDITOR)
	void EditorUpdate() override;
#endif
	void Destroy() override;

	float GetWaveAmp1() const { return waveAmp1; }
	void SetWaveAmp1(float amp) { waveAmp1 = amp; }
	// Todo: Add other functions for wave parameters

	bool IsInfinite() const { return infiniteOcean; }
	void SetInfinite(bool inf) { infiniteOcean = inf; }

private:
	// Wave parameters
	float waveAmp1 = 0.5f, waveLambda1 = 15.0f, waveSpeed1 = 1.0f;
	Vector2 waveDir1 = { 1.0f, 0.6f };
	float waveAmp2 = 0.25f, waveLambda2 = 8.0f, waveSpeed2 = 1.2f;
	Vector2 waveDir2 = { 0.8f, -0.6f };
	float planeSize = 2000.0f;
	int planeRes = 64;
	Vector4 waterColor = { 0.0f, 0.1f, 0.3f, 1.0f };
	bool infiniteOcean = false;

	Material* waterMaterial = nullptr;
	bool ownMaterial = false;
	RaylibModel raylibModel;
	bool modelSet = false;
	bool editorSetup = false;
	CameraComponent* mainCamera = nullptr;
};