#include "RanakEngine/Math/Vector4.h"
#include "RanakEngine/Math/Vector3.h"
#include "RanakEngine/Math/Vector2.h"

#include <cmath>

Vector4::Vector4()
: x(0.0f)
, y(0.0f)
, z(0.0f)
, w(0.0f)
{

};

Vector4::Vector4(float _value)
: x(_value)
, y(_value)
, z(_value)
, w(_value)
{
};

Vector4::Vector4(float _x, float _y, float _z, float _w)
: x(_x)
, y(_y)
, z(_z)
, w(_w)
{

}

Vector4::Vector4(glm::vec4 _v)
: x(_v.x)
, y(_v.y)
, z(_v.z)
, w(_v.w)
{

};

Vector4::Vector4(Vector2 _xy, Vector2 _zw)
{
    x = _xy.x;
    y = _xy.y;
    z = _zw.x;
    w = _zw.y;
}

Vector4::Vector4(Vector2 _xy, float _z, float _w)
{
    x = _xy.x;
    y = _xy.y;
    z = _z;
    w = _w;
}

Vector4::Vector4(Vector3 _xyz, float _w)
{
    x = _xyz.x;
    y = _xyz.y;
    z = _xyz.z;
    w = _w;
}

float Vector4::Length()
{
    return std::sqrt(x * x + y * y + z * z + w * w);
};

void Vector4::Normalise()
{
    float l_magInv = 1.0f / Length();
    x *= l_magInv;
    y *= l_magInv;
    z *= l_magInv;
    w *= l_magInv;
}

Vector4 Vector4::Normalised()
{
    float l_magInv = 1.0f / Length();

    return Vector4(x * l_magInv, y * l_magInv, z * l_magInv, w * l_magInv);
};
