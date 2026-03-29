#ifndef PANEL_H
#define PANEL_H

#include <memory>
#include <string>

#include "imgui/imgui.h"

class Editor;

/**
 * @class Panel
 * @brief Abstract base for all dockable editor panel windows.
 *
 * Every concrete panel holds a weak back-reference to the Editor,
 * tracks its own visibility, and implements Draw() to emit ImGui content
 * for that frame.
 */
class Panel
{
    protected:
    std::weak_ptr<Editor> m_editor; ///< Back-reference to the owning Editor instance.
    bool m_showPanel;               ///< Whether this panel should render this frame.
    std::string m_panelTitle;           ///< Title displayed in the panel's ImGui window header.
    std::string m_filterString;     ///< Current text filter used by list-based panels.

    virtual void Draw() = 0; ///< Pure virtual method to render the panel's ImGui content.

    public:
    Panel() {};
    /**
     * @brief Constructs the panel with an editor reference.
     * @param _editor Weak pointer to the owning Editor.
     */
    Panel(std::string _name, std::weak_ptr<Editor> _editor);
    virtual ~Panel() = default;

    void DrawAsWindow(ImGuiWindowFlags _flags = 0);
    void DrawAsChild(ImGuiChildFlags _flags = 0);

    /** @brief Returns whether the panel is currently visible. */
    bool IsShown() const { return m_showPanel; }
    /**
     * @brief Shows or hides the panel.
     * @param _shown True to make visible, false to hide.
     */
    void SetShown(bool _shown) { m_showPanel = _shown; }

    /**
     * @brief Sets the text filter applied to list entries.
     * @param _filter Filter string to match against.
     */
    void SetFilterString(std::string _filter) { m_filterString = _filter; }
    /** @brief Returns the current text filter string. */
    std::string GetFilterString() const { return m_filterString; }
};

#endif
