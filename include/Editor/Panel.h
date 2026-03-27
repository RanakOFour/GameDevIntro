#ifndef PANEL_H
#define PANEL_H

#include <memory>
#include <string>

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
    std::string m_filterString;     ///< Current text filter used by list-based panels.

    public:
    Panel() {};
    /**
     * @brief Constructs the panel with an editor reference.
     * @param _editor Weak pointer to the owning Editor.
     */
    Panel(std::weak_ptr<Editor> _editor);
    virtual ~Panel() = default;

    /** @brief Renders the panel's ImGui content for this frame. */
    virtual void Draw() = 0;

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
