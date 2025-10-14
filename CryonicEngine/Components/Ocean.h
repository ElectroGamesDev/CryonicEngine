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
            2.0,
            "Wave Amplitude 1",
            { "min": 0.0, "max": 5.0 }
        ],
        [
            "float",
            "waveLambda1",
            15.0,
            "Wave Length 1",
            { "min": 1.0, "max": 100.0 }
        ],
        [
            "float",
            "waveSpeed1",
            1.0,
            "Wave Speed 1",
            { "min": 0.0, "max": 10.0 }
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
            1.0,
            "Wave Amplitude 2",
            { "min": 0.0, "max": 5.0 }
        ],
        [
            "float",
            "waveLambda2",
            8.0,
            "Wave Length 2",
            { "min": 1.0, "max": 100.0 }
        ],
        [
            "float",
            "waveSpeed2",
            1.2,
            "Wave Speed 2",
            { "min": 0.0, "max": 10.0 }
        ],
        [
            "Vector2",
            "waveDir2",
            [0.8, -0.6],
            "Wave Direction 2"
        ],
        [
            "float",
            "waveAmp3",
            0.5,
            "Wave Amplitude 3",
            { "min": 0.0, "max": 5.0 }
        ],
        [
            "float",
            "waveLambda3",
            4.0,
            "Wave Length 3",
            { "min": 1.0, "max": 100.0 }
        ],
        [
            "float",
            "waveSpeed3",
            1.5,
            "Wave Speed 3",
            { "min": 0.0, "max": 10.0 }
        ],
        [
            "Vector2",
            "waveDir3",
            [0.6, 0.8],
            "Wave Direction 3"
        ],
        [
            "float",
            "waveAmp4",
            0.05,
            "Wave Amplitude 4",
            { "min": 0.0, "max": 5.0 }
        ],
        [
            "float",
            "waveLambda4",
            1.0,
            "Wave Length 4",
            { "min": 1.0, "max": 100.0 }
        ],
        [
            "float",
            "waveSpeed4",
            1.0,
            "Wave Speed 4",
            { "min": 0.0, "max": 10.0 }
        ],
        [
            "Vector2",
            "waveDir4",
            [-0.8, 0.5],
            "Wave Direction 4"
        ],
        [
            "float",
            "steepness",
            0.7,
            "Steepness/Choppiness",
		{ "min": 0.0, "max" : 1.0 }
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
			128,
			"Plane Resolution",
			{ "min": 10, "max" : 256 }
		],
		[
			"Vector4",
			"waterColor",
			[0.0, 0.1, 0.3, 1.0],
			"Water Color"
		],
		[
			"float",
			"foamThreshold",
			0.02,
			"Foam Threshold",
			{ "min": 0.0, "max" : 0.5 }
		],
		[
			"float",
			"waterTransparency",
			0.9,
			"Water Transparency",
			{ "min": 0.0, "max" : 1.0 }
		],
        [
            "float",
            "rippleScale",
            30.0,
            "Ripple Scale",
            { "min": 10.0, "max" : 50.0 }
        ],
        [
            "float",
            "rippleAmp",
            0.15,
            "Ripple Amplitude",
            { "min": 0.0, "max" : 0.5 }
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
#if defined(EDITOR)
	void EditorUpdate() override;
#endif
	void Destroy() override;

	float GetWaveAmp1() const { return waveAmp1; }
	void SetWaveAmp1(float amp) { waveAmp1 = amp; }
	// Todo: Add other functions for wave parameters

	bool IsInfinite() const { return infiniteOcean; }
	void SetInfinite(bool inf) { infiniteOcean = inf; }

	static void RenderOceans();
	void RenderOcean();

private:
	// Wave parameters
	float waveAmp1 = 2.0f, waveLambda1 = 15.0f, waveSpeed1 = 1.0f;
	Vector2 waveDir1 = { 1.0f, 0.6f };
	float waveAmp2 = 1.0f, waveLambda2 = 8.0f, waveSpeed2 = 1.2f;
	Vector2 waveDir2 = { 0.8f, -0.6f };
	float waveAmp3 = 0.5f, waveLambda3 = 4.0f, waveSpeed3 = 1.5f;
	Vector2 waveDir3 = { 0.6f, 0.8f };
	float waveAmp4 = 0.05f, waveLambda4 = 1.0f, waveSpeed4 = 1.0f;
	Vector2 waveDir4 = { -0.8f, 0.5f };
	float steepness = 0.7f;
	float planeSize = 2000.0f;
	int planeRes = 128;
	Vector4 waterColor = { 0.0f, 0.1f, 0.3f, 1.0f };
	float foamThreshold = 0.02f;
	float waterTransparency = 0.9f;
	float rippleScale = 30.0f;
	float rippleAmp = 0.15f;
	bool infiniteOcean = false;

	Material* waterMaterial = nullptr;
	bool ownMaterial = false;
	RaylibModel raylibModel;
	bool modelSet = false;
	bool editorSetup = false;
	CameraComponent* mainCamera = nullptr;
    bool doOnce = true;

	static std::vector<Ocean*> oceans;
};