#ifndef THEMESETTINGSPANEL_H
#define THEMESETTINGSPANEL_H

#include "../../UI/Panel.h"

class ThemeManager;

/**
 * @class ThemeSettingsPanel
 * @brief Editor panel for selecting, editing, and managing colour theme presets.
 *
 * Accessible from the Settings menu in the top bar.  Displays a list of presets,
 * a colour editor for the selected preset, and controls to create / delete / rename
 * presets.  Changes are applied live and persisted via ThemeManager.
 */
class ThemeSettingsPanel : public Panel
{
public:
    /**
     * @param _editor Reference to the owning Editor.
     * @param _themeManager Reference to the shared ThemeManager.
     */
    ThemeSettingsPanel(Editor& _editor, ThemeManager& _themeManager);

protected:
    void Draw() override;

private:
    ThemeManager& m_themeManager;
    char m_newPresetName[128] = {};
};

#endif
