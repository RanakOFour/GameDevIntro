#include "RanakEngine/Math/Vector3.h"
#include "RanakEngine/Math/Vector2.h"
#include <cmath>

Vector3::Vector3()
: x(0.0f)
, y(0.0f)
, z(0.0f)
{

};

Vector3::Vector3(float _value)
: x(_value)
, y(_value)
, z(_value)
{
};

Vector3::Vector3(Vector2 _xy, float _z)
{
    x = _xy.x;
    y = _xy.y;
    z = _z;
}

Vector3::Vector3(float _x, float _y, float _z)
: x(_x)
, y(_y)
, z(_z)
{

}

Vector3::Vector3(glm::vec3 _v)
: x(_v.x)
, y(_v.y)
, z(_v.z)
{

};

float Vector3::Length()
{
    return std::sqrt(x * x + y * y + z * z);
};

void Vector3::Normalise()
{
    float l_magInv = 1.0f / Length();
    x *= l_magInv;
    y *= l_magInv;
    z *= l_magInv;
}

Vector3 Vector3::Normalised()
{
    float l_magInv = 1.0f / Length();

    return Vector3(x * l_magInv, y * l_magInv, z * l_magInv);
};