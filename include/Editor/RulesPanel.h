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

    bool m_showCreateDialog; ///< Whether the "Create New Rule" modal is open.
    bool m_showLoadDialog;   ///< Whether the file-open dialog is in progress.

    /** @brief Re-queries the scene for the current active and loaded rule lists. */
    void RefreshRuleList();
    /** @brief Renders the modal dialog for creating a new rule from a name. */
    void DrawCreateRuleDialog();
    /** @brief Renders the file-chooser dialog for loading a rule .lua file. */
    void DrawLoadRuleDialog();

    void Draw() override;

    public:
    RulesPanel() {};
    /**
     * @brief Constructs the panel.
     * @param _editor Weak pointer to the owning Editor.
     */
    RulesPanel(std::weak_ptr<Editor> _editor);
    ~RulesPanel();

    /**
     * @brief Selects the rule at the given list index.
     * @param _idx Index into the active-rules list.
     */
    void SelectRule(int _idx);
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
};

#endif
