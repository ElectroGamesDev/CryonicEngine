#pragma once

#include "Component.h"
#include "../RaylibWrapper.h"
#include "../RaylibModelWrapper.h"
#include "../Material.h"
#include "../ShaderManager.h"
#include <filesystem>
#include "CameraComponent.h"

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
	float coverage = 0.5f;
	float density = 1.0f;
	float windSpeed = 20.0f;
	Vector2 windDir = { 1.0f, 0.0f };
	float cloudHeight = 1000.0f;
	float cloudThickness = 200.0f;
	int raymarchSteps = 16;
	float absorption = 0.8f;
	float scattering = 1.2f;
	float phaseG = 0.5f;
	Vector3 sunDir = { 0.0f, -1.0f, 0.0f };
	Vector3 sunColor = { 1.0f, 0.95f, 0.8f };
	float sunIntensity = 1.0f;
	float ambientLight = 0.2f;
	bool highQuality = true;

	bool editorSetup = false;
	CameraComponent* mainCamera = nullptr;
	float startTime = 0.0f;
	bool doOnce = true;

	static std::vector<Clouds*> clouds;
};