#include "CryonicCore.h"
#include <cmath>

float fixedDeltaTime = 0.0f;
float deltaTime = 0.0f;

Vector3 RotateVector3ByQuaternion(Vector3 vector, Quaternion quaternion)
{
    Vector3 result = { 0 };

    result.x = vector.x * (quaternion.x * quaternion.x + quaternion.w * quaternion.w - quaternion.y * quaternion.y - quaternion.z * quaternion.z) + vector.y * (2 * quaternion.x * quaternion.y - 2 * quaternion.w * quaternion.z) + vector.z * (2 * quaternion.x * quaternion.z + 2 * quaternion.w * quaternion.y);
    result.y = vector.x * (2 * quaternion.w * quaternion.z + 2 * quaternion.x * quaternion.y) + vector.y * (quaternion.w * quaternion.w - quaternion.x * quaternion.x + quaternion.y * quaternion.y - quaternion.z * quaternion.z) + vector.z * (-2 * quaternion.w * quaternion.x + 2 * quaternion.y * quaternion.z);
    result.z = vector.x * (-2 * quaternion.w * quaternion.y + 2 * quaternion.x * quaternion.z) + vector.y * (2 * quaternion.w * quaternion.x + 2 * quaternion.y * quaternion.z) + vector.z * (quaternion.w * quaternion.w - quaternion.x * quaternion.x - quaternion.y * quaternion.y + quaternion.z * quaternion.z);

    return result;
}

// Todo: Make this take in degrees instaed of radios. Will need to update all code using this function
Quaternion EulerToQuaternion(float roll, float pitch, float yaw)
{
    Quaternion result = { 0 };

    float x0 = cosf(roll * 0.5f);
    float x1 = sinf(roll * 0.5f);
    float y0 = cosf(pitch * 0.5f);
    float y1 = sinf(pitch * 0.5f);
    float z0 = cosf(yaw * 0.5f);
    float z1 = sinf(yaw * 0.5f);

    result.x = x1 * y0 * z0 - x0 * y1 * z1;
    result.y = x0 * y1 * z0 + x1 * y0 * z1;
    result.z = x0 * y0 * z1 - x1 * y1 * z0;
    result.w = x0 * y0 * z0 + x1 * y1 * z1;

    return result;
}


// Todo: Make this return degrees instaed of radios. Will need to update all code using this function
Vector3 QuaternionToEuler(Quaternion quaternion)
{
    Vector3 result = { 0 };

    // Roll (x-axis rotation)
    float x0 = 2.0f * (quaternion.w * quaternion.x + quaternion.y * quaternion.z);
    float x1 = 1.0f - 2.0f * (quaternion.x * quaternion.x + quaternion.y * quaternion.y);
    result.x = atan2f(x0, x1);

    // Pitch (y-axis rotation)
    float y0 = 2.0f * (quaternion.w * quaternion.y - quaternion.z * quaternion.x);
    y0 = y0 > 1.0f ? 1.0f : y0;
    y0 = y0 < -1.0f ? -1.0f : y0;
    result.y = asinf(y0);

    // Yaw (z-axis rotation)
    float z0 = 2.0f * (quaternion.w * quaternion.z + quaternion.x * quaternion.y);
    float z1 = 1.0f - 2.0f * (quaternion.y * quaternion.y + quaternion.z * quaternion.z);
    result.z = atan2f(z0, z1);

    // Normalize euler angles
    //static float max = 6.2831853072f;
    //result.x = fmod(fmod(result.x, max) + max, max);
    //result.y = fmod(fmod(result.y, max) + max, max);
    //result.z = fmod(fmod(result.z, max) + max, max);

    return result;
}

void NormalizeEuler(Vector3& euler)
{
    euler.x = ((int)euler.x % 360 + 360) % 360;
    euler.y = ((int)euler.y % 360 + 360) % 360;
    euler.z = ((int)euler.z % 360 + 360) % 360;
}

float GetDeltaTime()
{
    return deltaTime;
}

float GetFixedDeltaTime()
{
    return fixedDeltaTime;
}