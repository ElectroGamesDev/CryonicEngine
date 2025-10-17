#pragma once
#include "Components/Component.h"
#include <filesystem>
#include <deque>
#include "Systems/Rendering/ShadowManager.h"

class Lighting : public Component
{
public:
	Lighting(GameObject* obj, int id) : Component(obj, id)
	{
		name = "Lighting";
		iconUnicode = "\xef\x83\xab";
		runInEditor = true;
		Awake();
#if defined(EDITOR)
		std::string variables = R"(
        [
            0,
            [
                [
                    "Color",
                    "color",
                    [ 255, 255, 255, 255 ],
                    "Color"
                ],
                [
                    "Type",
                    "type",
                    "Point",
                    "Type",
                    [
                        "Point",
                        "Spot",
                        "Directional"
                    ]
                ],
                [
                    "float",
                    "intensity",
                    1.0,
                    "Intensity"
                ],
                [
                    "float",
                    "range",
                    10.0,
                    "Range"
                ],
                [
                    "float",
                    "spotInnerAngle",
                    22.5,
                    "Spot Inner Angle"
                ],
                [
                    "float",
                    "spotOuterAngle",
                    30.0,
                    "Spot Outer Angle"
                ],
                [
                    "bool",
                    "castShadows",
                    true,
                    "Cast Shadows"
                ]
            ]
        ]
        )";
		exposedVariables = nlohmann::json::parse(variables);
#endif
	}

	enum Type
	{
		Point = 0,
		Spot = 1,
		Directional = 2
	};

	Lighting* Clone() override
	{
		return new Lighting(gameObject, -1);
	}

	void Awake() override;
	void Destroy() override;
	void Enable() override;
	void Disable() override;
	void RenderLight(int index);

#ifdef EDITOR
	void EditorUpdate() override;
#endif

	static std::deque<Lighting*> lights;
	static int nextId;

	int lightId;
	Color color = { 255, 255, 255, 255 };
	Type type = Point;
	float intensity = 1.0f;
	float range = 10.0f;
	float spotInnerAngle = 22.5f;  // Degrees
	float spotOuterAngle = 30.0f;  // Degrees
	bool castShadows = true;

private:
	bool wasLastActive = false;
	bool needsShaderUpdate = true;
	ShadowManager shadowManager;

	Vector3 lastPosition = { 0, 0, 0 };
	Quaternion lastRotation = { 0, 0, 0, 1 };
	Color lastColor = { 0, 0, 0, 0 };
	Type lastType = Point;
	float lastIntensity = -1.0f;
	float lastRange = -1.0f;
	float lastSpotInnerAngle = -1.0f;
	float lastSpotOuterAngle = -1.0f;

	void UpdateShaderProperties();
	void UpdateShadowCamera();
};