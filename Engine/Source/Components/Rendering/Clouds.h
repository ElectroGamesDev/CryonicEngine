#pragma once

#include "Components/Component.h"
#include "Core/GameObject.h"
#include "Raylib/RaylibModelWrapper.h"
#include "Resources/Material.h"
#include "Systems/Rendering/ShaderManager.h"
#include <filesystem>
#include "Components/Misc/CameraComponent.h"

class Clouds : public Component
{
public:
	Clouds(GameObject* obj, int id) : Component(obj, id)
	{
		runInEditor = true;
		name = "Clouds";
		iconUnicode = "\xef\x83\x82";

#if defined(EDITOR)
		std::string variables = R"(
[
    0,
    [
        [
            "float",
            "coverage",
            0.5,
            "Coverage",
            { "min": 0.0, "max": 1.0 }
        ],
        [
            "float",
            "density",
            1.0,
            "Density",
            { "min": 0.0, "max": 2.0 }
        ],
        [
            "float",
            "baseNoiseFrequency",
            0.0025,
            "Base Scale",
            { "min": 0.0, "max": 1.0 }
        ],
        [
            "float",
            "detailNoiseMultiplier",
            2.0,
            "Detail Scale",
            { "min": 0.0, "max": 10.0 }
        ],
        [
            "int",
            "lightSteps",
            6,
            "Light Steps",
            { "min": 0, "max": 50 }
        ],
        [
            "float",
            "lightMarchSize",
            25.0,
            "Light March Size",
            { "min": 0.0, "max": 100.0 }
        ],
        [
            "float",
            "windSpeed",
            20.0,
            "Wind Speed",
            { "min": 0.0, "max": 100.0 }
        ],
        [
            "Vector2",
            "windDir",
            [1.0, 0.0],
            "Wind Direction"
        ],
        [
            "float",
            "cloudHeight",
            1000.0,
            "Cloud Height",
            { "min": 100.0, "max": 5000.0 }
        ],
        [
            "float",
            "cloudThickness",
            200.0,
            "Cloud Thickness",
            { "min": 50.0, "max": 1000.0 }
        ],
        [
            "int",
            "raymarchSteps",
            16,
            "Raymarch Steps",
            { "min": 8, "max": 64 }
        ],
        [
            "float",
            "absorption",
            0.8,
            "Absorption",
            { "min": 0.1, "max": 2.0 }
        ],
        [
            "float",
            "scattering",
            1.2,
            "Scattering",
            { "min": 0.5, "max": 3.0 }
        ],
        [
            "float",
            "phaseG",
            0.5,
            "Phase Function G",
            { "min": -0.99, "max": 0.99 }
        ],
        [
            "Vector3",
            "sunDir",
            [0.0, -1.0, 0.0],
            "Sun Direction"
        ],
        [
            "Vector3",
            "sunColor",
            [1.0, 0.95, 0.8],
            "Sun Color"
        ],
        [
            "float",
            "sunIntensity",
            1.0,
            "Sun Intensity",
            { "min": 0.0, "max": 5.0 }
        ],
        [
            "float",
            "ambientLight",
            0.2,
            "Ambient Light",
            { "min": 0.0, "max": 1.0 }
        ],
        [
            "bool",
            "highQuality",
            true,
            "High Quality"
        ]
    ]
]
)";

		exposedVariables = nlohmann::json::parse(variables);

#endif
	}

	Clouds* Clone() override
	{
		return new Clouds(gameObject, -1);
	}

	void Awake() override;
	void Start() override;
	void Update() override;
#if defined(EDITOR)
	void EditorUpdate() override;
#endif
	void Destroy() override;

	float GetCoverage() const { return coverage; }
	void SetCoverage(float cov) { coverage = cov; }

	// TODO: Add other getters/setters

	static void RenderClouds();
	void RenderCloud();

private:
	// Cloud parameters
	float coverage = 0.45f;
	float density = 0.015f;
	float windSpeed = 20.0f;
	float baseNoiseFrequency = 0.0025f;
	float detailNoiseMultiplier = 2.0;
	Vector2 windDir = { 1.0f, 0.0f };
	float cloudHeight = 1000.0f;
	float cloudThickness = 500.0f;
	int raymarchSteps = 32;
    int lightSteps = 6;
    float lightMarchSize = 25.0f;
	float absorption = 0.18f;
	float scattering = 1.6f;
	float phaseG = 0.65f;
	Vector3 sunDir = { 0.0f, -1.0f, 0.0f };
	Vector3 sunColor = { 1.0f, 0.95f, 0.8f };
	float sunIntensity = 1.0f;
	float ambientLight = 0.2f;
	bool highQuality = true;

    std::pair<unsigned int, int*> shader;
    unsigned int raymarchShader;
    unsigned int noiseShader;
	unsigned int noiseTexture = 0;
	unsigned int halfCloudTexture = 0;
	unsigned int previousCloudTexture = 0;
    unsigned int currentCloudTexture = 0;

	bool editorSetup = false;
	CameraComponent* mainCamera = nullptr;
	float startTime = 0.0f;
    bool firstFrame = true;
	bool doOnce = true;

    RaylibWrapper::Matrix prevView;
    RaylibWrapper::Matrix prevProj;

	static std::vector<Clouds*> clouds;
};