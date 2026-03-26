#ifndef AUTOCOMPLETETREE_H
#define AUTOCOMPLETETREE_H

#include "RanakEngine/RanakEngine.h"

#include "imgui/imgui.h"
#include "imguiTextEdit/TextEditor.h"

#include "sol/sol.hpp"

#include <vector>
#include <map>
#include <functional>

class TextEditTab;
class AutoCompleteTree
{
    private:
    std::map<std::string, std::vector<std::string>> m_completionTree;
    std::string m_lastCompleteWord;
    std::weak_ptr<TextEditTab> m_editTab;

    public:
    AutoCompleteTree() : m_completionTree(), m_lastCompleteWord("") {};
    ~AutoCompleteTree() {};

    static AutoCompleteTree InitTree(std::weak_ptr<RE::Core::LuaContext> _context)
    {
        //Take global table
        sol::table l_globals = _context.lock()->GetGlobalTable();
        AutoCompleteTree l_toReturn;
        l_toReturn.m_completionTree[""] = std::vector<std::string>();

        auto l_globalPairs = l_globals.pairs();

        for(auto& l_globalPair : l_globalPairs)
        {
            std::string l_globalName = l_globalPair.first.as<std::string>();
            RE::Log::Message("Creating completion tree for global: " + l_globalName);

            if(l_globalName.substr(0, 3) == "sol")
                continue;

            l_toReturn.m_completionTree[""].push_back(l_globalName);
            l_toReturn.m_completionTree[l_globalName] = std::vector<std::string>();

            // Some global members may just be functions instead of tables to other functions
            sol::optional<sol::table> l_maybeTable = l_globalPair.second.as<sol::optional<sol::table>>();

            if(!l_maybeTable.has_value())
                continue;

            std::vector<std::string>& l_completionVector = l_toReturn.m_completionTree[l_globalName];
            sol::table l_valueTable = l_maybeTable.value();
            auto l_valuePairs = l_valueTable.pairs();
            for(auto& l_valuePair : l_valueTable)
            {
                std::string l_valueName = l_valuePair.first.as<std::string>();
                RE::Log::Message(l_globalName + ": " + l_valueName);

                if(l_valueName[0] != '_')
                {
                    l_completionVector.push_back(l_valueName);
                } 
            }

            std::sort(l_completionVector.begin(),l_completionVector.end());
        }

        return l_toReturn;
    };

    void SetTextEdit(std::weak_ptr<TextEditTab> _editTab);

    void textCallback();

    void transactionCallback(std::vector<TextEditor::Change>& _changes);

    void autocompleteCallback(TextEditor::AutoCompleteState&);
};

#endif