#ifndef PROJECT_SELECTION_SCREEN_H
#define PROJECT_SELECTION_SCREEN_H

#include "Editor/Project.h"
#include "RanakEngine/RanakEngine.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_opengl3.h"

#include "imgui/misc/cpp/imgui_stdlib.h"

#define USE_STD_FILESYSTEM 1
#include "imguiFileDialog/ImGuiFileDialog.h"

#include <string>

/**
 * @class ProjectSelectionScreen
 * @brief Project-selection UI shown before the editor opens.
 *
 * Pages:
 *   - Main        : "Start New Project" / "Load Project" / "Exit"
 *   - NewProject  : location Browse button, project name input, Tutorial/Sandbox radio
 *   - LoadProject : directory Browse button, Load
 */
class ProjectSelectionScreen
{
    // Public srructs and enums, methods below
    public:
    enum class Action
    {
        StartTutorial,
        StartSandbox,
        Exit
    };

    struct Result
    {
        Action  action;
        Project project;
    };

    private:
    enum class Page { Main, NewProject, LoadProject };

    struct State
    {
        Page page            = Page::Main;
        
        std::string location = "";   ///< Parent directory chosen by Browse
        std::string name     = "";   ///< Project name (NewProject only)
        std::string loadDir  = "";   ///< Full project directory (LoadProject)
        std::string errorMsg;

        bool tutorial        = true;
        bool decided         = false;
        
        Result result        { Action::Exit, Project{} };
    };

    static void DrawMain       (State& _state, ImVec2 _displaySize);
    static void DrawNewProject (State& _state, ImVec2 _displaySize);
    static void DrawLoadProject(State& _state, ImVec2 _displaySize);
    
    static void DrawDirDialogs (State& _state);

    static void DrawCentredTitle (const std::string& _text, ImVec2 _displaySize);
    static bool DrawCentredButton(const std::string& _label, ImVec2 _btnSize);

    public:

    static Result Run(RE::EngineContents& _engineContents);
};

#endif
