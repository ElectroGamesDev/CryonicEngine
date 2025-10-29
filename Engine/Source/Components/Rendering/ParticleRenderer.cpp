//#include "ParticleRenderer.h"
//#include "Core/SceneManager.h"
//#include "Utilities/ConsoleLogger.h"
//#include "Raylib/RaylibDrawWrapper.h"
//#include "Systems/Physics/PhysicsSystem.h"
//struct CountersGPU
//{
//	unsigned int aliveCount;
//	unsigned int newAliveCount;
//	unsigned int deadCount;
//};
//struct IndirectDrawGPU
//{
//	unsigned int vertexCount;
//	unsigned int instanceCount;
//	unsigned int firstVertex;
//	unsigned int firstInstance;
//};
//void ParticleRenderer::Awake()
//{
//	computeEmit = RaylibWrapper::LoadShader(0, "Resources/Shaders/particle_emit.comp");
//	computeSim = RaylibWrapper::LoadShader(0, "Resources/Shaders/particle_sim.comp");
//	particleShader = RaylibWrapper::LoadShader("Resources/Shaders/particle.vs", "Resources/Shaders/particle.fs");
//	SetParticles(particles);
//}
//void ParticleRenderer::Start()
//{
//	Play();
//}
//void ParticleRenderer::Update()
//{
//	if (!playing) return;
//	float delta = RaylibWrapper::GetFrameTime();
//	for (auto& emitter : emitters)
//	{
//		emitter.time += delta;
//		if (gpu)
//		{
//			UpdateGPU(emitter, delta);
//		}
//		else
//		{
//			SpawnParticlesCPU(emitter, delta);
//			UpdateParticlesCPU(emitter, delta);
//		}
//		UpdateSubEmitters(emitter, delta);
//	}
//}
//void ParticleRenderer::Render()
//{
//	RaylibWrapper::Camera3D camera = SceneManager::GetActiveScene()->GetCamera();
//	BeginBlendMode(BLEND_ALPHA); // or from config
//	rlDisableDepthMask();
//	for (const auto& emitter : emitters)
//	{
//		if (gpu)
//			RenderGPU(emitter);
//		else
//			RenderParticlesCPU(emitter);
//	}
//	rlEnableDepthMask();
//	EndBlendMode();
//}
//#if defined(EDITOR)
//void ParticleRenderer::EditorUpdate()
//{
//	std::string newPath = exposedVariables[1][0][2].get<std::string>();
//	if ((!particles && !newPath.empty() && newPath != "nullptr") || (particles && particles->GetRelativePath() != newPath))
//	{
//		if (newPath.empty() || newPath == "nullptr")
//			SetParticles(nullptr);
//		else
//			SetParticles(new Particles(newPath));
//	}
//	lit = exposedVariables[1][1][2].get<bool>();
//	gpu = exposedVariables[1][2][2].get<bool>();
//}
//#endif
//void ParticleRenderer::Destroy()
//{
//	if (particles)
//		particles->onDataChangeEvent.Unsubscribe(onDataChangeEventId);
//	UnloadShader(computeEmit);
//	UnloadShader(computeSim);
//	UnloadShader(particleShader);
//	for (auto& emitter : emitters)
//	{
//		if (gpu)
//			DestroyGPU(emitter);
//	}
//}
//void ParticleRenderer::SetParticles(Particles* particles)
//{
//	if (this->particles)
//		this->particles->onDataChangeEvent.Unsubscribe(onDataChangeEventId);
//	this->particles = particles;
//	if (!particles)
//		return;
//	onDataChangeEventId = particles->onDataChangeEvent.Subscribe([this]() {
//		LoadParticles();
//		});
//	LoadParticles();
//}
//Particles* ParticleRenderer::GetParticles()
//{
//	return particles;
//}
//void ParticleRenderer::Play()
//{
//	playing = true;
//}
//void ParticleRenderer::Pause()
//{
//	playing = false;
//}
//void ParticleRenderer::Stop()
//{
//	playing = false;
//	ClearParticles();
//}
//void ParticleRenderer::ClearParticles()
//{
//	for (auto& emitter : emitters)
//		emitter.ClearParticles();
//}
//void ParticleRenderer::Simulate(float delta)
//{
//	for (auto& emitter : emitters)
//	{
//		emitter.time += delta;
//		if (gpu)
//			UpdateGPU(emitter, delta);
//		else
//		{
//			SpawnParticlesCPU(emitter, delta);
//			UpdateParticlesCPU(emitter, delta);
//		}
//		UpdateSubEmitters(emitter, delta);
//	}
//}
//void ParticleRenderer::LoadParticles()
//{
//	for (auto& emitter : emitters)
//	{
//		if (gpu)
//			DestroyGPU(emitter);
//	}
//	emitters.clear();
//	nlohmann::json* data = particles->GetData();
//	if (!data || data->is_null())
//		return;
//	worldSpace = (*data)["simulation_space"] == "world";
//	duration = (*data)["duration"].get<float>();
//	looping = (*data)["looping"].get<bool>();
//	maxParticles = (*data)["max_particles"].get<int>();
//	gpu = (*data)["gpu"].get<bool>();
//	for (auto& emJson : (*data)["emitters"])
//	{
//		emitters.emplace_back();
//		InitializeEmitter(emitters.back(), &emJson);
//	}
//}
//void ParticleRenderer::InitializeEmitter(EmitterInstance& emitter, nlohmann::json* config, EmitterInstance* parent)
//{
//	emitter.config = config;
//	emitter.parent = parent;
//	if (config->contains("local_position"))
//		emitter.localPosition = { (*config)["local_position"][0].get<float>(), (*config)["local_position"][1].get<float>(), (*config)["local_position"][2].get<float>() };
//	emitter.seed = (*config)["random_seed"].get<unsigned int>();
//	if (gpu)
//		InitializeGPU(emitter);
//	else
//	{
//		emitter.particles.resize(maxParticles);
//		for (auto& p : emitter.particles) p.active = false;
//	}
//	if (config->contains("bursts"))
//		emitter.burstDone.resize((*config)["bursts"].size(), false);
//	if (config->contains("children"))
//	{
//		for (auto& childJson : (*config)["children"])
//		{
//			emitter.children.emplace_back(new EmitterInstance());
//			InitializeEmitter(*emitter.children.back(), &childJson, &emitter);
//		}
//	}
//}
//void ParticleRenderer::SpawnParticlesCPU(EmitterInstance& emitter, float delta)
//{
//	nlohmann::json& cfg = *emitter.config;
//	if (cfg["modules"]["emission"]["delay"].get<float>() > emitter.time) return;
//	if (!cfg["modules"]["emission"]["looping"].get<bool>() && emitter.time > cfg["modules"]["emission"]["duration"].get<float>()) return;
//	// Burst
//	if (cfg["modules"]["emission"].contains("bursts"))
//	{
//		auto& bursts = cfg["modules"]["emission"]["bursts"];
//		for (size_t i = 0; i < bursts.size(); i++)
//		{
//			float bt = bursts[i][0].get<float>();
//			if (emitter.time >= bt && !emitter.burstDone[i])
//			{
//				int count = bursts[i][1].get<int>();
//				for (int j = 0; j < count; j++)
//					SpawnSingleParticleCPU(emitter);
//				emitter.burstDone[i] = true;
//			}
//		}
//	}
//	// Rate
//	float rate = cfg["modules"]["emission"]["rate"].get<float>();
//	float toSpawn = rate * delta;
//	while (toSpawn >= 1.0f)
//	{
//		SpawnSingleParticleCPU(emitter);
//		toSpawn -= 1.0f;
//	}
//	if (GetRandomValue(0, 1) < toSpawn)
//		SpawnSingleParticleCPU(emitter);
//}
//void ParticleRenderer::SpawnSingleParticleCPU(EmitterInstance& emitter)
//{
//	nlohmann::json& cfg = *emitter.config;
//	Particle* p = nullptr;
//	for (auto& part : emitter.particles)
//	{
//		if (!part.active)
//		{
//			p = &part;
//			break;
//		}
//	}
//	if (!p) return;
//	p->active = true;
//	p->time = 0.0f;
//	p->lifetime = SampleCurve(GetRandomValue(0, 1), cfg["modules"]["emission"]["lifetime_curve"]);
//	p->position = GetSpawnPosition(cfg["modules"]["shape"], emitter.GetWorldPosition(this));
//	p->velocity = { cfg["modules"]["velocity"]["initial"][0].get<float>(),[1],[2] };
//	// add random, inherit
//	p->color = SampleGradient(0.0f, cfg["modules"]["color"]["gradient"]);
//	p->size = SampleCurve(0.0f, cfg["modules"]["size"]["curve"]);
//	p->rotation = cfg["modules"]["rotation"]["initial"].get<float>();
//	p->angularVelocity = cfg["modules"]["rotation"]["angular_velocity"].get<float>();
//	p->frame = GetRandomValue(0, cfg["modules"]["appearance"]["frame_count"].get<int>() - 1);
//	p->frameTime = 0.0f;
//	p->seed = rand();
//	TriggerSubEmitters(emitter, *p, "spawn");
//}
//void ParticleRenderer::UpdateParticlesCPU(EmitterInstance& emitter, float delta)
//{
//	nlohmann::json& cfg = *emitter.config;
//	for (auto& p : emitter.particles)
//	{
//		if (!p.active) continue;
//		p.time += delta;
//		if (p.time > p.lifetime)
//		{
//			p.active = false;
//			TriggerSubEmitters(emitter, p, "death");
//			continue;
//		}
//		float normTime = p.time / p.lifetime;
//		p.velocity = Vector3Scale(p.velocity, SampleCurve(normTime, cfg["modules"]["velocity"]["over_lifetime"]));
//		p.velocity = Vector3Add(p.velocity, Vector3Scale(cfg["modules"]["velocity"]["acceleration"], delta));
//		p.velocity = Vector3Scale(p.velocity, 1.0f - cfg["modules"]["velocity"]["drag"].get<float>() * delta);
//		p.position = Vector3Add(p.position, Vector3Scale(p.velocity, delta));
//		p.color = SampleGradient(normTime, cfg["modules"]["color"]["gradient"]);
//		p.size = SampleCurve(normTime, cfg["modules"]["size"]["curve"]);
//		p.rotation += p.angularVelocity * delta * SampleCurve(normTime, cfg["modules"]["rotation"]["over_lifetime"]);
//		p.frameTime += delta;
//		if (p.frameTime > 1.0f / cfg["modules"]["appearance"]["fps"].get<float>())
//		{
//			p.frame = mod(p.frame + 1, cfg["modules"]["appearance"]["frame_count"].get<int>());
//			p.frameTime = 0.0f;
//		}
//		ApplyForces(p, cfg["modules"]["forces"], delta);
//		Vector3 oldPos = Vector3Subtract(p.position, Vector3Scale(p.velocity, delta));
//		HandleCollision(p, oldPos, cfg["modules"]["collision"]);
//		TriggerSubEmitters(emitter, p, "update");
//	}
//}
//void ParticleRenderer::RenderParticlesCPU(const EmitterInstance& emitter)
//{
//	nlohmann::json& cfg = *emitter.config;
//	RaylibWrapper::Camera3D camera = SceneManager::GetActiveScene()->GetCamera();
//	std::vector<Particle*> sortedParticles;
//	for (const auto& p : emitter.particles)
//		if (p.active) sortedParticles.push_back(const_cast<Particle*>(&p));
//	if (cfg["rendering"]["sorting"].get<std::string>() == "depth")
//		SortParticles(sortedParticles, camera);
//	BeginShaderMode(particleShader);
//	rlSetTexture(TextureManager::GetTexture(cfg["modules"]["appearance"]["texture"].get<std::string>())->id);
//	for (const auto* p : sortedParticles)
//	{
//		DrawParticle(*p, cfg["modules"]["appearance"], camera);
//	}
//	rlSetTexture(0);
//	EndShaderMode();
//	for (const auto* child : emitter.children)
//		RenderParticlesCPU(*child);
//	for (const auto* sub : emitter.subEmitters)
//		RenderParticlesCPU(*sub);
//}
//void ParticleRenderer::UpdateGPU(EmitterInstance& emitter, float delta)
//{
//	SetUniforms(emitter);
//	nlohmann::json& cfg = *emitter.config;
//	float rate = cfg["modules"]["emission"]["rate"].get<float>();
//	accumEmit += rate * delta;
//	unsigned int emit = static_cast<unsigned int>(accumEmit);
//	accumEmit -= emit;
//	// burst add to emit
//	if (cfg["modules"]["emission"].contains("bursts"))
//	{
//		// similar
//	}
//	if (emit > 0)
//	{
//		RaylibWrapper::rlEnableShader(computeEmit.id);
//		int locEmitCount = RaylibWrapper::GetShaderLocation(computeEmit, "emitCount");
//		RaylibWrapper::SetShaderValue(computeEmit, locEmitCount, &emit, SHADER_UNIFORM_INT);
//		unsigned int groups = (emit + 63) / 64;
//		RaylibWrapper::rlComputeShaderDispatch(groups, 1, 1);
//		RaylibWrapper::rlDisableShader();
//	}
//	RaylibWrapper::rlEnableShader(computeSim.id);
//	unsigned int simGroups = (maxParticles + 63) / 64;
//	RaylibWrapper::rlComputeShaderDispatch(simGroups, 1, 1);
//	RaylibWrapper::rlDisableShader();
//	emitter.currentPing = 1 - emitter.currentPing;
//}
//void ParticleRenderer::RenderGPU(const EmitterInstance& emitter)
//{
//	RaylibWrapper::BeginShaderMode(particleShader);
//	int locLit = RaylibWrapper::GetShaderLocation(particleShader, "lit");
//	RaylibWrapper::SetShaderValue(particleShader, locLit, &lit, SHADER_UNIFORM_INT);
//	// set other uniforms like mvp, viewPos, lightDir
//	Vector3 camPos = SceneManager::GetActiveScene()->GetCamera().position;
//	int locViewPos = RaylibWrapper::GetShaderLocation(particleShader, "viewPos");
//	RaylibWrapper::SetShaderValue(particleShader, locViewPos, &camPos, SHADER_UNIFORM_VEC3);
//	// similar for light
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, emitter.aliveSSBO[emitter.currentPing]);
//	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, emitter.indirectSSBO);
//	glDrawArraysIndirect(GL_TRIANGLES, 0);
//	RaylibWrapper::EndShaderMode();
//	// children sub if gpu support
//}
//void ParticleRenderer::InitializeGPU(EmitterInstance& emitter)
//{
//	emitter.particleSSBO = RaylibWrapper::rlLoadShaderBuffer(maxParticles * sizeof(ParticleGPU), NULL, RL_DYNAMIC_COPY);
//	emitter.aliveSSBO[0] = RaylibWrapper::rlLoadShaderBuffer(maxParticles * sizeof(unsigned int), NULL, RL_DYNAMIC_COPY);
//	emitter.aliveSSBO[1] = RaylibWrapper::rlLoadShaderBuffer(maxParticles * sizeof(unsigned int), NULL, RL_DYNAMIC_COPY);
//	emitter.deadSSBO = RaylibWrapper::rlLoadShaderBuffer(maxParticles * sizeof(unsigned int), NULL, RL_DYNAMIC_COPY);
//	emitter.counterSSBO = RaylibWrapper::rlLoadShaderBuffer(sizeof(CountersGPU), NULL, RL_DYNAMIC_COPY);
//	emitter.indirectSSBO = RaylibWrapper::rlLoadShaderBuffer(sizeof(IndirectDrawGPU), NULL, RL_DYNAMIC_COPY);
//	unsigned int* deadData = new unsigned int[maxParticles];
//	for (unsigned int i = 0; i < maxParticles; i++) deadData[i] = i;
//	RaylibWrapper::rlUpdateShaderBuffer(emitter.deadSSBO, deadData, maxParticles * sizeof(unsigned int), 0);
//	delete[] deadData;
//	CountersGPU counters = { 0, 0, maxParticles };
//	RaylibWrapper::rlUpdateShaderBuffer(emitter.counterSSBO, &counters, sizeof(counters), 0);
//	IndirectDrawGPU indirect = { 6, 0, 0, 0 };
//	RaylibWrapper::rlUpdateShaderBuffer(emitter.indirectSSBO, &indirect, sizeof(indirect), 0);
//	emitter.currentPing = 0;
//	emitter.accumEmit = 0.0f;
//}
//void ParticleRenderer::DestroyGPU(EmitterInstance& emitter)
//{
//	RaylibWrapper::rlUnloadShaderBuffer(emitter.particleSSBO);
//	RaylibWrapper::rlUnloadShaderBuffer(emitter.aliveSSBO[0]);
//	RaylibWrapper::rlUnloadShaderBuffer(emitter.aliveSSBO[1]);
//	RaylibWrapper::rlUnloadShaderBuffer(emitter.deadSSBO);
//	RaylibWrapper::rlUnloadShaderBuffer(emitter.counterSSBO);
//	RaylibWrapper::rlUnloadShaderBuffer(emitter.indirectSSBO);
//}
//void ParticleRenderer::SetUniforms(EmitterInstance& emitter)
//{
//	// Set all uniforms for computeEmit and computeSim from cfg
//	// Example
//	float dt = RaylibWrapper::GetFrameTime();
//	int locDt = RaylibWrapper::GetShaderLocation(computeSim, "dt");
//	RaylibWrapper::SetShaderValue(computeSim, locDt, &dt, SHADER_UNIFORM_FLOAT);
//	// similarly for others: rate, shape params, curve points as array, etc.
//	// For particleShader too
//}
//Vector3 EmitterInstance::GetWorldPosition(const ParticleRenderer* renderer)
//{
//	Vector3 pos = localPosition;
//	EmitterInstance* par = parent;
//	while (par)
//	{
//		pos = Vector3Add(pos, par->localPosition);
//		par = par->parent;
//	}
//	if (renderer->worldSpace)
//		pos = Vector3Add(pos, renderer->gameObject->transform.GetPosition());
//	return pos;
//}
//void EmitterInstance::ClearParticles()
//{
//	for (auto& p : particles) p.active = false;
//	for (auto* child : children) child->ClearParticles();
//	for (auto* sub : subEmitters) sub->ClearParticles();
//}
//Vector3 ParticleRenderer::GetSpawnPosition(const nlohmann::json& shape, const Vector3& emitterPos)
//{
//	std::string type = shape["type"].get<std::string>();
//	if (type == "point")
//		return emitterPos;
//	if (type == "sphere")
//	{
//		float r = shape["radius"].get<float>();
//		Vector3 dir = Vector3Normalize({ GetRandomValue(-1,1), GetRandomValue(-1,1), GetRandomValue(-1,1) });
//		float len = GetRandomValue(0, r) if volume else r;
//		return Vector3Add(emitterPos, Vector3Scale(dir, len));
//	}
//	// Todo: box, cone, cylinder, mesh, etc.
//	return emitterPos;
//}
//float ParticleRenderer::SampleCurve(float t, const nlohmann::json& curve)
//{
//	if (curve.empty()) return 1.0f;
//	for (size_t i = 0; i < curve.size() - 1; i++)
//	{
//		if (t >= curve[i][0] && t < curve[i + 1][0])
//		{
//			float frac = (t - curve[i][0].get<float>()) / (curve[i + 1][0].get<float>() - curve[i][0].get<float>());
//			return curve[i][1].get<float>() + frac * (curve[i + 1][1].get<float>() - curve[i][1].get<float>());
//		}
//	}
//	return curve.back()[1].get<float>();
//}
//Color ParticleRenderer::SampleGradient(float t, const nlohmann::json& gradient)
//{
//	if (gradient.empty()) return WHITE;
//	for (size_t i = 0; i < gradient.size() - 1; i++)
//	{
//		if (t >= gradient[i][0] && t < gradient[i + 1][0])
//		{
//			float frac = (t - gradient[i][0].get<float>()) / (gradient[i + 1][0].get<float>() - gradient[i][0].get<float>());
//			Color c1 = { gradient[i][1][0], gradient[i][1][1], gradient[i][1][2], gradient[i][1][3] };
//			Color c2 = { gradient[i + 1][1][0], gradient[i + 1][1][1], gradient[i + 1][1][2], gradient[i + 1][1][3] };
//			return { (unsigned char)(c1.r + frac * (c2.r - c1.r)), similarly for g,b,a };
//		}
//	}
//	Color c = { gradient.back()[1][0],[1],[2],[3] };
//	return c;
//}
//void ParticleRenderer::ApplyForces(Particle& p, const nlohmann::json& forces, float delta)
//{
//	// Todo: for each force field, apply based on type, falloff
//}
//void ParticleRenderer::HandleCollision(Particle& p, Vector3 oldPos, const nlohmann::json& collision)
//{
//	if (!collision["enabled"].get<bool>()) return;
//	// Todo: Ray ray = {oldPos, p.velocity * delta};
//	// RayCollision hit = GetCollisionRayScene(ray); // Assume function
//	// if (hit.hit) p.velocity = Vector3Reflect(p.velocity, hit.normal) * collision["bouncy"].get<float>();
//	// or kill, stick, etc.
//}
//void ParticleRenderer::TriggerSubEmitters(const EmitterInstance& emitter, const Particle& p, const std::string& trigger)
//{
//	nlohmann::json& cfg = *emitter.config;
//	if (cfg["sub_emitters"].empty()) return;
//	for (auto& sub : cfg["sub_emitters"])
//	{
//		if (sub["trigger"].get<std::string>() == trigger)
//		{
//			EmitterInstance* newSub = new EmitterInstance();
//			InitializeEmitter(*newSub, &sub["emitter"], const_cast<EmitterInstance*>(&emitter));
//			newSub->localPosition = p.position; // Todo: offset
//			emitter.subEmitters.push_back(newSub);
//		}
//	}
//}
//void ParticleRenderer::SortParticles(std::vector<Particle*>& sorted, const Camera3D& camera)
//{
//	std::sort(sorted.begin(), sorted.end(), [&](const Particle* a, const Particle* b) {
//		return Vector3DistanceSqr(camera.position, a->position) > Vector3DistanceSqr(camera.position, b->position);
//		});
//}
//void ParticleRenderer::DrawParticle(const Particle& p, const nlohmann::json& appearance, const Camera3D& camera)
//{
//	std::string mode = appearance["billboard_mode"].get<std::string>();
//	Vector3 pos = worldSpace ? p.position : gameObject->transform.TransformPoint(p.position);
//	Vector3 direction = Vector3Normalize(Vector3Subtract(camera.position, pos));
//	Vector3 up = camera.up;
//	Vector3 right = Vector3Normalize(Vector3CrossProduct(up, direction));
//	up = Vector3CrossProduct(direction, right);
//	if (mode == "velocity_aligned")
//	{
//		up = Vector3Normalize(p.velocity);
//		right = Vector3Normalize(Vector3CrossProduct(up, direction));
//		up = Vector3CrossProduct(direction, right);
//	}
//	else if (mode == "stretched")
//	{
//		float speed = Vector3Length(p.velocity);
//		float stretch = appearance["stretch_factor"].get<float>() * speed;
//		right = Vector3Scale(right, stretch);
//	} // Todo: fixed rotation
//	Matrix rot = MatrixRotateZ(p.rotation * DEG2RAD);
//	right = Vector3Transform(right, rot);
//	up = Vector3Transform(up, rot);
//	Vector3 v1 = Vector3Add(pos, Vector3Subtract(Vector3Scale(right, p.size / 2), Vector3Scale(up, p.size / 2)));
//	Vector3 v2 = Vector3Add(pos, Vector3Add(Vector3Scale(right, p.size / 2), Vector3Scale(up, p.size / 2)));
//	Vector3 v3 = Vector3Add(pos, Vector3Add(Vector3Scale(right, p.size / 2), Vector3Scale(up, p.size / 2)));
//	Vector3 v4 = Vector3Add(pos, Vector3Subtract(Vector3Scale(right, p.size / 2), Vector3Scale(up, p.size / 2)));
//	rlColor4ub(p.color.r, p.color.g, p.color.b, p.color.a);
//	rlTexCoord2f(0, 0); rlVertex3f(v1.x, v1.y, v1.z);
//	rlTexCoord2f(1, 0); rlVertex3f(v2.x, v2.y, v2.z);
//	rlTexCoord2f(1, 1); rlVertex3f(v3.x, v3.y, v3.z);
//	rlTexCoord2f(0, 1); rlVertex3f(v4.x, v4.y, v4.z);
//	// Todo: for atlas, adjust uv based on frame
//}
//void ParticleRenderer::UpdateSubEmitters(EmitterInstance& emitter, float delta)
//{
//	for (auto* sub : emitter.subEmitters)
//	{
//		SpawnParticles(*sub, delta);
//		UpdateParticles(*sub, delta);
//		UpdateSubEmitters(*sub, delta);
//		if (sub->time > sub->config->["modules"]["emission"]["duration"].get<float>())
//		{
//			// Delete sub if done
//			delete sub;
//			// Remove from list
//		}
//	}
//}
//// Advanced GPU
//void ParticleRenderer::InitializeGPUSimulation()
//{
//	// Todo: Use rlgl to create compute shader, buffers for particles
//}
//void ParticleRenderer::UpdateGPUSimulation(float delta)
//{
//	// Todo: Dispatch compute
//}
//void ParticleRenderer::RenderGPUParticles()
//{
//	// Todo: Draw with instance buffer from GPU
//}