#include "Editor/Editor.h"
#include "Editor/ProjectSelectionScreen.h"

int main()
{
    try
    {
        // Initialise just enough of the engine to show the project selection
        // screen (window + GL context).  Editor::Create() reuses this handle.
        RE::EngineContents l_engineContents = RE::Initialise(true, Vector2(1920 / 4, 1080 / 4));

        ProjectSelectionScreen::Result l_choice =
            ProjectSelectionScreen::Run(l_engineContents);

        if (l_choice.action != ProjectSelectionScreen::Action::Exit)
        {
            Editor l_editor = Editor::Create(l_engineContents, l_choice.project);

            l_engineContents.io->SetScreenSize(Vector2(1920, 1080));

            if (l_choice.action == ProjectSelectionScreen::Action::StartTutorial)
                l_editor.GetTutorialPanel().LoadTutorial("./resources/Tutorials/GettingStarted.lua");

            l_editor.Run();
        }

        RE::Shutdown(l_engineContents);
    }
    catch (std::exception& e)
    {
        printf("Editor failed to run: %s", e.what());
        return 1;
    }

    return 0;
}
