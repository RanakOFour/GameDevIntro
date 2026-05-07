#include "Editor/Support/AutoCompleteTree.h"
#include "Editor/Tabs/TextEditTab.h"

// Check if a string starts with another string (case insensitive)
bool StartsWith(const std::string& str, const std::string& prefix)
{
    if (prefix.length() > str.length()) return false;
    return std::equal(prefix.begin(), prefix.end(), str.begin(), 
        [](char c1, char c2)
        {
        return std::tolower(c1) == std::tolower(c2);
        });
}

// Check if a string contains another string (case insensitive)
bool Contains(const std::string& str, const std::string& substring)
{
    if (substring.length() > str.length()) return false;
    std::string lowerStr = str;
    std::string lowerSubstring = substring;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    std::transform(lowerSubstring.begin(), lowerSubstring.end(), lowerSubstring.begin(), ::tolower);
    return lowerStr.find(lowerSubstring) != std::string::npos;
}

void AutoCompleteTree::SetTextEdit(std::weak_ptr<TextEditTab> _editTab)
{
    m_editTab = _editTab;
}

void AutoCompleteTree::textCallback()
{
    m_editTab.lock()->m_fileToEdit->FlagReloaded();
}

void AutoCompleteTree::transactionCallback(std::vector<TextEditor::Change>& _changes)
{
    std::string l_isInsert = _changes.back().insert ? "Insert" : "Remove";

    for(auto& l_lastChange : _changes)
    {
        if(l_lastChange.insert)
        {
            if(l_lastChange.text.size() > 1)
            {
                m_lastCompleteWord = "";
            }
            else
            {
                switch(l_lastChange.text.back())
                {
                    // Reset suggestion on tab, newline, space
                    case '\t':
                    case '\n':
                    case ' ':
                    m_lastCompleteWord = "";
                    break;

                    default:
                    m_lastCompleteWord += l_lastChange.text;
                }
            }
        }
        else
        {
            if(m_lastCompleteWord.size() >= l_lastChange.text.size())
            {
                m_lastCompleteWord = m_lastCompleteWord.substr(0, m_lastCompleteWord.size() - l_lastChange.text.size());
            }
            else
            {
                m_lastCompleteWord = "";
            }
            
        }


        //RE::Log::Message("Change callback text: " + l_isInsert + " " + l_lastChange.text + "\n     Current Word: " + m_lastCompleteWord);
    }

};

void AutoCompleteTree::autocompleteCallback(TextEditor::AutoCompleteState& _state)
{
    //RE::Log::Message("Current search term: " + _state.searchTerm + " | Last word: " + m_lastCompleteWord);

    _state.suggestions.clear();

    // m_lastCompleteWord is the source of truth: it accumulates the full token including dots
    // (e.g. "Vector2.ne") because transactionCallback only resets it on whitespace/tab/newline.
    // _state.searchTerm resets whenever '.' is typed, so it is only used as a fallback for
    // manual triggers (Ctrl+Space) where m_lastCompleteWord may still be empty.
    const std::string& context = m_lastCompleteWord.empty() ? _state.searchTerm : m_lastCompleteWord;

    size_t lastDot = context.find_last_of('.');

    if (lastDot == std::string::npos)
    {
        // No dot — complete against top-level names.
        for (const auto& name : m_completionTree[""])
        {
            if (context.empty() || StartsWith(name, context))
            {
                _state.suggestions.push_back(name);
            }
        }
    }
    else
    {
        // Dot present — split into namespace and the fragment typed after the last dot.
        std::string namespaceName = context.substr(0, lastDot);
        std::string fragment      = context.substr(lastDot + 1);

        auto it = m_completionTree.find(namespaceName);
        if (it != m_completionTree.end())
        {
            for (const auto& member : it->second)
            {
                if (fragment.empty() || StartsWith(member, fragment))
                {
                    _state.suggestions.push_back(member);
                }
            }
        }
    }

    std::sort(_state.suggestions.begin(), _state.suggestions.end());
}