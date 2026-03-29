#ifndef CAMERAOPTIONPANEL_H
#define CAMERAOPTIONPANEL_H

#include "Editor/Panel.h"

#include "RanakEngine/RanakEngine.h"

#include <memory>

class Editor;

/**
 * @class CameraPanel
 * @brief Editor panel for adjusting the scene camera at runtime.
 *
 * Exposes camera width (orthographic zoom), position, and projection type
 * via sliders and inputs so the user can tweak the viewport interactively.
 */
class CameraPanel : public Panel
{
    private:
    std::weak_ptr<RE::Core::Camera> m_camera; ///< The camera being configured.

    /** @brief Renders the "Camera Options" window. */
    void Draw() override;

    public:
    CameraPanel() {};
    /**
     * @brief Constructs the panel and caches the camera reference.
     * @param _editor Weak pointer to the owning Editor.
     */
    CameraPanel(std::weak_ptr<Editor> _editor);
    ~CameraPanel();
};

#endif