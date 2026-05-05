#ifndef TOPBAR_H
#define TOPBAR_H

#include "imgui/imgui.h"

#include <string>

class Editor;

/**
 * @class TopBar
 * @brief The main menu bar drawn at the top of the editor viewport.
 *
 * Extracted from Editor so that both SceneEditTab and TextEditTab can
 * trigger menu actions without coupling directly to each other.
 * Owned by Editor and drawn once per frame via Draw().
 */
class TopBar
{
public:
    /**
     * @brief Constructs the top bar with a back-reference to the owning Editor.
     * @param _editor Reference to the owning Editor.
     */
    TopBar(Editor& _editor);

    /** @brief Renders the main menu bar and any open file dialogs. */
    void Draw();

    /** @brief Opens the file dialog in load mode. */
    void OpenLoadDialog();
    /** @brief Opens the file dialog in save mode. */
    void OpenSaveDialog();

private:
    Editor& m_editor;

    bool m_showLoadDialog   = false;
    bool m_showSaveDialog   = false;
    bool m_showExportDialog = false;
};

#endif
