#include "Editor/SceneEditTab.h"

SceneEditTab::SceneEditTab(std::weak_ptr<Editor> _editor)
	: m_editor(_editor)
	, m_entityPanel(m_editor)
	, m_propertiesPanel(m_editor)
	, m_categoryPanel(m_editor)
	, m_rulesPanel(m_editor)
	, m_selectedEntityId(-1)
	, m_isEditorRunning(false)
	, m_isGameRunning(false)
{
	auto l_editor = _editor.lock();

	m_scene = l_editor->GetEngineContents().core->GetScene().lock();
	m_camera = l_editor->GetEngineContents().core->GetCamera().lock();
	m_camera = l_editor->GetEngineContents().io->GetWindow().lock();
}