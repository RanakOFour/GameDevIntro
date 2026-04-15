#include "Editor/Gizmo.h"

#include <cmath>
#include <algorithm>

static const ImU32 k_RedActive    = IM_COL32(255, 80, 80, 255);
static const ImU32 k_RedDim       = IM_COL32(200, 60, 60, 180);
static const ImU32 k_GreenActive  = IM_COL32(80, 255, 80, 255);
static const ImU32 k_GreenDim     = IM_COL32(60, 200, 60, 180);
static const ImU32 k_YellowActive = IM_COL32(255, 255, 80, 255);
static const ImU32 k_YellowDim    = IM_COL32(200, 200, 60, 180);
static const ImU32 k_CircleColor  = IM_COL32(200, 200, 255, 180);
static const ImU32 k_CircleHi     = IM_COL32(220, 220, 255, 255);

static float Dist(ImVec2 a, ImVec2 b)
{
    float dx = a.x - b.x, dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

static float PointToSegmentDist(ImVec2 p, ImVec2 a, ImVec2 b)
{
    float abx = b.x - a.x, aby = b.y - a.y;
    float apx = p.x - a.x, apy = p.y - a.y;
    float t = (apx * abx + apy * aby) / (abx * abx + aby * aby + 1e-8f);
    t = std::clamp(t, 0.0f, 1.0f);
    float cx = a.x + t * abx - p.x;
    float cy = a.y + t * aby - p.y;
    return std::sqrt(cx * cx + cy * cy);
}

static void DrawTranslate(ImDrawList* _dl, ImVec2 _centre, ImVec2 _entityScale, Gizmo::Axis _active)
{
    const float L = Gizmo::k_HandleLength;
    const float arrowHead = 12.0f;

    // X axis (red, rightward)
    ImU32 xCol = (_active == Gizmo::Axis::X || _active == Gizmo::Axis::XY)
                 ? k_RedActive : k_RedDim;
    ImVec2 xEnd(_centre.x + L, _centre.y);
    _dl->AddLine(_centre, xEnd, xCol, 2.5f);
    _dl->AddTriangleFilled(
        ImVec2(xEnd.x + arrowHead, xEnd.y),
        ImVec2(xEnd.x - 4, xEnd.y - 6),
        ImVec2(xEnd.x - 4, xEnd.y + 6),
        xCol);

    // Y axis (green, upward)
    ImU32 yCol = (_active == Gizmo::Axis::Y || _active == Gizmo::Axis::XY)
                 ? k_GreenActive : k_GreenDim;
    ImVec2 yEnd(_centre.x, _centre.y - L);
    _dl->AddLine(_centre, yEnd, yCol, 2.5f);
    _dl->AddTriangleFilled(
        ImVec2(yEnd.x, yEnd.y - arrowHead),
        ImVec2(yEnd.x - 6, yEnd.y + 4),
        ImVec2(yEnd.x + 6, yEnd.y + 4),
        yCol);

    // XY box at corner (free translate)
    float boxSize = 18.0f;
    ImU32 xyCol = (_active == Gizmo::Axis::XY) ? k_YellowActive : k_YellowDim;
    _dl->AddRectFilled(
        ImVec2(_centre.x + 6, _centre.y - 6 - boxSize),
        ImVec2(_centre.x + 6 + boxSize, _centre.y - 6),
        xyCol);
}

static void DrawRotate(ImDrawList* _dl, ImVec2 _centre, ImVec2 _entityScale, Gizmo::Axis _active)
{
    ImU32 col = (_active == Gizmo::Axis::Rotate) ? k_CircleHi : k_CircleColor;
    float radius = std::max(Gizmo::k_RotateRadius, std::max(_entityScale.x, _entityScale.y) + 15.0f);
    _dl->AddCircle(_centre, radius, col, 48, 2.5f);
}

static void DrawScale(ImDrawList* _dl, ImVec2 _centre, ImVec2 _entityScale, Gizmo::Axis _active)
{
    const float h = Gizmo::k_ScaleBoxHalf;

    // X axis line + box — endpoint at entity's right edge
    ImU32 xCol = (_active == Gizmo::Axis::ScaleX || _active == Gizmo::Axis::ScaleXY)
                 ? k_RedActive : k_RedDim;
    ImVec2 xEnd(_centre.x + _entityScale.x, _centre.y);
    _dl->AddLine(_centre, xEnd, xCol, 2.0f);
    _dl->AddRectFilled(ImVec2(xEnd.x - h, xEnd.y - h),
                       ImVec2(xEnd.x + h, xEnd.y + h), xCol);

    // Y axis line + box — endpoint at entity's top edge
    ImU32 yCol = (_active == Gizmo::Axis::ScaleY || _active == Gizmo::Axis::ScaleXY)
                 ? k_GreenActive : k_GreenDim;
    ImVec2 yEnd(_centre.x, _centre.y - _entityScale.y);
    _dl->AddLine(_centre, yEnd, yCol, 2.0f);
    _dl->AddRectFilled(ImVec2(yEnd.x - h, yEnd.y - h),
                       ImVec2(yEnd.x + h, yEnd.y + h), yCol);

    // Centre box (uniform scale)
    ImU32 xyCol = (_active == Gizmo::Axis::ScaleXY) ? k_YellowActive : k_YellowDim;
    _dl->AddRectFilled(ImVec2(_centre.x - h, _centre.y - h),
                       ImVec2(_centre.x + h, _centre.y + h), xyCol);
}

void Gizmo::Draw(GizmoMode _mode, ImVec2 _screenPos, ImVec2 _entityScale, Axis _activeAxis)
{
    ImDrawList* l_dl = ImGui::GetBackgroundDrawList();

    switch (_mode)
    {
        case GizmoMode::Translate:
            DrawTranslate(l_dl, _screenPos, _entityScale, _activeAxis);
            break;
        case GizmoMode::Rotate:
            DrawRotate(l_dl, _screenPos, _entityScale, _activeAxis);
            break;
        case GizmoMode::Scale:
            DrawScale(l_dl, _screenPos, _entityScale, _activeAxis);
            break;
    }
}

static Gizmo::Axis HitTestTranslate(ImVec2 _centre, ImVec2 _entityScale, ImVec2 _mousePos)
{
    // XY box
    float l_boxSize = 18.0f;
    if (_mousePos.x >= _centre.x + 6 && _mousePos.x <= _centre.x + 6 + l_boxSize &&
        _mousePos.y >= _centre.y - 6 - l_boxSize && _mousePos.y <= _centre.y - 6)
        return Gizmo::Axis::XY;

    // X axis
    ImVec2 xEnd(_centre.x + Gizmo::k_HandleLength, _centre.y);
    if (PointToSegmentDist(_mousePos, _centre, xEnd) < Gizmo::k_HitRadius)
        return Gizmo::Axis::X;

    // Y axis
    ImVec2 yEnd(_centre.x, _centre.y - Gizmo::k_HandleLength);
    if (PointToSegmentDist(_mousePos, _centre, yEnd) < Gizmo::k_HitRadius)
        return Gizmo::Axis::Y;

    return Gizmo::Axis::None;
}

static Gizmo::Axis HitTestRotate(ImVec2 _centre, ImVec2 _entityScale, ImVec2 _mousePos)
{
    float l_radius = std::max(Gizmo::k_RotateRadius, std::max(_entityScale.x, _entityScale.y) + 15.0f);
    float l_distance = Dist(_centre, _mousePos);
    if (std::abs(l_distance - l_radius) < Gizmo::k_HitRadius)
        return Gizmo::Axis::Rotate;

    return Gizmo::Axis::None;
}

static Gizmo::Axis HitTestScale(ImVec2 _centre, ImVec2 _entityScale, ImVec2 _mousePos)
{
    const float l_halfSize = Gizmo::k_ScaleBoxHalf + 4.0f;

    // Centre box (uniform)
    if (std::abs(_mousePos.x - _centre.x) < l_halfSize && std::abs(_mousePos.y - _centre.y) < l_halfSize)
        return Gizmo::Axis::ScaleXY;

    // X end box (at entity's right edge)
    ImVec2 xEnd(_centre.x + _entityScale.x, _centre.y);
    if (std::abs(_mousePos.x - xEnd.x) < l_halfSize && std::abs(_mousePos.y - xEnd.y) < l_halfSize)
        return Gizmo::Axis::ScaleX;

    // Y end box (at entity's top edge)
    ImVec2 yEnd(_centre.x, _centre.y - _entityScale.y);
    if (std::abs(_mousePos.x - yEnd.x) < l_halfSize && std::abs(_mousePos.y - yEnd.y) < l_halfSize)
        return Gizmo::Axis::ScaleY;

    // Axis lines
    if (PointToSegmentDist(_mousePos, _centre, xEnd) < Gizmo::k_HitRadius)
        return Gizmo::Axis::ScaleX;
    if (PointToSegmentDist(_mousePos, _centre, yEnd) < Gizmo::k_HitRadius)
        return Gizmo::Axis::ScaleY;

    return Gizmo::Axis::None;
}

Gizmo::Axis Gizmo::HitTest(GizmoMode _mode, ImVec2 _entityScreenPos,
                            ImVec2 _entityScale, ImVec2 _mouseScreen)
{
    switch (_mode)
    {
        case GizmoMode::Translate:
            return HitTestTranslate(_entityScreenPos, _entityScale, _mouseScreen);

        case GizmoMode::Rotate:
            return HitTestRotate(_entityScreenPos, _entityScale, _mouseScreen);
    
        case GizmoMode::Scale:
            return HitTestScale(_entityScreenPos, _entityScale, _mouseScreen);
    }
    return Axis::None;
}

Vector2 Gizmo::ConstrainDelta(Axis _axis, Vector2 _delta)
{
    switch (_axis)
    {
        case Axis::X:
            return Vector2(_delta.x, 0.0f);
        
        case Axis::Y:
            return Vector2(0.0f, _delta.y);
        
        case Axis::ScaleX:
            return Vector2(_delta.x, 0.0f);
        
        case Axis::ScaleY:
            return Vector2(0.0f, _delta.y);
        
        default:
            return _delta; // XY, ScaleXY, Rotate, None → unconstrained
    }
}

Vector2 Gizmo::Snap(Vector2 _pos, float _gridSize)
{
    return Vector2(
        std::round(_pos.x / _gridSize) * _gridSize,
        std::round(_pos.y / _gridSize) * _gridSize
    );
}
