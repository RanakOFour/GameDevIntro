#include "Editor/Core/Editor.h"
#include "Editor/Project/ProjectSelectionScreen.h"
#include "RanakEngine/Math/Vector2.h"
#include <cstdio>

int main()
{
    try
    {
        RE::EngineContents l_engineContents = RE::Initialise(true, Vector2(960, 540), "GameDevIntro");

        // The user can Load an existing project, create a new one, or exit the editor.
        ProjectSelectionScreen::Result l_choice = ProjectSelectionScreen::Run(l_engineContents);

        if (l_choice.action != ProjectSelectionScreen::Action::Exit)
        {
            // Transfer ownership of the Engine to the Editor, which will manage its lifetime from here on.
            Editor l_editor(l_engineContents, l_choice.project);

            Vector2 l_idealScreenSize(1920, 1080);
            Vector2 l_windowPos = l_engineContents.io->GetScreenPosition();

            printf("Window position: (%.0f, %.0f)\n", l_windowPos.x, l_windowPos.y);

            // Try to account for taskbar (Does NOT work on my DE, but should work on Windows)
            l_idealScreenSize.y = l_idealScreenSize.y - l_windowPos.y;

            printf("Ideal screen size: (%.0f, %.0f)\n", l_idealScreenSize.x, l_idealScreenSize.y);
            
            l_engineContents.io->SetScreenSize(l_idealScreenSize);

            if (l_choice.action == ProjectSelectionScreen::Action::StartTutorial)
            {
                TutorialRegistry& l_registry = l_editor.GetTutorialRegistry();
                l_editor.GetTutorialPanel().LoadTutorial("GettingStarted", l_registry.GetTutorialSource("GettingStarted"));
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