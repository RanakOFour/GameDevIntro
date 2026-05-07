#ifndef TUTORIALREGISTRY_H
#define TUTORIALREGISTRY_H

#include <string>
#include <vector>

class TutorialPanel;
class TutorialRegistry
{
    friend class BuiltinTutorials;
    private:
    struct Entry
    {
        const std::string name;
        const std::string source; ///< Lua source for the tutorial scene file.
    };

    std::vector<Entry> m_entries = {};

    public:
    TutorialRegistry();
    
    void RegisterTutorial(const std::string& _name, const std::string& _source);
    void LoadTutorial(const std::string& _name, TutorialPanel& _panel);

    std::string GetTutorialSource(const std::string& _name) const;

    std::vector<std::string> GetTutorialNames() const;
};

#endif