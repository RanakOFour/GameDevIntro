#include "Editor/Editor.h"

int main()
{
    try
    {
        Editor editor;
        editor.Run();
    }
    catch (const std::exception& e)
    {
        RE::Log::Error("Editor failed to run: " + std::string(e.what()));
        return 1;
    }

    return 0;
}
