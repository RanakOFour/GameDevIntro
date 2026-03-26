#include "Editor/AutoCompleteTree.h"
#include "Editor/TextEditTab.h"

// Check if a string starts with another string (case insensitive)
bool StartsWith(const std::string& str, const std::string& prefix)
{
    if (prefix.length() > str.length()) return false;
    return std::equal(prefix.begin(), prefix.end(), str.begin(), [](char c1, char c2) {
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

    TextEditor::Change l_lastChange = _changes.back();

    if(l_lastChange.insert)
    {
        switch(l_lastChange.text[0])
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

    RE::Log::Message("Change callback text: " + l_isInsert + " " + _changes.back().text + "\nCurrent Word: " + m_lastCompleteWord);
};

void AutoCompleteTree::autocompleteCallback(TextEditor::AutoCompleteState& _state)
{
    RE::Log::Message("Current search term: " + _state.searchTerm);
    
    // Clear previous suggestions
    _state.suggestions.clear();
    
    if(_state.searchTerm.empty())
    {
        // Show all top-level completions
        _state.suggestions = m_completionTree[""];
    }
    else
    {
        // Check if we're looking for a specific library (contains a dot)
        size_t lastDot = _state.searchTerm.find_last_of('.');
        std::string searchTerm = _state.searchTerm;
        
        // Remove trailing dots or spaces for better matching
        while (!searchTerm.empty() && (searchTerm.back() == '.' || searchTerm.back() == ' ')) {
            searchTerm.pop_back();
        }
        
        if (lastDot != std::string::npos) {
            // We're looking for a specific library function
            std::string libraryName = searchTerm.substr(0, lastDot);
            std::string functionName = searchTerm.substr(lastDot + 1);
            
            // Find the library in our completion tree
            auto libraryIt = m_completionTree.find(libraryName);
            if (libraryIt != m_completionTree.end()) {
                // Filter functions in this library by the function name
                for (const auto& func : libraryIt->second) {
                    if (StartsWith(func, functionName)) {
                        _state.suggestions.push_back(libraryName + "." + func);
                    }
                }
            }
        } else {
            // Simple search - look for functions that match the search term
            // First, check if we're completing a library name
            for (const auto& lib : m_completionTree) {
                if (lib.first.empty()) continue; // Skip root level
                
                if (StartsWith(lib.first, searchTerm)) {
                    _state.suggestions.push_back(lib.first);
                }
                
                // Also check if any functions in this library match
                for (const auto& func : lib.second) {
                    if (StartsWith(func, searchTerm)) {
                        _state.suggestions.push_back(lib.first + "." + func);
                    }
                }
            }
            
            // If we have a m_lastCompleteWord, check if it's a library name
            if (!m_lastCompleteWord.empty()) {
                // Check if m_lastCompleteWord matches any library names
                for (const auto& lib : m_completionTree) {
                    if (lib.first.empty()) continue; // Skip root level
                    
                    if (lib.first == m_lastCompleteWord) {
                        // If we're completing a library name, show its functions
                        for (const auto& func : lib.second) {
                            _state.suggestions.push_back(lib.first + "." + func);
                        }
                        break;
                    }
                }
            }
        }
    }
    
    // Sort suggestions alphabetically
    std::sort(_state.suggestions.begin(), _state.suggestions.end());
}