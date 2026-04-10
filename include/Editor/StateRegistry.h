#ifndef STATEREGISTRY_H
#define STATEREGISTRY_H

#include <string>
#include <functional>
#include <unordered_map>

/**
 * @class StateRegistry
 * @brief Named condition and action registry for editor state management.
 *
 * Decouples state queriers (e.g. TutorialPanel) from state owners
 * (Editor, SceneEditTab, Panels).  Owners register predicates and
 * callbacks by name at construction time; consumers evaluate or invoke
 * them by name with no knowledge of the owning types.
 *
 * Conditions: registered with RegisterCondition(), evaluated with Evaluate().
 * Actions:    registered with RegisterAction(),    invoked   with Apply().
 *
 * Both maps use the same key namespace, so a caller can reuse the same string
 * for both "check if state is active" and "force that state on".
 *
 * Examples already registered by the editor subsystems:
 *   Conditions: "scene_tab", "text_tab", "game_running", "game_stopped",
 *               "entity_selected", "panel_open:<PanelTitle>"
 *   Actions:    "scene_tab", "text_tab", "game_run",     "game_stop",
 *               "panel:<PanelTitle>"
 */
class StateRegistry
{
public:
    /**
     * @brief Registers a named boolean condition.
     * @param _name  Lookup key (e.g. "entity_selected", "panel_open:Categories").
     * @param _pred  Predicate evaluated on demand.
     */
    void RegisterCondition(const std::string& _name, std::function<bool()> _pred);

    /**
     * @brief Registers a named action.
     * @param _name    Lookup key (e.g. "game_run", "panel:Rules").
     * @param _action  Callable invoked when the action is applied.
     */
    void RegisterAction(const std::string& _name, std::function<void()> _action);

    /**
     * @brief Evaluates a registered condition by name.
     * @return Result of the predicate, or false if @p _name is unregistered.
     */
    bool Evaluate(const std::string& _name) const;

    /**
     * @brief Invokes a registered action by name.
     * @return True if the action was found and called, false if unregistered.
     */
    bool Apply(const std::string& _name) const;

private:
    std::unordered_map<std::string, std::function<bool()>> m_conditions;
    std::unordered_map<std::string, std::function<void()>> m_actions;
};

#endif
