#ifndef VEC3_H
#define VEC3_H

#include "GLM/vec3.hpp"

#include <string>

struct Vector2;

/**
 * @struct Vector3
 * @brief Custom 3D vector struct providing interoperability between GLM and ReactPhysics3D.
 * 
 * This struct bridges the gap between the GLM and ReactPhysics3D vector types,
 * allowing seamless conversion between them throughout the engine.
 */
struct Vector3
{
    union
    {
        struct { float x, y, z; };  ///< Standard component names
        struct { float r, g, b; };  ///< Color component aliases (RGB)
    };

    /**
     * @brief Creates a zero vector.
     */
    Vector3();

    /**
     * @brief Creates a vector with all components set to the same value.
     * 
     * @param _value The value for all components.
     */
    Vector3(float _value);

    /**
     * @brief Creates a vector with given components.
     * 
     * @param _xy The X and Y components as a Vector2.
     * @param _z The Z component.
     */
    Vector3(Vector2 _xy, float _z);

    /**
     * @brief Creates a vector with given components.
     * 
     * @param _x The X component.
     * @param _y The Y component.
     * @param _z The Z component.
     */
    Vector3(float _x, float _y, float _z);

    /**
     * @brief Creates a vector from a GLM vec3.
     * 
     * @param _v The GLM vector to copy from.
     */
    Vector3(glm::vec3 _v);

    /**
     * @brief Converts to GLM vec3.
     */
    explicit operator glm::vec3() const
    {
        return glm::vec3(x, y, z);
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
     * @return Vector3 The normalised vector.
     */
    Vector3 Normalised();

    /**
     * @brief Converts the vector to a string representation.
     * 
     * @return std::string The formatted string "(x, y, z)".
     */
    std::string ToString() const
    {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }

    /// @brief Multiplies this vector by a scalar.
    /// @param _value The scalar to multiply by.
    /// @return The scaled vector.
    Vector3 operator* (float _value) const
    {
        return Vector3(x * _value, y * _value, z * _value);
    };

    /// @brief Adds another vector to this vector.
    /// @param _vector The vector to add.
    /// @return Reference to this vector.
    Vector3& operator+= (const Vector3& _vector)
    {
        this->x += _vector.x;
        this->y += _vector.y;
        this->z += _vector.z;
        return *this;
    };

    /// @brief Subtracts another vector from this vector.
    /// @param _vector The vector to subtract.
    /// @return Reference to this vector.
    Vector3& operator-= (const Vector3& _vector)
    {
        this->x -= _vector.x;
        this->y -= _vector.y;
        this->z -= _vector.z;
        return *this;
    };

    /// @brief Component-wise multiplication with another vector.
    /// @param _vector The vector to multiply with.
    /// @return Reference to this vector.
    Vector3& operator*= (const Vector3& _vector)
    {
        this->x *= _vector.x;
        this->y *= _vector.y;
        this->z *= _vector.z;
        return *this;
    };

    /// @brief Negates this vector.
    /// @return The negated vector.
    Vector3 operator-() const
    {
        return Vector3(-x, -y, -z);
    };
};

/// @brief Adds a scalar to each component of a vector.
/// @param _scalar The scalar value.
/// @param _vector The vector.
/// @return The resulting vector.
inline Vector3 operator+(float _scalar, const Vector3& _vector)
{
    return Vector3(_scalar + _vector.x, _scalar + _vector.y, _scalar + _vector.z);
};

/// @brief Subtracts each component of a vector from a scalar.
/// @param _scalar The scalar value.
/// @param _vector The vector.
/// @return The resulting vector.
inline Vector3 operator-(float _scalar, const Vector3& _vector)
{
    return Vector3(_scalar - _vector.x, _scalar - _vector.y, _scalar - _vector.z);
};

/// @brief Multiplies a scalar by each component of a vector.
/// @param _scalar The scalar value.
/// @param _vector The vector.
/// @return The resulting vector.
inline Vector3 operator*(float _scalar, const Vector3& _vector)
{
    return Vector3(_scalar * _vector.x, _scalar * _vector.y, _scalar * _vector.z);
};

/// @brief Divides a scalar by each component of a vector.
/// @param _scalar The scalar value.
/// @param _vector The vector.
/// @return The resulting vector.
inline Vector3 operator/(float _scalar, const Vector3& _vector)
{
    return Vector3(_scalar / _vector.x, _scalar / _vector.y, _scalar / _vector.z);
};

/// @brief Adds two vectors component-wise.
/// @param _a First vector.
/// @param _b Second vector.
/// @return The sum of the two vectors.
inline Vector3 operator+(const Vector3& _a, const Vector3& _b)
{
    return Vector3(_a.x + _b.x, _a.y + _b.y, _a.z + _b.z);
};

/// @brief Subtracts two vectors component-wise.
/// @param _a First vector.
/// @param _b Second vector.
/// @return The difference of the two vectors.
inline Vector3 operator-(const Vector3& _a, const Vector3& _b)
{
    return Vector3(_a.x - _b.x, _a.y - _b.y, _a.z - _b.z);
};

/// @brief Multiplies two vectors component-wise.
/// @param _a First vector.
/// @param _b Second vector.
/// @return The component-wise product.
inline Vector3 operator*(const Vector3& _a, const Vector3& _b)
{
    return Vector3(_a.x * _b.x, _a.y * _b.y, _a.z * _b.z);
};

/// @brief Divides two vectors component-wise.
/// @param _a First vector.
/// @param _b Second vector.
/// @return The component-wise quotient.
inline Vector3 operator/(const Vector3& _a, const Vector3& _b)
{
    return Vector3(_a.x / _b.x, _a.y / _b.y, _a.z / _b.z);
};

#endif