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

/**
 * @class AutoCompleteTree
 * @brief Provides Lua-aware autocomplete suggestions for the code editor.
 *
 * On construction via InitTree() it introspects the live sol2 Lua state and
 * builds a two-level completion map:  top-level global identifiers map to
 * the list of their table members.  Three callbacks are wired into the
 * imguiTextEdit TextEditor delegate system:
 *
 *   - textCallback()         - called on every keystroke.
 *   - transactionCallback()  - called with a batch of change records.
 *   - autocompleteCallback() - called when the user triggers autocomplete
 *                              (Ctrl+Space) and fills the suggestion list.
 *
 * The autocomplete logic uses m_lastCompleteWord (the last fully typed
 * identifier before the cursor) as context, splitting on '.' to determine
 * a namespace prefix.
 */
class AutoCompleteTree
{
    private:
    std::map<std::string, std::vector<std::string>> m_completionTree; ///< Two-level completion map: namespace -> member list.
    std::string m_lastCompleteWord; ///< Last complete identifier before the cursor (updated by textCallback).
    std::weak_ptr<TextEditTab> m_editTab; ///< Back-reference to the owning TextEditTab.

    public:
    AutoCompleteTree() : m_completionTree(), m_lastCompleteWord("") {};
    ~AutoCompleteTree() {};

    /**
     * @brief Builds an AutoCompleteTree by interrogating all globals in the Lua state.
     *
     * Iterates the global table; for each non-sol global that is itself a table,
     * it records the table's member names as completions under the global's name.
     * Globals that start with "sol" are skipped.
     *
     * @param _context Weak pointer to the engine's live LuaContext.
     * @return A fully populated AutoCompleteTree ready for use.
     */
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

    /**
     * @brief Binds this tree to a TextEditTab so callbacks can access the editor state.
     * @param _editTab Weak pointer to the owning TextEditTab.
     */
    void SetTextEdit(std::weak_ptr<TextEditTab> _editTab);

    /** @brief Keystroke callback - updates m_lastCompleteWord each time text changes. */
    void textCallback();

    /**
     * @brief Batch-change callback called after a group of edits is committed.
     * @param _changes List of TextEditor change records describing what was altered.
     */
    void transactionCallback(std::vector<TextEditor::Change>& _changes);

    /**
     * @brief Autocomplete callback that populates the suggestion list.
     *
     * Uses m_lastCompleteWord as context.  If the word contains '.' it looks
     * up completions for the namespace prefix; otherwise it suggests top-level
     * globals.  Results are written into _state.
     *
     * @param _state ImGuiTextEdit state object; completion candidates are appended here.
     */
    void autocompleteCallback(TextEditor::AutoCompleteState&);
};

#endif