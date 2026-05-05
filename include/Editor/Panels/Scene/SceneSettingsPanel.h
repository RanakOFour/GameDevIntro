#ifndef SCENESETTINGSPANEL_H
#define SCENESETTINGSPANEL_H

#include "../../UI/Panel.h"
#include "../../Scene/SceneSettings.h"

#include <memory>

class Editor;

/**
 * @class SceneSettingsPanel
 * @brief Dockable panel for editing per-scene simulation and rendering settings.
 *
 * Exposes gravity (X/Y) and background clear colour.  Changes take effect
 * immediately: clear colour is read each frame by the renderer; gravity is
 * pushed to Physics::Manager whenever a drag widget changes.
 *
 * The panel holds a raw pointer to the SceneSettings owned by SceneEditTab.
 * This is safe because SceneEditTab owns both the settings and the panel,
 * so the settings always outlive the panel.
 */
class SceneSettingsPanel : public Panel
{
private:
    SceneSettings* m_settings = nullptr; ///< Live settings owned by SceneEditTab.

    /** @brief Renders gravity and clear colour controls. */
    void Draw() override;

public:
    /**
     * @brief Constructs the panel.
     * @param _editor   Reference to the owning Editor (used to push gravity changes).
     * @param _settings Pointer to the SceneSettings owned by SceneEditTab.
     */
    SceneSettingsPanel(Editor& _editor, SceneSettings* _settings);
    ~SceneSettingsPanel() override = default;
};

#endif
