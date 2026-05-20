#ifndef TUTORIALPANEL_H
#define TUTORIALPANEL_H

#include "../../UI/Panel.h"
#include "RanakEngine/RanakEngine.h"
#include "imgui/imgui_internal.h"

#include <vector>
#include <string>
#include <map>

namespace RanakEngine::Asset { class Texture; }

/**
 * @struct ChecklistItem
 * @brief A single task within a TutorialStep checklist.
 */
struct ChecklistItem
{
    std::string label; ///< Human-readable task description shown in the panel.
    std::string event; ///< Event name that fulfills this item when fired via NotifyEvent().
};

/**
 * @struct TutorialStep
 * @brief Data for a single step in a tutorial sequence.
 *
 * All fields are optional (empty string = not used for that step).
 */
struct TutorialStep
{
    std::string title;        ///< Short heading shown in yellow at the top of the panel.
    std::string body;         ///< Scrollable description text (may contain newlines).
    std::string imagePath;    ///< Asset path to an optional image shown above the body.
    std::string highlightKey; ///< Key into TutorialPanel::m_regions (or ImGui window title) to highlight.
    std::string event;        ///< Advance trigger: "next" (default), "click_region", "wait_state", or "checklist".
    std::string forceState;   ///< Editor state to force immediately when this step is entered (optional).
    std::string waitState;    ///< State condition checked each frame when event="wait_state".
    std::vector<ChecklistItem> checklist; ///< Optional ordered tasks; step auto-advances when all are fulfilled.
};

/**
 * @class TutorialPanel
 * @brief In-editor guided tutorial overlay with step-by-step progression.
 *
 * Loaded from a Lua table (see resources/Tutorials/) via LoadTutorial().  Each step
 * can display a title, body text and an optional image.  Steps with event="next"
 * dim the editor and require the user to click "Next".  Steps with event="click_region"
 * leave the editor interactive and auto-advance when the user clicks inside the
 * highlighted ImGui window or widget region.
 *
 * Highlights are resolved by first checking the m_regions map (fine-grained
 * per-widget rects registered with RegisterRegion()), then falling back to
 * ImGui::FindWindowByName() for whole-window highlights.
 *
 * Supported event types:
 *   "next"         — Next/Prev buttons and progress bar (default).
 *   "click_region" — Auto-advances when the user left-clicks inside the region
 *                    identified by @c highlight (manual region or ImGui window).
 *   "click_panel"  — Auto-advances when the user left-clicks anywhere inside
 *                    the ImGui panel/window whose title matches @c highlight.
 *                    The panel is auto-shown by SceneEditTab before drawing.
 *   "wait_state"   — Auto-advances when the named StateRegistry condition fires.
 */
class TutorialPanel : public Panel
{
    
    private:
    std::string m_tutorialTitle; ///< Main title of the loaded tutorial.
    std::vector<TutorialStep> m_steps; ///< Chronological list of tutorial steps.
    int m_currentStep = 0; ///< Index of the currently displayed step.
    int m_lastAppliedForceStep = -1; ///< Tracks which step's force state was last applied.
    bool m_waitSatisfiedAtEntry = false; ///< True when the wait_state condition was already met when the step was entered (e.g. satisfied by force_state). Auto-advance is suppressed until the condition first drops false.
    bool m_waitStateSatisfied = false; ///< True once a wait_state step's condition has been genuinely satisfied; enables the manual Next button instead of auto-advancing.

    std::vector<bool> m_checklistDone; ///< Per-item fulfillment state for the current step's checklist.

    /// Loaded textures for the tutorial steps.
    std::map<std::string, std::shared_ptr<RE::Asset::Texture>> m_imageCache;

    /// Named screen regions registered by other panels each frame.
    std::map<std::string, ImRect> m_regions;

    /** @brief Draws the pulsing highlight rect over the current step's target region. */
    void DrawHighlightOverlay();

    /** @brief Renders the tutorial window, dim overlay (if applicable), and highlight. */
    void Draw() override;

    /**
     * @brief Applies a force-state directive, immediately mutating editor state.
     *
     * Supported values: \"scene_tab\", \"text_tab\", \"game_run\", \"game_stop\",
     * \"panel:<PanelTitle>\" (e.g. \"panel:Categories\").
     *
     * @param _state Force-state string from TutorialStep::forceState.
     */
    void ApplyForceState(const std::string& _state);

    /**
     * @brief Returns true when the given state condition is currently satisfied.
     *
     * Supported values: \"scene_tab\", \"text_tab\", \"game_running\", \"game_stopped\",
     * \"entity_selected\", \"panel_open:<PanelTitle>\" (e.g. \"panel_open:Categories\").
     *
     * @param _state Wait-state condition string from TutorialStep::waitState.
     */
    bool IsStateAchieved(const std::string& _state) const;

    public:
    /**
     * @brief Panel constructor.
     * @param _editor Reference to the owning Editor.
     */
    TutorialPanel(Editor& _editor);
    ~TutorialPanel() = default;

    /**
     * @brief Loads and starts a tutorial from the registry
     *
     * @param _steps Lua table containing the tutorial steps.
     */
    void LoadTutorial(const std::string _title, const std::string& _steps);

    /**
     * @brief Registers a widget rect for highlighting by key.
     *
     * Call this after ImGui::GetItemRectMin/Max() for the widget you want
     * highlighted.  The registration is frame-scoped; ClearRegions() is
     * called at the start of each frame.
     *
     * @param _key  Arbitrary string key matching TutorialStep::highlightKey.
     * @param _rect Screen-space bounding box of the widget.
     */
    void RegisterRegion(const std::string& _key, ImRect _rect);

    /**
     * @brief Clears all per-widget regions registered in the previous frame.
     *
     * Must be called once per frame before any panel's Draw(), so stale
     * regions from the previous frame cannot match a moved or closed panel.
     */
    void ClearRegions();

    /** @brief Returns true while a tutorial is loaded and the panel is visible. */
    bool IsActive() const { return m_showPanel && !m_steps.empty(); }

    /**
     * @brief Returns true when the current step advances by clicking a region.
     *
     * When true, the editor remains fully interactive and is not dimmed;
     * clicking inside the highlighted area triggers progression.
     */
    bool IsStepInteractive() const;

    /**
     * @brief Returns the highlight key of the current step, or an empty string.
     *
     * Used by SceneEditTab::DrawEditorUI() to force the target panel open
     * before Draw() is called so FindWindowByName() can locate it.
     */
    std::string GetCurrentHighlightKey() const;

    /**
     * @brief Notifies the panel that an editor action event occurred.
     *
     * If the active step has a checklist, marks the first unfulfilled item
     * whose event matches @p _event.  Has no effect when no tutorial is active.
     *
     * @param _event  Event name (e.g. "entity_created", "category_added", "rule_added").
     */
    void NotifyEvent(const std::string& _event);
};

#endif
