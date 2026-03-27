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

struct TutorialStep
{
    std::string title;
    std::string body;
    std::string imagePath;    // empty = no image this step
    std::string highlightKey; // empty = no highlight this step
    std::string event;        // "next" (default) or "click_region"
};

class TutorialPanel : public Panel
{
private:
    std::string m_tutorialTitle;
    std::vector<TutorialStep> m_steps;
    int m_currentStep = 0;

    // Keeps loaded textures alive for the duration of the tutorial
    std::map<std::string, std::shared_ptr<RE::Asset::Texture>> m_imageCache;

    // Named screen regions registered by other panels each frame
    std::map<std::string, ImRect> m_regions;

    void DrawHighlightOverlay();

public:
    TutorialPanel() {};
    TutorialPanel(std::weak_ptr<Editor> _editor);

    // Load a tutorial from a Lua file path
    void LoadTutorial(const std::string& _path);

    // Called by other panels to register a named bounding box for highlighting.
    // Use ImGui::GetItemRectMin() / GetItemRectMax() after the widget you want highlighted.
    void RegisterRegion(const std::string& _key, ImRect _rect);

    // Must be called once per frame before any panel Draws, to clear last frame's registrations
    void ClearRegions();

    // True while a tutorial is loaded and visible.
    bool IsActive() const { return m_showPanel && !m_steps.empty(); }

    // True when the current step requires the user to click an editor region (not the Next button).
    bool IsStepInteractive() const;

    // Returns the highlight key for the current step, or an empty string.
    const std::string& GetCurrentHighlightKey() const;

    void Draw() override;
};

#endif
