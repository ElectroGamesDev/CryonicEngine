#pragma once
#include <string>
#include <vector>
#include <any>

namespace EventSheetSystem
{
	// Custom ints to maintain backwards compatibility with saved data
	enum class Conditions {
		None = 0,

		// Input - Keyboard (1-20)
		AnyKeyPressed = 1,
		AnyKeyReleased = 2,
		KeyPressed = 3,
		KeyReleased = 4,
		KeyDown = 5,

		// Input - Mouse (21-40)
		MouseButtonPressed = 6,
		MouseButtonReleased = 7,
		MouseButtonDown = 8,
		MouseMoved = 9,
		MouseInZone = 10,
		MouseOverObject = 21,
		MouseX = 22,
		MouseY = 23,

		// Input - Gamepad (41-60)
		GamepadButtonPressed = 11,
		GamepadButtonReleased = 12,
		GamepadButtonDown = 13,
		GamepadAxisMoved = 14,
		GamepadConnected = 41,
		GamepadDisconnected = 42,

		// GameObject - Transform (61-80)
		ComparePosition = 61,
		CompareX = 62,
		CompareY = 63,
		CompareZ = 64,
		CompareRotation = 65,
		CompareScale = 66,
		CompareDistance = 67,
		IsVisible = 68,

		// GameObject - Properties (81-100)
		IsActive = 81,
		HasTag = 82,
		HasComponent = 83,
		CompareLayer = 84,
		CompareName = 85,

		// Physics (101-120)
		IsColliding = 101,
		HasRigidbody = 102,
		CompareVelocity = 103,
		IsGrounded = 104,
		IsTrigger = 105,

		// Time (121-140)
		TimerElapsed = 121,
		EveryXSeconds = 122,
		CompareTime = 123,
		CompareDeltaTime = 124,

		// Logic (141-160)
		CompareVariable = 141,
		IsTrue = 142,
		IsFalse = 143,
		And = 144,
		Or = 145,
		Not = 146,

		// System (161-180)
		OnStart = 161,
		OnUpdate = 162,
		FrameCount = 163,
		PlatformIs = 164,

		// Audio (181-200)
		IsSoundPlaying = 181,
		IsMusicPlaying = 182,
		CompareVolume = 183,

		// Scene (201-220)
		SceneIs = 201,
		ObjectExists = 202,
		ObjectCount = 203,

		// Rendering (221-240)
		IsOnScreen = 221,
		CompareOpacity = 222,
		CompareColor = 223,

		// Animation (241-260)
		IsAnimationPlaying = 241,
		AnimationFinished = 242,
		CurrentFrame = 243,

		// UI (261-280)
		ButtonClicked = 261,
		ButtonHovered = 262,
		TextInputChanged = 263,
		SliderChanged = 264,

		// Math (281-300)
		CompareNumbers = 281,
		IsEven = 282,
		IsOdd = 283,
		InRange = 284,
		RandomChance = 285,
	};

	enum class Actions {
		None = 0,

		// Transform - Position (1-20)
		Position = 5,
		MovePosition = 1,
		XPosition = 2,
		YPosition = 3,
		ZPosition = 4,
		MoveTowards = 21,
		LookAt = 22,

		// Transform - Rotation (23-40)
		SetRotation = 23,
		SetRotationX = 24,
		SetRotationY = 25,
		SetRotationZ = 26,
		Rotate = 27,
		RotateTowards = 28,

		// Transform - Scale (41-60)
		SetScale = 41,
		SetScaleX = 42,
		SetScaleY = 43,
		SetScaleZ = 44,
		ScaleBy = 45,

		// GameObject (61-80)
		SetActive = 61,
		Destroy = 62,
		Instantiate = 63,
		SetName = 64,
		SetTag = 65,
		SetLayer = 66,
		AddComponent = 67,
		RemoveComponent = 68,
		SetParent = 69,

		// Physics (81-100)
		SetVelocity = 81,
		AddForce = 82,
		SetGravity = 83,
		SetMass = 84,
		SetDrag = 85,
		SetAngularVelocity = 86,
		AddTorque = 87,
		FreezePosition = 88,
		FreezeRotation = 89,

		// Rendering (101-120)
		SetColor = 101,
		SetOpacity = 102,
		SetSprite = 103,
		SetMaterial = 104,
		FlipX = 105,
		FlipY = 106,
		SetSortingOrder = 107,
		SetVisible = 108,

		// Animation (121-140)
		PlayAnimation = 121,
		StopAnimation = 122,
		PauseAnimation = 123,
		SetAnimationSpeed = 124,
		SetFrame = 125,

		// Audio (141-160)
		PlaySound = 141,
		StopSound = 142,
		PauseSound = 143,
		ResumeSound = 144,
		SetVolume = 145,
		SetPitch = 146,
		PlayMusic = 147,
		StopMusic = 148,
		FadeIn = 149,
		FadeOut = 150,

		// Variables (161-180)
		SetVariable = 161,
		AddToVariable = 162,
		SubtractFromVariable = 163,
		MultiplyVariable = 164,
		DivideVariable = 165,

		// Timer (181-200)
		StartTimer = 181,
		StopTimer = 182,
		ResetTimer = 183,
		PauseTimer = 184,

		// Scene (201-220)
		LoadScene = 201,
		ReloadScene = 202,
		QuitGame = 203,
		PauseGame = 204,
		ResumeGame = 205,
		SetTimeScale = 206,

		// Camera (221-240)
		SetCameraPosition = 221,
		SetCameraRotation = 222,
		CameraFollow = 223,
		SetCameraSize = 224,
		ShakeCamera = 225,

		// Effects (241-260)
		CreateParticles = 241,
		StopParticles = 242,
		ScreenFlash = 243,

		// UI (261-280)
		SetText = 261,
		SetButtonEnabled = 262,
		SetSliderValue = 263,
		ShowUI = 264,
		HideUI = 265,

		// Debug (281-300)
		Log = 281,
		LogWarning = 282,
		LogError = 283,
		DrawLine = 284,
		DrawCircle = 285,

		// Flow Control (301-320)
		Wait = 301,
		CallFunction = 302,

		// Input (321-340)
		SetCursorVisible = 321,
		SetCursorLocked = 322,
		VibrateGamepad = 323,
	};

	struct Param {
		std::string name;
		std::string type;
		std::any value;

		// Helper function to get parameter display string
		std::string GetDisplayString() const;
	};

	struct Condition {
		Conditions type;
		std::string stringType;
		std::vector<std::string> category;
		std::vector<Param> params;

		// Helper to get formatted condition text
		std::string GetFormattedText() const;
	};

	struct Action {
		Actions type;
		std::string stringType;
		std::vector<std::string> category;
		std::vector<Param> params;

		// Helper to get formatted action text
		std::string GetFormattedText() const;
	};

	struct Event {
		std::vector<Condition> conditions;
		std::vector<Action> actions;
	};

	// Helper functions for getting all available conditions/actions
	const std::vector<Condition>& GetAllConditions();
	const std::vector<Action>& GetAllActions();
}