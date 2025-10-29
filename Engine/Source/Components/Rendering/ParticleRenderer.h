//#pragma once
//#include "Components/Component.h"
//#include "ThirdParty/Misc/json.hpp"
//#include "Core/GameObject.h"
//#include "Resources/Particles.h"
//#include "Raylib/RaylibWrapper.h"
//#include "Systems/Rendering/ShaderManager.h"
//#include "Utilities/MathUtility.h" 
//#include "ThirdParty/raylib/include/rlgl.h"
//#include "ThirdParty/raylib/include/external/glad.h"
//struct Particle
//{
//	Vector3 position;
//	Vector3 velocity;
//	Color color;
//	float size;
//	float rotation;
//	float angularVelocity;
//	float time;
//	float lifetime;
//	bool active;
//	float frame;
//	float frameTime;
//	unsigned int seed;
//};
//struct ParticleGPU
//{
//	float pos[3];
//	float time;
//	float vel[3];
//	float lifetime;
//	float color[4];
//	float size;
//	float rotation;
//	float angularVelocity;
//	float frame;
//	float frameTime;
//};
//struct EmitterInstance
//{
//	nlohmann::json* config;
//	EmitterInstance* parent = nullptr;
//	Vector3 localPosition = { 0,0,0 };
//	float time = 0.0f;
//	float lastSpawnTime = 0.0f;
//	std::vector<Particle> particles;
//	std::vector<EmitterInstance*> children;
//	std::vector<EmitterInstance*> subEmitters;
//	std::vector<bool> burstDone;
//	unsigned int seed = 0;
//	Vector3 GetWorldPosition(const ParticleRenderer* renderer);
//	void ClearParticles();
//	// For GPU, additional
//	unsigned int particleSSBO = 0;
//	unsigned int aliveSSBO[2] = { 0,0 };
//	unsigned int deadSSBO = 0;
//	unsigned int counterSSBO = 0;
//	unsigned int indirectSSBO = 0;
//	int currentPing = 0;
//	float accumEmit = 0.0f;
//};
//class ParticleRenderer : public Component
//{
//public:
//	ParticleRenderer(GameObject* obj, int id) : Component(obj, id)
//	{
//		runInEditor = true;
//		name = "ParticleRenderer";
//		iconUnicode = "\xef\x9c\xa8";
//#if defined(EDITOR)
//		std::string variables = R"(
//        [
//            0,
//            [
//                [
//                    "Particles",
//                    "particles",
//                    "nullptr",
//                    "Particles",
//                    {
//                        "Extensions": [".particles"]
//                    }
//                ],
//                [
//                    "bool",
//                    "lit",
//                    false,
//                    "Lit"
//                ],
//                [
//                    "bool",
//                    "gpu",
//                    true,
//                    "GPU Simulation"
//                ]
//            ]
//        ]
//    )";
//		exposedVariables = nlohmann::json::parse(variables);
//#endif
//	}
//	ParticleRenderer* Clone() override { return new ParticleRenderer(gameObject, -1); }
//	void Awake() override;
//	void Start() override;
//	void Update() override;
//	void Render() override;
//#if defined(EDITOR)
//	void EditorUpdate() override;
//#endif
//	void Destroy() override;
//	void SetParticles(Particles* particles);
//	Particles* GetParticles();
//	void Play();
//	void Pause();
//	void Stop();
//	void ClearParticles();
//	void Simulate(float delta);
//private:
//	Particles* particles = nullptr;
//	std::vector<EmitterInstance> emitters;
//	bool playing = false;
//	bool lit = false;
//	bool gpu = true;
//	Shader computeEmit;
//	Shader computeSim;
//	Shader particleShader;
//	bool worldSpace = false;
//	float duration = 5.0f;
//	bool looping = true;
//	int maxParticles = 100000;
//	size_t onDataChangeEventId;
//	void LoadParticles();
//	void InitializeEmitter(EmitterInstance& emitter, nlohmann::json* config, EmitterInstance* parent = nullptr);
//	void SpawnParticlesCPU(EmitterInstance& emitter, float delta);
//	void UpdateParticlesCPU(EmitterInstance& emitter, float delta);
//	void RenderParticlesCPU(const EmitterInstance& emitter);
//	void UpdateGPU(EmitterInstance& emitter, float delta);
//	void RenderGPU(const EmitterInstance& emitter);
//	void InitializeGPU(EmitterInstance& emitter);
//	void DestroyGPU(EmitterInstance& emitter);
//	void SetUniforms(EmitterInstance& emitter);
//	Vector3 GetSpawnPosition(const nlohmann::json& shape, const Vector3& emitterPos);
//	float SampleCurve(float t, const nlohmann::json& curve);
//	Color SampleGradient(float t, const nlohmann::json& gradient);
//	void ApplyForces(Particle& p, const nlohmann::json& forces, float delta);
//	void HandleCollision(Particle& p, Vector3 oldPos, const nlohmann::json& collision);
//	void TriggerSubEmitters(const EmitterInstance& emitter, const Particle& p, const std::string& trigger);
//	void SortParticles(std::vector<Particle*>& sorted, const Camera3D& camera);
//	void DrawParticle(const Particle& p, const nlohmann::json& appearance, const Camera3D& camera);
//	void UpdateSubEmitters(EmitterInstance& emitter, float delta);
//};