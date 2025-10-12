#pragma once
#include <vector>
#include <unordered_map>
#include "Animation.h"
#include "RaylibWrapper.h"

class MotionMatchingSystem
{
public:
	struct Pose
	{
		std::vector<RaylibWrapper::Vector3> jointPositions;
		std::vector<RaylibWrapper::Vector3> jointVelocities;
		RaylibWrapper::Vector3 rootPosition;
		RaylibWrapper::Vector3 rootVelocity;
		float trajectoryDirection; // Future trajectory
	};

	struct PoseFeature
	{
		int animationId;
		int frameIndex;
		float timeInAnimation;
		Pose pose;
		float cost; // For matching
	};

	struct MotionQuery
	{
		RaylibWrapper::Vector3 desiredVelocity;
		RaylibWrapper::Vector3 desiredFacingDirection;
		RaylibWrapper::Vector3 currentPosition;
		RaylibWrapper::Vector3 currentVelocity;
		std::vector<RaylibWrapper::Vector3> trajectoryPoints; // Future trajectory
	};

	MotionMatchingSystem();
	~MotionMatchingSystem();

	// Build motion database from animations
	void BuildDatabase(const std::vector<Animation*>& animations);
	void AddAnimationToDatabase(Animation* animation);
	void ClearDatabase();

	// Find best matching pose
	PoseFeature* FindBestMatch(const MotionQuery& query, int currentAnimId = -1, float currentTime = 0.0f);

	// Calculate pose difference cost
	float ComputePoseCost(const Pose& a, const Pose& b, const MotionQuery& query);

	// Settings
	void SetWeights(float posWeight, float velWeight, float trajWeight);
	void SetSearchRadius(float radius) { searchRadius = radius; }
	void SetMinSearchTime(float time) { minSearchTime = time; }

	// Database management
	void SaveDatabase(const std::string& path);
	void LoadDatabase(const std::string& path);

	const std::vector<PoseFeature>& GetDatabase() const { return poseDatabase; }

private:
	std::vector<PoseFeature> poseDatabase;

	// Matching weights
	float positionWeight = 1.0f;
	float velocityWeight = 1.5f;
	float trajectoryWeight = 2.0f;

	// Search parameters
	float searchRadius = 0.5f; // Time radius to search around current frame
	float minSearchTime = 0.1f; // Minimum time before allowing transition

	Pose ExtractPoseFromAnimation(Animation* anim, int frameIndex);
	float CalculateTrajectoryScore(const Pose& pose, const MotionQuery& query);
};
