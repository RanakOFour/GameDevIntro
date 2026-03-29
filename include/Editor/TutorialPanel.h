#ifndef TUTORIALPANEL_H
#define TUTORIALPANEL_H

#include "Editor/Panel.h"
#include "RanakEngine/RanakEngine.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <vector>
#include <string>
#include <map>
#include <memory>

namespace RanakEngine::Asset { class Texture; }

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
    std::string event;        ///< Advance trigger: \"next\" (default) or \"click_region\".
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
 */
class TutorialPanel : public Panel
{
private:
    std::string m_tutorialTitle; ///< Main title of the loaded tutorial.
    std::vector<TutorialStep> m_steps; ///< Chronological list of tutorial steps.
    int m_currentStep = 0; ///< Index of the currently displayed step.

    /// Loaded textures for the tutorial steps.
    std::map<std::string, std::shared_ptr<RE::Asset::Texture>> m_imageCache;

    /// Named screen regions registered by other panels each frame.
    std::map<std::string, ImRect> m_regions;

    /** @brief Draws the pulsing highlight rect over the current step's target region. */
    void DrawHighlightOverlay();

public:
    TutorialPanel() {};
    /**
     * @brief Panel constructor.
     * @param _editor Weak pointer to the owning Editor.
     */
    TutorialPanel(std::weak_ptr<Editor> _editor);

    /**
     * @brief Loads and starts a tutorial from a Lua data file.
     *
     * The file must return a table with an optional "title" string and a
     * "steps" array of step tables.  All step images are loaded into
     * m_imageCache.
     *
     * @param _path Asset path to the .lua tutorial file.
     */
    void LoadTutorial(const std::string& _path);

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

    /** @brief Renders the tutorial window, dim overlay (if applicable), and highlight. */
    void Draw() override;
};

#endif
