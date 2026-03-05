#include "RanakEngine/Math/Quaternion.h"

#if WIN32
#define M_PI 3.14159265358979323846
#endif

Quaternion::Quaternion(float _angle, Vector3 _axis)
{
    float l_halfAngle = _angle * 0.5f;
    float l_sin = sin(l_halfAngle);

    x = _axis.x * l_sin;
    y = _axis.y * l_sin;
    z = _axis.z * l_sin;
    w = cos(l_halfAngle);
};

Quaternion::Quaternion(float _value)
: x(_value)
, y(_value)
, z(_value)
, w(_value)
{
};

Quaternion::Quaternion()
: x(0.0f)
, y(0.0f)
, z(0.0f)
, w(1.0f)
{
};

Quaternion::Quaternion(float _x, float _y, float _z, float _w)
: x(_x)
, y(_y)
, z(_z)
, w(_w)
{
};

Quaternion::Quaternion(float _pitch, float _yaw, float _roll)
{
    float cy = cos(_yaw * 0.5f);
    float sy = sin(_yaw * 0.5f);
    float cp = cos(_pitch * 0.5f);
    float sp = sin(_pitch * 0.5f);
    float cr = cos(_roll * 0.5f);
    float sr = sin(_roll * 0.5f);

    w = cr * cp * cy + sr * sp * sy;
    x = sr * cp * cy - cr * sp * sy;
    y = cr * sp * cy + sr * cp * sy;
    z = cr * cp * sy - sr * sp * cy;
};



Quaternion::Quaternion(Vector3 _eulers)
{
    float cy = cos(_eulers.y * 0.5f);
    float sy = sin(_eulers.y * 0.5f);
    float cp = cos(_eulers.x * 0.5f);
    float sp = sin(_eulers.x * 0.5f);
    float cr = cos(_eulers.z * 0.5f);
    float sr = sin(_eulers.z * 0.5f);

    w = cr * cp * cy + sr * sp * sy;
    x = sr * cp * cy - cr * sp * sy;
    y = cr * sp * cy + sr * cp * sy;
    z = cr * cp * sy - sr * sp * cy;
};

Quaternion::Quaternion(glm::quat _q)
: x(_q.x)
, y(_q.y)
, z(_q.z)
, w(_q.w)
{

}

Quaternion Quaternion::operator * (const Quaternion& q)
{
    return Quaternion(
        w * q.x + x * q.w + y * q.z - z * q.y,
		w * q.y + y * q.w + z * q.x - x * q.z,
		w * q.z + z * q.w + x * q.y - y * q.x,
        w * q.w - x * q.x - y * q.y - z * q.z
    );
};

Quaternion& Quaternion::operator *= (const Quaternion& q)
{
    *this = *this * q;
    return *this;
};

Quaternion::operator glm::quat() const
{
    // GLM quat constructor order: (w, x, y, z)
    return glm::quat(w, x, y, z);
};

void Quaternion::Normalise()
{
    float length = std::sqrt(Dot(*this));
    float l_oneOverLength = 1.0f / length;
    w *= l_oneOverLength;
    x *= l_oneOverLength;
    y *= l_oneOverLength;
    z *= l_oneOverLength;
};

Quaternion Quaternion::Normalised() const
{
    Quaternion q = *this;
    q.Normalise();
    return q;
};

float Quaternion::Dot(Quaternion _q)
{
    return x * _q.x + y * _q.y + z * _q.z + w * _q.w;
};

// Convert quaternion to Euler angles consistent with the constructor
// Constructor uses: x = pitch, y = yaw, z = roll
Vector3 Quaternion::EulerAngles()
{
    Vector3 l_eulers;

    // Pitch (x-axis rotation, stored in eulers.x)
    float sinp = 2 * (w * x + y * z);
    float cosp = 1 - 2 * (x * x + y * y);
    l_eulers.x = std::atan2(sinp, cosp);

    // Yaw (y-axis rotation, stored in eulers.y)
    float siny = 2 * (w * y - z * x);
    siny = std::max(-1.0f, std::min(1.0f, siny));
    l_eulers.y = std::asin(siny);

    // Roll (z-axis rotation, stored in eulers.z)
    float sinr = 2 * (w * z + x * y);
    float cosr = 1 - 2 * (y * y + z * z);
    l_eulers.z = std::atan2(sinr, cosr);

    return l_eulers;
};

std::string Quaternion::ToString()
{
    return "(" + std::to_string(x)  + ", " + std::to_string(y)  + ", " +std::to_string(z)  + ", " + std::to_string(w)  + ")";
};