#include "Editor/Core/UndoManager.h"

void UndoManager::ExecuteCommand(std::unique_ptr<ICommand> _command)
{
    _command->Execute();
    m_undoStack.push(std::move(_command));

    // New action invalidates redo history.
    while (!m_redoStack.empty())
        m_redoStack.pop();
}

void UndoManager::PushCommand(std::unique_ptr<ICommand> _command)
{
    m_undoStack.push(std::move(_command));

    while (!m_redoStack.empty())
        m_redoStack.pop();
}

void UndoManager::Undo()
{
    if (m_undoStack.empty())
        return;

    auto l_cmd = std::move(m_undoStack.top());
    m_undoStack.pop();

    l_cmd->Undo();
    m_redoStack.push(std::move(l_cmd));
}

void UndoManager::Redo()
{
    if (m_redoStack.empty())
        return;

    auto l_cmd = std::move(m_redoStack.top());
    m_redoStack.pop();

    l_cmd->Execute();
    m_undoStack.push(std::move(l_cmd));
}

std::string UndoManager::GetUndoDescription() const
{
    if (m_undoStack.empty())
        return {};
    return m_undoStack.top()->GetDescription();
}

std::string UndoManager::GetRedoDescription() const
{
    if (m_redoStack.empty())
        return {};
    return m_redoStack.top()->GetDescription();
}

void UndoManager::Clear()
{
    while (!m_undoStack.empty()) m_undoStack.pop();
    while (!m_redoStack.empty()) m_redoStack.pop();
}
