#include "MotionMatchingSystem.h"
#include "ConsoleLogger.h"
#include <algorithm>
#include <cmath>
#include <fstream>

MotionMatchingSystem::MotionMatchingSystem()
{
}

MotionMatchingSystem::~MotionMatchingSystem()
{
	ClearDatabase();
}

void MotionMatchingSystem::BuildDatabase(const std::vector<Animation*>& animations)
{
	ClearDatabase();

	for (Animation* anim : animations)
	{
		if (anim)
			AddAnimationToDatabase(anim);
	}

	ConsoleLogger::InfoLog("Motion Matching Database built with " + std::to_string(poseDatabase.size()) + " poses");
}

void MotionMatchingSystem::AddAnimationToDatabase(Animation* animation)
{
	if (!animation || animation->GetSprites().empty())
		return;

	int frameCount = static_cast<int>(animation->GetSprites().size());
	float frameDuration = animation->GetSpeed() / frameCount;

	for (int i = 0; i < frameCount; ++i)
	{
		PoseFeature feature;
		feature.animationId = animation->GetId();
		feature.frameIndex = i;
		feature.timeInAnimation = i * frameDuration;
		feature.pose = ExtractPoseFromAnimation(animation, i);
		feature.cost = 0.0f;

		poseDatabase.push_back(feature);
	}
}

MotionMatchingSystem::Pose MotionMatchingSystem::ExtractPoseFromAnimation(Animation* anim, int frameIndex)
{
	Pose pose;

	// For 2D sprite animations, we create simplified pose data
	// In a full 3D system, this would extract actual skeleton joint data

	// Simplified 2D pose extraction
	pose.rootPosition = RaylibWrapper::Vector3{ 0.0f, 0.0f, 0.0f };
	pose.rootVelocity = RaylibWrapper::Vector3{ 0.0f, 0.0f, 0.0f };
	pose.trajectoryDirection = 0.0f;

	// Calculate velocity based on frame progression
	int nextFrame = (frameIndex + 1) % anim->GetSprites().size();
	float dt = anim->GetSpeed() / anim->GetSprites().size();

	if (dt > 0.0f)
	{
		// Estimate velocity from frame difference
		// This is a simplified version - in full 3D you'd use actual pose data
		pose.rootVelocity.x = (nextFrame - frameIndex) / dt;
	}

	return pose;
}

MotionMatchingSystem::PoseFeature* MotionMatchingSystem::FindBestMatch(const MotionQuery& query, int currentAnimId, float currentTime)
{
	if (poseDatabase.empty())
		return nullptr;

	PoseFeature* bestMatch = nullptr;
	float bestCost = std::numeric_limits<float>::max();

	for (auto& feature : poseDatabase)
	{
		// Skip poses too close to current time in same animation
		if (feature.animationId == currentAnimId)
		{
			float timeDiff = std::abs(feature.timeInAnimation - currentTime);
			if (timeDiff < minSearchTime)
				continue;
		}

		// Compute matching cost
		float cost = ComputePoseCost(feature.pose, Pose(), query);
		feature.cost = cost;

		if (cost < bestCost)
		{
			bestCost = cost;
			bestMatch = &feature;
		}
	}

	return bestMatch;
}

float MotionMatchingSystem::ComputePoseCost(const Pose& candidatePose, const Pose& currentPose, const MotionQuery& query)
{
	float totalCost = 0.0f;

	// Position difference cost
	RaylibWrapper::Vector3 positionDiff = {
		candidatePose.rootPosition.x - query.currentPosition.x,
		candidatePose.rootPosition.y - query.currentPosition.y,
		candidatePose.rootPosition.z - query.currentPosition.z
	};
	float positionCost = std::sqrt(positionDiff.x * positionDiff.x +
		positionDiff.y * positionDiff.y +
		positionDiff.z * positionDiff.z);
	totalCost += positionCost * positionWeight;

	// Velocity difference cost
	RaylibWrapper::Vector3 velocityDiff = {
		candidatePose.rootVelocity.x - query.desiredVelocity.x,
		candidatePose.rootVelocity.y - query.desiredVelocity.y,
		candidatePose.rootVelocity.z - query.desiredVelocity.z
	};
	float velocityCost = std::sqrt(velocityDiff.x * velocityDiff.x +
		velocityDiff.y * velocityDiff.y +
		velocityDiff.z * velocityDiff.z);
	totalCost += velocityCost * velocityWeight;

	// Trajectory cost
	float trajectoryCost = CalculateTrajectoryScore(candidatePose, query);
	totalCost += trajectoryCost * trajectoryWeight;

	return totalCost;
}

float MotionMatchingSystem::CalculateTrajectoryScore(const Pose& pose, const MotionQuery& query)
{
	if (query.trajectoryPoints.empty())
		return 0.0f;

	float totalError = 0.0f;

	// Compare candidate trajectory direction with desired trajectory
	for (const auto& point : query.trajectoryPoints)
	{
		RaylibWrapper::Vector3 diff = {
			point.x - pose.rootPosition.x,
			point.y - pose.rootPosition.y,
			point.z - pose.rootPosition.z
		};

		float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
		totalError += distance;
	}

	return totalError / query.trajectoryPoints.size();
}

void MotionMatchingSystem::SetWeights(float posWeight, float velWeight, float trajWeight)
{
	positionWeight = posWeight;
	velocityWeight = velWeight;
	trajectoryWeight = trajWeight;
}

void MotionMatchingSystem::ClearDatabase()
{
	poseDatabase.clear();
}

void MotionMatchingSystem::SaveDatabase(const std::string& path)
{
	std::ofstream file(path, std::ios::binary);
	if (!file.is_open())
	{
		ConsoleLogger::ErrorLog("Failed to save motion matching database: " + path);
		return;
	}

	size_t count = poseDatabase.size();
	file.write(reinterpret_cast<const char*>(&count), sizeof(count));

	for (const auto& feature : poseDatabase)
	{
		file.write(reinterpret_cast<const char*>(&feature.animationId), sizeof(feature.animationId));
		file.write(reinterpret_cast<const char*>(&feature.frameIndex), sizeof(feature.frameIndex));
		file.write(reinterpret_cast<const char*>(&feature.timeInAnimation), sizeof(feature.timeInAnimation));
		file.write(reinterpret_cast<const char*>(&feature.pose), sizeof(feature.pose));
	}

	file.close();
	ConsoleLogger::InfoLog("Motion matching database saved: " + path);
}

void MotionMatchingSystem::LoadDatabase(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
	{
		ConsoleLogger::ErrorLog("Failed to load motion matching database: " + path);
		return;
	}

	ClearDatabase();

	size_t count;
	file.read(reinterpret_cast<char*>(&count), sizeof(count));
	poseDatabase.resize(count);

	for (auto& feature : poseDatabase)
	{
		file.read(reinterpret_cast<char*>(&feature.animationId), sizeof(feature.animationId));
		file.read(reinterpret_cast<char*>(&feature.frameIndex), sizeof(feature.frameIndex));
		file.read(reinterpret_cast<char*>(&feature.timeInAnimation), sizeof(feature.timeInAnimation));
		file.read(reinterpret_cast<char*>(&feature.pose), sizeof(feature.pose));
	}

	file.close();
	ConsoleLogger::InfoLog("Motion matching database loaded: " + path + " (" + std::to_string(count) + " poses)");
}