#ifndef VEC2_H
#define VEC2_H

#include "GLM/vec2.hpp"

#include <string>
#include <cmath>

/**
 * @struct Vector2
 * @brief Custom 2D vector struct providing interoperability with GLM.
 * 
 * This struct bridges the gap between the GLM vec2 type,
 * allowing seamless conversion throughout the engine.
 */
struct Vector2
{
    float x, y;  ///< Vector components

    /**
     * @brief Creates a zero vector.
     */
    Vector2();

    /**
     * @brief Creates a vector with all components set to the same value.
     * 
     * @param _value The value for all components.
     */
    Vector2(float _value);

    /**
     * @brief Creates a vector with given components.
     * 
     * @param _x The X component.
     * @param _y The Y component.
     */
    Vector2(float _x, float _y);

    /**
     * @brief Creates a vector from a GLM vec2.
     * 
     * @param _v The GLM vector to copy from.
     */
    Vector2(glm::vec2 _v);

    /**
     * @brief Converts to GLM vec2.
     */
    explicit operator glm::vec2() const
    {
        return glm::vec2(x, y);
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
     * @return Vector2 The normalised vector.
     */
    Vector2 Normalised();

    /**
     * @brief Converts the vector to a string representation.
     * 
     * @return std::string The formatted string "(x, y)".
     */
    std::string ToString() const
    {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }

    /// @brief Multiplies this vector by a scalar.
    /// @param _value The scalar to multiply by.
    /// @return The scaled vector.
    Vector2 operator* (float _value) const
    {
        return Vector2(x * _value, y * _value);
    };

    /// @brief Divides this vector by a scalar.
    /// @param _value The scalar to divide by.
    /// @return The divided vector.
    Vector2 operator/ (float _value) const
    {
        return Vector2(x / _value, y / _value);
    };

    /// @brief Adds another vector to this vector.
    /// @param _vector The vector to add.
    /// @return Reference to this vector.
    Vector2& operator+= (const Vector2& _vector)
    {
        this->x += _vector.x;
        this->y += _vector.y;
        return *this;
    };

    /// @brief Subtracts another vector from this vector.
    /// @param _vector The vector to subtract.
    /// @return Reference to this vector.
    Vector2& operator-= (const Vector2& _vector)
    {
        this->x -= _vector.x;
        this->y -= _vector.y;
        return *this;
    };

    /// @brief Component-wise multiplication with another vector.
    /// @param _vector The vector to multiply with.
    /// @return Reference to this vector.
    Vector2& operator*= (const Vector2& _vector)
    {
        this->x *= _vector.x;
        this->y *= _vector.y;
        return *this;
    };

    /// @brief Negates this vector.
    /// @return The negated vector.
    Vector2 operator-() const
    {
        return Vector2(-x, -y);
    };
};

/// @brief Adds a scalar to each component of a vector.
/// @param _scalar The scalar value.
/// @param _vector The vector.
/// @return The resulting vector.
inline Vector2 operator+(float _scalar, const Vector2& _vector)
{
    return Vector2(_scalar + _vector.x, _scalar + _vector.y);
};

/// @brief Subtracts each component of a vector from a scalar.
/// @param _scalar The scalar value.
/// @param _vector The vector.
/// @return The resulting vector.
inline Vector2 operator-(float _scalar, const Vector2& _vector)
{
    return Vector2(_scalar - _vector.x, _scalar - _vector.y);
};

/// @brief Multiplies a scalar by each component of a vector.
/// @param _scalar The scalar value.
/// @param _vector The vector.
/// @return The resulting vector.
inline Vector2 operator*(float _scalar, const Vector2& _vector)
{
    return Vector2(_scalar * _vector.x, _scalar * _vector.y);
};

/// @brief Divides a scalar by each component of a vector.
/// @param _scalar The scalar value.
/// @param _vector The vector.
/// @return The resulting vector.
inline Vector2 operator/(float _scalar, const Vector2& _vector)
{
    return Vector2(_scalar / _vector.x, _scalar / _vector.y);
};

/// @brief Adds two vectors component-wise.
/// @param _a First vector.
/// @param _b Second vector.
/// @return The sum of the two vectors.
inline Vector2 operator+(const Vector2& _a, const Vector2& _b)
{
    return Vector2(_a.x + _b.x, _a.y + _b.y);
};

/// @brief Subtracts two vectors component-wise.
/// @param _a First vector.
/// @param _b Second vector.
/// @return The difference of the two vectors.
inline Vector2 operator-(const Vector2& _a, const Vector2& _b)
{
    return Vector2(_a.x - _b.x, _a.y - _b.y);
};

/// @brief Multiplies two vectors component-wise.
/// @param _a First vector.
/// @param _b Second vector.
/// @return The component-wise product.
inline Vector2 operator*(const Vector2& _a, const Vector2& _b)
{
    return Vector2(_a.x * _b.x, _a.y * _b.y);
};

/// @brief Divides two vectors component-wise.
/// @param _a First vector.
/// @param _b Second vector.
/// @return The component-wise quotient.
inline Vector2 operator/(const Vector2& _a, const Vector2& _b)
{
    return Vector2(_a.x / _b.x, _a.y / _b.y);
};

#endif
