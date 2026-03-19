#include "Editor/Editor.h"

int main()
{
    try
    {
        std::shared_ptr<Editor> l_editor = Editor::Create();
        l_editor->Run();
    }
    catch (std::exception e)
    {
        printf("Editor failed to run: %s", std::string(e.what()).c_str());
        return 1;
    }

    return 0;
}
