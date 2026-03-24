#include "Editor/Panel.h"
#include "Editor/Editor.h"

Panel::Panel(std::weak_ptr<Editor> _editor)
: m_editor(_editor)
, m_showPanel(false)
, m_filterString("")
{
}