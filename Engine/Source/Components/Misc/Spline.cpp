//#include "Spline.h"
//#include <cmath>
//#include <algorithm>
//
//void Spline::AddPoint(const Vector3& position, const Vector3& tangent)
//{
//    points.emplace_back(position, tangent);
//    lengthsDirty = true;
//}
//
//void Spline::RemovePoint(int index)
//{
//    if (index >= 0 && index < (int)points.size())
//    {
//        points.erase(points.begin() + index);
//        lengthsDirty = true;
//    }
//}
//
//void Spline::ClearPoints()
//{
//    points.clear();
//    lengthsDirty = true;
//}
//
//SplinePoint& Spline::GetPoint(int index)
//{
//    static SplinePoint defaultPoint;
//    if (index >= 0 && index < (int)points.size())
//        return points[index];
//    return defaultPoint;
//}
//
//void Spline::SetClosedLoop(bool closed)
//{
//    closedLoop = closed;
//    lengthsDirty = true;
//}
//
//int Spline::GetSegmentIndex(float t) const
//{
//    if (points.empty()) return -1;
//
//    if (closedLoop)
//    {
//        t = fmod(t, 1.0f);
//        if (t < 0) t += 1.0f;
//        return (int)(t * points.size()) % points.size();
//    }
//    else
//    {
//        t = std::clamp(t, 0.0f, 1.0f);
//        float segmentCount = (float)(points.size() - 1);
//        return (int)(t * segmentCount);
//    }
//}
//
//float Spline::GetSegmentT(float t) const
//{
//    if (points.empty()) return 0.0f;
//
//    if (closedLoop)
//    {
//        t = fmod(t, 1.0f);
//        if (t < 0) t += 1.0f;
//        return t * points.size() - floor(t * points.size());
//    }
//    else
//    {
//        t = std::clamp(t, 0.0f, 1.0f);
//        float segmentCount = (float)(points.size() - 1);
//        return t * segmentCount - floor(t * segmentCount);
//    }
//}
//
//Vector3 Spline::EvaluatePosition(float t)
//{
//    if (points.size() < 2) return points.empty() ? Vector3{ 0,0,0 } : points[0].position;
//
//    int segmentIndex = GetSegmentIndex(t);
//    float segmentT = GetSegmentT(t);
//
//    if (closedLoop || (segmentIndex < (int)points.size() - 1))
//    {
//        int nextIndex = (segmentIndex + 1) % points.size();
//
//        const Vector3& p0 = points[segmentIndex].position;
//        const Vector3& p1 = points[nextIndex].position;
//        const Vector3& m0 = points[segmentIndex].tangent;
//        const Vector3& m1 = points[nextIndex].tangent;
//
//        // Cubic Hermite spline
//        float t2 = segmentT * segmentT;
//        float t3 = t2 * segmentT;
//
//        float h1 = 2.0f * t3 - 3.0f * t2 + 1.0f;
//        float h2 = -2.0f * t3 + 3.0f * t2;
//        float h3 = t3 - 2.0f * t2 + segmentT;
//        float h4 = t3 - t2;
//
//        return Vector3{
//            h1 * p0.x + h2 * p1.x + h3 * m0.x + h4 * m1.x,
//            h1 * p0.y + h2 * p1.y + h3 * m0.y + h4 * m1.y,
//            h1 * p0.z + h2 * p1.z + h3 * m0.z + h4 * m1.z
//        };
//    }
//
//    return points.back().position;
//}
//
//Vector3 Spline::EvaluateTangent(float t)
//{
//    if (points.size() < 2) return { 1,0,0 };
//
//    int segmentIndex = GetSegmentIndex(t);
//    float segmentT = GetSegmentT(t);
//
//    if (closedLoop || (segmentIndex < (int)points.size() - 1))
//    {
//        int nextIndex = (segmentIndex + 1) % points.size();
//
//        const Vector3& p0 = points[segmentIndex].position;
//        const Vector3& p1 = points[nextIndex].position;
//        const Vector3& m0 = points[segmentIndex].tangent;
//        const Vector3& m1 = points[nextIndex].tangent;
//
//        // Derivative of cubic Hermite spline
//        float t2 = segmentT * segmentT;
//
//        float h1 = 6.0f * t2 - 6.0f * segmentT;
//        float h2 = -6.0f * t2 + 6.0f * segmentT;
//        float h3 = 3.0f * t2 - 4.0f * segmentT + 1.0f;
//        float h4 = 3.0f * t2 - 2.0f * segmentT;
//
//        Vector3 tangent{
//            h1 * p0.x + h2 * p1.x + h3 * m0.x + h4 * m1.x,
//            h1 * p0.y + h2 * p1.y + h3 * m0.y + h4 * m1.y,
//            h1 * p0.z + h2 * p1.z + h3 * m0.z + h4 * m1.z
//        };
//
//        return RaylibWrapper::Vector3Normalize(tangent);
//    }
//
//    return { 1,0,0 };
//}
//
//Vector3 Spline::EvaluateNormal(float t, const Vector3& up)
//{
//    Vector3 tangent = EvaluateTangent(t);
//    Vector3 binormal = RaylibWrapper::Vector3Cross(up, tangent);
//    return RaylibWrapper::Vector3Cross(tangent, binormal);
//}
//
//Vector3 Spline::EvaluateBinormal(float t, const Vector3& up)
//{
//    Vector3 tangent = EvaluateTangent(t);
//    return RaylibWrapper::Vector3Normalize(RaylibWrapper::Vector3Cross(up, tangent));
//}
//
////Matrix Spline::EvaluateTransform(float t, const Vector3& up)
////{
////    Vector3 position = EvaluatePosition(t);
////    Vector3 tangent = EvaluateTangent(t);
////    Vector3 normal = EvaluateNormal(t, up);
////    Vector3 binormal = EvaluateBinormal(t, up);
////
////    return Matrix{
////        binormal.x, normal.x, tangent.x, position.x,
////        binormal.y, normal.y, tangent.y, position.y,
////        binormal.z, normal.z, tangent.z, position.z,
////        0, 0, 0, 1
////    };
////}
//
//void Spline::CalculateSegmentLengths() const
//{
//    if (!lengthsDirty) return;
//
//    segmentLengths.clear();
//
//    if (points.size() < 2)
//    {
//        lengthsDirty = false;
//        return;
//    }
//
//    const int steps = 10;
//    float totalLength = 0.0f;
//
//    int segmentCount = closedLoop ? (int)points.size() : (int)points.size() - 1;
//
//    for (int i = 0; i < segmentCount; i++)
//    {
//        float segmentLength = 0.0f;
//        Vector3 prevPoint = points[i].position;
//
//        for (int j = 1; j <= steps; j++)
//        {
//            float t = (float)j / (float)steps;
//            Vector3 currentPoint = EvaluatePosition((float)i + t);
//            segmentLength += RaylibWrapper::Vector3Distance(prevPoint, currentPoint);
//            prevPoint = currentPoint;
//        }
//
//        segmentLengths.push_back(segmentLength);
//        totalLength += segmentLength;
//    }
//
//    lengthsDirty = false;
//}
//
//float Spline::GetSplineLength() const
//{
//    CalculateSegmentLengths();
//
//    float totalLength = 0.0f;
//    for (float length : segmentLengths)
//        totalLength += length;
//
//    return totalLength;
//}
//
//float Spline::GetSegmentLength(int segmentIndex) const
//{
//    CalculateSegmentLengths();
//
//    if (segmentIndex >= 0 && segmentIndex < (int)segmentLengths.size())
//        return segmentLengths[segmentIndex];
//    return 0.0f;
//}
//
//void Spline::DrawSpline(Color color) const
//{
//    if (points.size() < 2) return;
//
//    const int segments = 20 * (int)points.size();
//
//    Vector3 prevPoint = EvaluatePosition(0.0f);
//    for (int i = 1; i <= segments; i++)
//    {
//        float t = (float)i / (float)segments;
//        Vector3 currentPoint = EvaluatePosition(t);
//        RaylibWrapper::DrawLine3D(prevPoint, currentPoint, color);
//        prevPoint = currentPoint;
//    }
//}
//
//void Spline::DrawTangents(Color color) const
//{
//    for (const SplinePoint& point : points)
//    {
//        RaylibWrapper::DrawLine3D(point.position,
//            RaylibWrapper::Vector3Add(point.position, point.tangent),
//            color);
//    }
//}
//
//void Spline::DrawNormals(Color color) const
//{
//    const int segments = 10 * (int)points.size();
//
//    for (int i = 0; i <= segments; i++)
//    {
//        float t = (float)i / (float)segments;
//        Vector3 position = EvaluatePosition(t);
//        Vector3 normal = EvaluateNormal(t);
//        RaylibWrapper::DrawLine3D(position,
//            RaylibWrapper::Vector3Add(position, RaylibWrapper::Vector3Scale(normal, 0.5f)),
//            color);
//    }
//}