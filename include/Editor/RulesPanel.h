#ifndef RULES_PANEL_H
#define RULES_PANEL_H

#include "RanakEngine/RanakEngine.h"
#include "Editor/Panel.h"
#include <vector>
#include <string>
#include <memory>

class Editor;

class RulesPanel : public Panel
{
    private:

    std::vector<std::string> m_activeRules;
    std::vector<std::string> m_availableRuleFiles;
    
    std::string m_newRuleName;
    std::string m_selectedRuleFilter;
    
    bool m_showCreateDialog;
    bool m_showLoadDialog;

    void RefreshRuleList();
    void DrawCreateRuleDialog();
    void DrawLoadRuleDialog();

    public:
    RulesPanel() {};
    RulesPanel(std::weak_ptr<Editor> _editor);
    ~RulesPanel();

    void Draw();
    void CreateNewRule(const std::string& _name);
    void LoadRuleFromFile(const std::string& _path);
    void RemoveRule(const std::string& _ruleName);

    bool IsDialogOpen();
};

#endif
