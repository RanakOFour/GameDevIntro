#ifndef TEXTEDITTAB_H
#define TEXTEDITTAB_H

#include "RanakEngine/RanakEngine.h"

#include "Editor/Editor.h"
#include "Editor/AutoCompleteTree.h"
#include "Editor/RulesPanel.h"
#include "Editor/CategoryPanel.h"

#include "imgui/imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

#include "imguiTextEdit/TextEditor.h"

#include <memory>

class AutoCompleteTree;

/**
 * @class TextEditTab
 * @brief The Lua source-file editing tab of the editor.
 *
 * Presents a three-column layout:
 *   - Left  : a file browser (tabbed Categories / Rules) backed by CategoryPanel and RulesPanel.
 *   - Centre: a code editor (imguiTextEdit / TextEditor) with Lua syntax highlighting
 *             and autocomplete wired through AutoCompleteTree.
 *   - Right : a live properties inspector showing the base fields of the currently
 *             loaded category file.
 *
 * Saving (Ctrl+S or the Save button) writes the file to disk and triggers
 * hot-reload of the affected Category in the running scene.
 */
class TextEditTab
{
    friend Editor;
    friend AutoCompleteTree;
    private:
    std::weak_ptr<Editor> m_editor;               ///< Back-reference to the owning Editor.
    std::shared_ptr<RE::Asset::LuaFile> m_fileToEdit; ///< The currently open .lua source file.

    AutoCompleteTree m_acTree;    ///< Autocomplete provider wired to the text editor.
    TextEditor m_textEditor;      ///< ImGuiColorTextEdit code editor widget.
    CategoryPanel m_categoryPanel; ///< Left-column category file browser.
    RulesPanel    m_rulesPanel;    ///< Left-column rule file browser.

    ImVec2 m_size; ///< Current window size, updated each frame to fill below the menu bar.

    public:
    /**
     * @brief Constructs the tab and wires the autocomplete tree callbacks.
     * @param _editor Weak pointer to the owning Editor.
     */
    TextEditTab(std::weak_ptr<Editor> _editor);
    ~TextEditTab();

    /** @brief Renders the full three-column text-editing window. */
    void Draw();

    /**
     * @brief Opens the given LuaFile in the code editor.
     * @param _file Weak pointer to the LuaFile to display.
     */
    void SetFile(std::weak_ptr<RE::Asset::LuaFile> _file);
    /**
     * @brief Writes the current editor content to disk and hot-reloads the Category.
     *
     * Caches entity field values, removes entities from the old category,
     * triggers LuaFile::Reload(), re-adds entities, and restores matching fields.
     */
    void SaveCurrentFile();
};

#endif