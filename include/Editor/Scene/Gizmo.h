#ifndef GIZMO_H
#define GIZMO_H

#include "./SceneSettings.h"

#include "RanakEngine/Math.h"

#include "imgui/imgui.h"

/**
 * @class Gizmo
 * @brief Renders 2D transform gizmo handles over the selected entity and
 *        performs hit-testing so the gizmo intercepts clicks before scene
 *        raycasting.
 *
 * All rendering is done through ImGui's background draw list so it
 * overlays the OpenGL scene without requiring additional shaders.
 */

class Gizmo
{
public:
    /** Axis / handle that was clicked. */
    enum class Axis { None, X, Y, XY, Rotate, ScaleX, ScaleY, ScaleXY };

    /** Handle length in screen pixels. */
    static constexpr float k_HandleLength = 80.0f;
    /** Hit-test tolerance in screen pixels. */
    static constexpr float k_HitRadius    = 10.0f;
    /** Rotate circle radius in screen pixels. */
    static constexpr float k_RotateRadius = 60.0f;
    /** Scale square half-size in screen pixels. */
    static constexpr float k_ScaleBoxHalf = 7.0f;

    /**
     * @brief Renders the gizmo for the current mode at @p _screenPos.
     * @param _mode     Active gizmo mode (translate / rotate / scale).
     * @param _screenPos Screen-space centre of the selected entity.
     * @param _entityScale Screen-space half extents of the entity (pixels).
     * @param _activeAxis The axis currently being dragged (highlighted).
     */
    static void Draw(GizmoMode _mode, ImVec2 _screenPos, ImVec2 _entityScale,
                     Axis _activeAxis = Axis::None);

    /**
     * @brief Tests whether @p _mouseScreen hits a gizmo handle.
     * @param _mode   Active gizmo mode.
     * @param _entityScreenPos Screen-space entity centre (same value passed to Draw).
     * @param _entityScale Screen-space half extents (same value passed to Draw).
     * @param _mouseScreen     Screen-space click position.
     * @return The axis/handle under the cursor, or Axis::None.
     */
    static Axis HitTest(GizmoMode _mode, ImVec2 _entityScreenPos,
                        ImVec2 _entityScale, ImVec2 _mousePos);

    /**
     * @brief Constrains a world-space position delta to the dragged axis.
     * @param _axis    Axis being dragged.
     * @param _delta   Raw world-space mouse delta (x, y).
     * @return Constrained delta.
     */
    static Vector2 ConstrainDelta(Axis _axis, Vector2 _delta);

    /**
     * @brief Snaps a world position to the nearest grid intersection.
     * @param _pos      World position.
     * @param _gridSize Grid cell size.
     * @return Snapped position.
     */
    static Vector2 Snap(Vector2 _pos, float _gridSize);
};

#endif
