#ifndef PANEL_H
#define PANEL_H

#include <memory>
#include <string>

class Editor;

class Panel
{
    protected:
    std::weak_ptr<Editor> m_editor;
    bool m_showPanel;
    std::string m_filterString;

    public:
    Panel() {};
    Panel(std::weak_ptr<Editor> _editor);
    virtual ~Panel() = default;

    virtual void Draw() = 0;

    bool IsShown() const { return m_showPanel; }
    void SetShown(bool _shown) { m_showPanel = _shown; }
    
    void SetFilterString(std::string _filter) { m_filterString = _filter; }
    std::string GetFilterString() const { return m_filterString; }
};

#endif
