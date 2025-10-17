#pragma once

// Core
#include "Utilities/ConsoleLogger.h"
#include "Core/GameObject.h"
#include "Systems/Scene/SceneManager.h"
#include "Systems/Input/InputSystem.h"
#include "Systems/Events/EventSystem.h"

// Rendering
#include "Components/Rendering/MeshRenderer.h"
#include "Components/Rendering/SpriteRenderer.h"
#include "Components/Rendering/Lighting.h"
#include "Components/Rendering/Skybox.h"

// Physics
#include "Components/Physics/Collider2D.h"
#include "Components/Physics/Rigidbody2D.h"
#if defined(IS3D)
#include "Components/Physics/Collider3D.h"
#include "Components/Physics/Rigidbody3D.h"
#endif

// UI
#include "Components/UI/Label.h"
#include "Components/UI/Image.h"
#include "Components/UI/Button.h"
#include "Components/UI/CanvasRenderer.h"

// Misc
#include "Components/Component.h"
#include "Components/Animation/AnimationPlayer.h"
#include "Components/Audio/AudioPlayer.h"
#include "Components/Misc/CameraComponent.h"
#include <functional>

// Resources
#include "Resources/Sprite.h"
#include "Resources/Material.h"
#include "Resources/Animation.h"
#include "Resources/AnimationGraph.h"
#include "Resources/AudioClip.h"
#include "Resources/Font.h"
#include "Resources/Prefab.h"
#include "Resources/Tilemap.h"