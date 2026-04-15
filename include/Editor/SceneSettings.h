#ifndef SCENESETTINGS_H
#define SCENESETTINGS_H

/**
 * @enum GizmoMode
 * @brief The active transform gizmo shown over the selected entity.
 */
enum class GizmoMode
{
    Translate, ///< Move arrows (X red, Y green).
    Rotate,    ///< Rotation circle.
    Scale      ///< Scale corner squares.
};

/**
 * @struct SceneSettings
 * @brief Per-scene simulation and rendering settings serialized inside .lua scene files.
 *
 * Gravity controls the Box2D world force applied during simulation.
 * Clear color is the OpenGL background colour rendered each frame.
 * Both are written by SceneSerializer::Serialize() and read back by
 * SceneSerializer::LoadFromString() / LoadFromFile().
 */
struct SceneSettings
{
    float gravityX    =  0.0f; ///< World gravity X component (Box2D units/s²).
    float gravityY    = -9.8f; ///< World gravity Y component. Negative = downward.

    float clearColorR = 0.2f;  ///< Background clear colour, red   [0, 1].
    float clearColorG = 0.2f;  ///< Background clear colour, green [0, 1].
    float clearColorB = 0.4f;  ///< Background clear colour, blue  [0, 1].
    float clearColorA = 1.0f;  ///< Background clear colour, alpha [0, 1].

    // -- Editor gizmo / snap settings (not serialized to scene files) --
    GizmoMode gizmoMode = GizmoMode::Translate; ///< Active gizmo tool.
    bool  snapEnabled   = false;                ///< Whether snap-to-grid is active.
    float snapGridSize  = 1.0f;                 ///< Grid cell size for snap (world units).
};

#endif
