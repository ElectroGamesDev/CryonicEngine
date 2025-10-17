#pragma once
#include "Raylib/RaylibWrapper.h"
#include "Components/Component.h"
#include <vector>
#include <memory>

struct SplinePoint
{
    Vector3 position;
    Vector3 tangent;
    Vector3 rotation;
    float scale;
};

class Spline : public Component
{
public:
    Spline(GameObject* obj, int id) : Component(obj, id)
    {
        name = "Spline";
        iconUnicode = "\xee\x84\xbb";
        runInEditor = true;
        Awake();

        std::string variables = R"(
        [
            0,
            [
                [
                    "vector",
                    "points",
                    {},
                    "Points"
                ]
            ]
        ]
    )";
        exposedVariables = nlohmann::json::parse(variables);
    }

    void AddPoint(const Vector3& position, const Vector3& tangent = { 1,0,0 });
    void RemovePoint(int index);
    void ClearPoints();

    SplinePoint& GetPoint(int index);
    const std::vector<SplinePoint>& GetPoints() const { return points; }
    int GetPointCount() const { return (int)points.size(); }

    void SetClosedLoop(bool closed);
    bool IsClosedLoop() const { return closedLoop; }

    float GetSplineLength() const;
    float GetSegmentLength(int segmentIndex) const;

private:

    std::vector<SplinePoint> points;
    bool closedLoop = false;

    Vector3 EvaluatePosition(float t) const;
    Vector3 EvaluateTangent(float t) const;
    Vector3 EvaluateNormal(float t, const Vector3& up = { 0,1,0 });
    Vector3 EvaluateBinormal(float t, const Vector3& up = { 0,1,0 });
    //Matrix EvaluateTransform(float t, const Vector3& up = { 0,1,0 });

    int GetSegmentIndex(float t) const;
    float GetSegmentT(float t) const;
    void CalculateSegmentLengths() const;
    mutable std::vector<float> segmentLengths;
    mutable bool lengthsDirty = true;
};