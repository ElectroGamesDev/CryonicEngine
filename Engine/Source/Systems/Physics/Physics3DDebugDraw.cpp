//#include "Physics3DDebugDraw.h"
//#include "ThirdParty/Raylib/include/raylib.h"
//#include "ThirdParty/Raylib/include/rlgl.h"
//#include "Utilities/ConsoleLogger.h"
//#include "ThirdParty/Raylib/include/raymath.h"
//
//void Physics3DDebugDraw::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
//{
//    Vector3 from = {
//        static_cast<float>(inFrom.GetX()),
//        static_cast<float>(inFrom.GetY()),
//        static_cast<float>(inFrom.GetZ())
//    };
//    Vector3 to = {
//        static_cast<float>(inTo.GetX()),
//        static_cast<float>(inTo.GetY()),
//        static_cast<float>(inTo.GetZ())
//    };
//
//    Color rayColor = {
//        static_cast<unsigned char>(inColor.r * 255),
//        static_cast<unsigned char>(inColor.g * 255),
//        static_cast<unsigned char>(inColor.b * 255),
//        static_cast<unsigned char>(inColor.a * 255)
//    };
//
//    DrawLine3D(from, to, {0, 255, 0, 255});
//}
//
//void Physics3DDebugDraw::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow)
//{
//    Vector3 v1 = {
//        static_cast<float>(inV1.GetX()),
//        static_cast<float>(inV1.GetY()),
//        static_cast<float>(inV1.GetZ())
//    };
//    Vector3 v2 = {
//        static_cast<float>(inV2.GetX()),
//        static_cast<float>(inV2.GetY()),
//        static_cast<float>(inV2.GetZ())
//    };
//    Vector3 v3 = {
//        static_cast<float>(inV3.GetX()),
//        static_cast<float>(inV3.GetY()),
//        static_cast<float>(inV3.GetZ())
//    };
//
//    Color rayColor = {
//        static_cast<unsigned char>(inColor.r * 255),
//        static_cast<unsigned char>(inColor.g * 255),
//        static_cast<unsigned char>(inColor.b * 255),
//        static_cast<unsigned char>(inColor.a * 255)
//    };
//
//    DrawTriangle3D(v1, v2, v3, {0, 255, 0, 25});
//}
//
//void Physics3DDebugDraw::DrawText3D(JPH::RVec3Arg inPosition, const JPH::string_view& inString, JPH::ColorArg inColor, float inHeight)
//{
//    Vector3 pos = {
//        static_cast<float>(inPosition.GetX()),
//        static_cast<float>(inPosition.GetY()),
//        static_cast<float>(inPosition.GetZ())
//    };
//
//    Color rayColor = {
//        static_cast<unsigned char>(inColor.r * 255),
//        static_cast<unsigned char>(inColor.g * 255),
//        static_cast<unsigned char>(inColor.b * 255),
//        static_cast<unsigned char>(inColor.a * 255)
//    };
//
//    DrawText(inString.data(),
//        static_cast<int>(pos.x),
//        static_cast<int>(pos.y),
//        static_cast<int>(inHeight),
//        rayColor);
//}