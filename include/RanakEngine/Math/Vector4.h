#ifndef VEC4_H
#define VEC4_H

#include "GLM/vec4.hpp"

#include <string>
#include <cmath>

struct Vector2;
struct Vector3;

/**
 * @struct Vector4
 * @brief Custom 4D vector struct providing interoperability with GLM.
 * 
 * This struct bridges the gap between the GLM vec4 type,
 * allowing seamless conversion throughout the engine.
 * Commonly used for RGBA colors and homogeneous coordinates.
 */
struct Vector4
{
    union {
        struct { float x, y, z, w; };  ///< Vector components
        struct { float r, g, b, a; };  ///< Color component aliases (RGBA)
    };

    /**
     * @brief Creates a zero vector.
     */
    Vector4();

    /**
     * @brief Creates a vector with all components set to the same value.
     * 
     * @param _value The value for all components.
     */
    Vector4(float _value);

    /**
     * @brief Creates a vector with given components.
     * 
     * @param _x The X component (or R for colors).
     * @param _y The Y component (or G for colors).
     * @param _z The Z component (or B for colors).
     * @param _w The W component (or A for colors).
     */
    Vector4(float _x, float _y, float _z, float _w);

    /**
     * @brief Creates a vector from a GLM vec4.
     * 
     * @param _v The GLM vector to copy from.
     */
    Vector4(glm::vec4 _v);

    Vector4(Vector2 _xy, Vector2 _zw);

    Vector4(Vector2 _xy, float _z, float _w);

    Vector4(Vector3 _xyz, float _w);

    /**
     * @brief Converts to GLM vec4.
     */
    explicit operator glm::vec4() const
    {
        return glm::vec4(x, y, z, w);
    };

    /**
     * @brief Calculates the length of the vector.
     * 
     * @return float The magnitude of the vector.
     */
    float Length();

    /**
     * @brief Normalises this vector in place.
     */
    void Normalise();

    /**
     * @brief Returns a normalised copy of this vector.
     * 
     * @return Vector4 The normalised vector.
     */
    Vector4 Normalised();

    /**
     * @brief Converts the vector to a string representation.
     * 
     * @return std::string The formatted string "(x, y, z, w)".
     */
    std::string ToString() const
    {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w) + ")";
    }

    /// @brief Multiplies this vector by a scalar.
    /// @param _value The scalar to multiply by.
    /// @return The scaled vector.
    Vector4 operator* (float _value) const
    {
        return Vector4(x * _value, y * _value, z * _value, w * _value);
    };

    /// @brief Divides this vector by a scalar.
    /// @param _value The scalar to divide by.
    /// @return The divided vector.
    Vector4 operator/ (float _value) const
    {
        return Vector4(x / _value, y / _value, z / _value, w / _value);
    };

    /// @brief Adds another vector to this vector.
    /// @param _vector The vector to add.
    /// @return Reference to this vector.
    Vector4& operator+= (const Vector4& _vector)
    {
        this->x += _vector.x;
        this->y += _vector.y;
        this->z += _vector.z;
        this->w += _vector.w;
        return *this;
    };

    /// @brief Subtracts another vector from this vector.
    /// @param _vector The vector to subtract.
    /// @return Reference to this vector.
    Vector4& operator-= (const Vector4& _vector)
    {
        this->x -= _vector.x;
        this->y -= _vector.y;
        this->z -= _vector.z;
        this->w -= _vector.w;
        return *this;
    };

    /// @brief Component-wise multiplication with another vector.
    /// @param _vector The vector to multiply with.
    /// @return Reference to this vector.
    Vector4& operator*= (const Vector4& _vector)
    {
        this->x *= _vector.x;
        this->y *= _vector.y;
        this->z *= _vector.z;
        this->w *= _vector.w;
        return *this;
    };

    /// @brief Negates this vector.
    /// @return The negated vector.
    Vector4 operator-() const
    {
        return Vector4(-x, -y, -z, -w);
    };
};

/// @brief Adds a scalar to each component of a vector.
/// @param _scalar The scalar value.
/// @param _vector The vector.
/// @return The resulting vector.
inline Vector4 operator+(float _scalar, const Vector4& _vector)
{
    return Vector4(_scalar + _vector.x, _scalar + _vector.y, _scalar + _vector.z, _scalar + _vector.w);
};

/// @brief Subtracts each component of a vector from a scalar.
/// @param _scalar The scalar value.
/// @param _vector The vector.
/// @return The resulting vector.
inline Vector4 operator-(float _scalar, const Vector4& _vector)
{
    return Vector4(_scalar - _vector.x, _scalar - _vector.y, _scalar - _vector.z, _scalar - _vector.w);
};

/// @brief Multiplies a scalar by each component of a vector.
/// @param _scalar The scalar value.
/// @param _vector The vector.
/// @return The resulting vector.
inline Vector4 operator*(float _scalar, const Vector4& _vector)
{
    return Vector4(_scalar * _vector.x, _scalar * _vector.y, _scalar * _vector.z, _scalar * _vector.w);
};

/// @brief Divides a scalar by each component of a vector.
/// @param _scalar The scalar value.
/// @param _vector The vector.
/// @return The resulting vector.
inline Vector4 operator/(float _scalar, const Vector4& _vector)
{
    return Vector4(_scalar / _vector.x, _scalar / _vector.y, _scalar / _vector.z, _scalar / _vector.w);
};

/// @brief Adds two vectors component-wise.
/// @param _a First vector.
/// @param _b Second vector.
/// @return The sum of the two vectors.
inline Vector4 operator+(const Vector4& _a, const Vector4& _b)
{
    return Vector4(_a.x + _b.x, _a.y + _b.y, _a.z + _b.z, _a.w + _b.w);
};

/// @brief Subtracts two vectors component-wise.
/// @param _a First vector.
/// @param _b Second vector.
/// @return The difference of the two vectors.
inline Vector4 operator-(const Vector4& _a, const Vector4& _b)
{
    return Vector4(_a.x - _b.x, _a.y - _b.y, _a.z - _b.z, _a.w - _b.w);
};

/// @brief Multiplies two vectors component-wise.
/// @param _a First vector.
/// @param _b Second vector.
/// @return The component-wise product.
inline Vector4 operator*(const Vector4& _a, const Vector4& _b)
{
    return Vector4(_a.x * _b.x, _a.y * _b.y, _a.z * _b.z, _a.w * _b.w);
};

/// @brief Divides two vectors component-wise.
/// @param _a First vector.
/// @param _b Second vector.
/// @return The component-wise quotient.
inline Vector4 operator/(const Vector4& _a, const Vector4& _b)
{
    return Vector4(_a.x / _b.x, _a.y / _b.y, _a.z / _b.z, _a.w / _b.w);
};

#endif
