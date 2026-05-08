#include "Editor/Core/Editor.h"
#include "Editor/Project/ProjectSelectionScreen.h"
#include "RanakEngine/Math/Vector2.h"
#include <cstdio>

int main()
{
    try
    {
        RE::EngineContents l_engineContents = RE::Initialise(true, Vector2(960, 540), "GameDevIntro");

        // The user can load an existing project, create a new one, or exit the editor.
        ProjectSelectionScreen::Result l_choice = ProjectSelectionScreen::Run(l_engineContents);

        if (l_choice.action != ProjectSelectionScreen::Action::Exit)
        {
            Vector2 l_idealScreenSize{1920, 1080};

            #if _WIN32
                // Account for title bar height
                l_idealScreenSize.y -= 32;
            #else
            #endif

            l_engineContents.io->MaximiseCurrentWindow();

            // Transfer ownership of the Engine to the Editor, which will manage its lifetime from here on.
            Editor l_editor(l_engineContents, l_choice.project);

            if (l_choice.action == ProjectSelectionScreen::Action::StartTutorial)
            {
                l_editor.LoadTutorial("Getting Started");
            }

            l_editor.Run();
        }

        // Engine shutdown is handled by the Editor, as it owns the EngineContents
    }
    catch (std::exception& e)
    {
        printf("Editor failed to run: %s", e.what());
        return 1;
    }

    return 0;
}