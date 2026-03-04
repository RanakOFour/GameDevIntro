#ifndef QUAT_H
#define QUAT_H

#include "GLM/gtc/quaternion.hpp"
#include "RanakEngine/Math/Vector3.h"

#include <string>

/**
 * @struct Quaternion
 * @brief Custom quaternion struct providing interoperability between GLM and ReactPhysics3D.
 * 
 * This struct bridges the gap between the GLM and ReactPhysics3D quaternion types,
 * allowing seamless conversion between them throughout the engine.
 */
struct Quaternion
{
    float w, x, y, z;   ///< Quaternion components

    /**
     * @brief Creates a quaternion representing a rotation around an axis.
     * 
     * @param _angle Rotation angle in radians.
     * @param _axis The axis to rotate around.
     */
    Quaternion(float _angle, Vector3 _axis);

    /**
     * @brief Creates a quaternion with all components set to the same value.
     * 
     * @param _value The value for all components.
     */
    Quaternion(float _value);

    /**
     * @brief Creates an identity quaternion.
     */
    Quaternion();

    /**
     * @brief Creates a quaternion with directly assigned component values.
     * 
     * @param _x The X component.
     * @param _y The Y component.
     * @param _z The Z component.
     * @param _w The W component.
     */
    Quaternion(float _x, float _y, float _z, float _w);

    /**
     * @brief Creates a quaternion from Euler angles.
     * 
     * @param _pitch Rotation around X axis in radians.
     * @param _yaw Rotation around Y axis in radians.
     * @param _roll Rotation around Z axis in radians.
     */
    Quaternion(float _pitch, float _yaw, float _roll);

    /**
     * @brief Creates a quaternion from Euler angles as a vector.
     * 
     * @param _eulers Vector containing (pitch, yaw, roll) in radians.
     */
    Quaternion(Vector3 _eulers);

    /**
     * @brief Creates a quaternion from a GLM quat.
     * 
     * @param _q The GLM quaternion to copy from.
     */
    Quaternion(glm::quat _q);

    /// @brief Multiplies this quaternion by another quaternion.
    /// @param q The quaternion to multiply with.
    /// @return The resulting quaternion.
    Quaternion operator * (const Quaternion& q);

    /// @brief Multiplies this quaternion by another and assigns the result.
    /// @param q The quaternion to multiply with.
    /// @return Reference to this quaternion.
    Quaternion& operator *= (const Quaternion& q);

    /**
     * @brief Converts to GLM quat.
     */
    operator glm::quat() const;

    /**
     * @brief Normalises this quaternion in place.
     */
    void Normalise();

    /**
     * @brief Returns a normalised copy of this quaternion.
     * 
     * @return Quaternion The normalised quaternion.
     */
    Quaternion Normalised() const;

    /**
     * @brief Calculates the dot product with another quaternion.
     * 
     * @param _q The other quaternion.
     * 
     * @return float The dot product.
     */
    float Dot(Quaternion _q);

    /**
     * @brief Converts the quaternion to Euler angles.
     * 
     * @return Vector3 The Euler angles (pitch, yaw, roll) in radians.
     */
    Vector3 EulerAngles();

    /**
     * @brief Converts to GLM quat explicitly.
     * 
     * @return glm::quat The GLM quaternion.
     */
    glm::quat ToGlm() const
    {
        return (glm::quat)(*this);
    };

    /**
     * @brief Converts the quaternion to a string representation.
     * 
     * @return std::string The formatted string.
     */
    std::string ToString();
};

#endif