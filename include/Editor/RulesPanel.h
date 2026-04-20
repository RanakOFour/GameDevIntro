#ifndef RULES_PANEL_H
#define RULES_PANEL_H

#include "RanakEngine/RanakEngine.h"
#include "Editor/Panel.h"

#include "imgui/imgui.h"

#include <vector>
#include <string>
#include <memory>

class Editor;

/**
 * @class RulesPanel
 * @brief Editor panel for managing active Rules in the current scene.
 *
 * Lists all rules that are loaded and running in the scene.  Rules can be
 * created by name, loaded from .lua files on disk, toggled on/off, or removed.
 * A DrawAsChild() variant renders the panel inside the TextEditTab column layout.
 */
class RulesPanel : public Panel
{
    private:
    std::vector<std::string> m_activeRules; ///< Names of rules currently running in the scene.
    std::vector<std::string> m_loadedRules; ///< Names of rules available but not yet active.

    std::string m_newRuleName; ///< Name typed in the create-rule dialog.

    int m_selectedRuleIndex = -1; ///< Index of the currently selected rule in the active list.
    std::shared_ptr<RE::Asset::LuaFile> m_selectedRuleFile; ///< Source .lua file of the selected rule.

    bool m_showCreateDialog; ///< Whether the "Create New Rule" modal is open.
    bool m_showLoadDialog;   ///< Whether the file-open dialog is in progress.
    bool m_showBuiltins;     ///< Whether built-in rules are visible in the list.
    bool m_needsRefresh = true; ///< Whether the rule list needs re-querying.

    /** @brief Re-queries the scene for the current active and loaded rule lists. */
    void RefreshRuleList();
    /** @brief Renders the modal dialog for creating a new rule from a name. */
    void DrawCreateRuleDialog();

    void Draw() override;

    public:
    /** @brief Renders the IGFD file-chooser dialog. Must be called outside any ImGui Begin/End scope. */
    void DrawLoadRuleDialog();
    /**
     * @brief Constructs the panel.
     * @param _editor Reference to the owning Editor.
     */
    RulesPanel(Editor& _editor);
    ~RulesPanel();

    /**
     * @brief Selects the rule at the given list index.
     * @param _idx Index into the active-rules list.
     */
    void SelectRule(int _idx);
    /**
     * @brief Returns the source LuaFile of the currently selected rule.
     */
    std::weak_ptr<RE::Asset::LuaFile> GetSelectedFile();
    /**
     * @brief Returns the name of the rule at the given index.
     * @param _idx Index into the active-rules list.
     */
    std::string GetRuleAt(int _idx);

    /**
     * @brief Creates and registers a new blank Rule with the given name.
     * @param _name Name for the new rule.
     */
    void CreateNewRule(const std::string _name);
    /**
     * @brief Loads a Rule from a .lua file and adds it to the scene.
     * @param _path Filesystem path to the .lua rule file.
     */
    void LoadRuleFromFile(const std::string _path);
    /**
     * @brief Removes the named Rule from the scene.
     * @param _ruleName Name of the rule to remove.
     */
    void RemoveRule(const std::string _ruleName);

    /** @brief Returns true while either the create or load dialog is open. */
    bool IsDialogOpen();

    /** @brief Marks the rule list for re-querying next frame. */
    void MarkDirty() { m_needsRefresh = true; }
};

#endif
