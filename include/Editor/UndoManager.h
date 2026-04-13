#ifndef UNDO_MANAGER_H
#define UNDO_MANAGER_H

#include <string>
#include <memory>
#include <stack>
#include <functional>

/**
 * @class ICommand
 * @brief Abstract base for undoable editor commands.
 *
 * Each concrete command captures enough state to execute and reverse
 * a single editor action.
 */
class ICommand
{
public:
    virtual ~ICommand() = default;

    /** @brief Applies (or re-applies) the command. */
    virtual void Execute() = 0;
    /** @brief Reverses the command, restoring the prior state. */
    virtual void Undo() = 0;
    /** @brief Short human-readable description (shown in Edit menu). */
    virtual std::string GetDescription() const = 0;
};

/**
 * @class LambdaCommand
 * @brief Generic undoable command built from two lambdas.
 *
 * Useful for one-off commands that don't warrant a dedicated subclass.
 */
class LambdaCommand : public ICommand
{
    private:
    std::string m_description;
    std::function<void()> m_execute;
    std::function<void()> m_undo;
    
    public:
    /**
     * @param _desc     Human-readable label.
     * @param _execute  Callable invoked on Execute / Redo.
     * @param _undo     Callable invoked on Undo.
     */
    LambdaCommand(std::string _desc,
                  std::function<void()> _execute,
                  std::function<void()> _undo)
        : m_description(std::move(_desc))
        , m_execute(std::move(_execute))
        , m_undo(std::move(_undo))
    {}

    void Execute() override { m_execute(); }
    void Undo()    override { m_undo(); }
    std::string GetDescription() const override { return m_description; }
};

/**
 * @class UndoManager
 * @brief Maintains undo and redo stacks of ICommand objects.
 *
 * Call ExecuteCommand() to execute a new command and push it onto the undo
 * stack. Any pending redo history is cleared when a new command is executed.
 */
class UndoManager
{
    private:
    std::stack<std::unique_ptr<ICommand>> m_undoStack;
    std::stack<std::unique_ptr<ICommand>> m_redoStack;
    
    public:
    /**
     * @brief Executes a command and pushes it onto the undo stack.
     * @param _command The command to execute (ownership transferred).
     */
    void ExecuteCommand(std::unique_ptr<ICommand> _command);

    /**
     * @brief Pushes an already-executed command onto the undo stack without calling Execute().
     * @param _command The command (ownership transferred).
     */
    void PushCommand(std::unique_ptr<ICommand> _command);

    /** @brief Undoes the most recent command. No-op if the undo stack is empty. */
    void Undo();
    /** @brief Re-does the most recently undone command. No-op if the redo stack is empty. */
    void Redo();

    /** @brief Returns true when the undo stack is non-empty. */
    bool CanUndo() const { return !m_undoStack.empty(); }
    /** @brief Returns true when the redo stack is non-empty. */
    bool CanRedo() const { return !m_redoStack.empty(); }

    /** @brief Description of the command that would be undone next, or empty. */
    std::string GetUndoDescription() const;
    /** @brief Description of the command that would be redone next, or empty. */
    std::string GetRedoDescription() const;

    /** @brief Clears both stacks. */
    void Clear();
};

#endif
