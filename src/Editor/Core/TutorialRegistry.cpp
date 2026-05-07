#include "Editor/Core/TutorialRegistry.h"
#include "Editor/Panels/UI/TutorialPanel.h"

TutorialRegistry::TutorialRegistry()
{
}

void TutorialRegistry::RegisterTutorial(const std::string& _name, const std::string& _source)
{
    m_entries.push_back({ _name, _source });
}

void TutorialRegistry::LoadTutorial(const std::string& _name, TutorialPanel& _panel)
{
    for (const auto& entry : m_entries)
    {
        if (entry.name == _name)
        {
            _panel.LoadTutorial(entry.name, entry.source);
            return;
        }
    }
}

std::string TutorialRegistry::GetTutorialSource(const std::string& _name) const
{
    for (const auto& entry : m_entries)
    {
        if (entry.name == _name)
        {
            return entry.source;
        }
    }
    return "";
}

std::vector<std::string> TutorialRegistry::GetTutorialNames() const
{
    std::vector<std::string> names;
    for (const auto& entry : m_entries)
    {
        names.push_back(entry.name);
    }
    return names;
}