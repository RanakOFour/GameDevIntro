#include "RanakEngine/Math/Vector2.h"

#include <cmath>

Vector2::Vector2()
: x(0.0f)
, y(0.0f)
{

};

Vector2::Vector2(float _value)
: x(_value)
, y(_value)
{
};

Vector2::Vector2(float _x, float _y)
: x(_x)
, y(_y)
{

}

Vector2::Vector2(glm::vec2 _v)
: x(_v.x)
, y(_v.y)
{

};

float Vector2::Length()
{
    return std::sqrt(x * x + y * y);
};

void Vector2::Normalise()
{
    float l_magInv = 1.0f / Length();
    x *= l_magInv;
    y *= l_magInv;
}

Vector2 Vector2::Normalised()
{
    float l_magInv = 1.0f / Length();

    return Vector2(x * l_magInv, y * l_magInv);
};
