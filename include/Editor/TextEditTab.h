#ifndef TEXTEDITTAB_H
#define TEXTEDITTAB_H

#include "RanakEngine/RanakEngine.h"

#include "Editor/Editor.h"
#include "Editor/RulesPanel.h"
#include "Editor/CategoryPanel.h"

#include "imgui/imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

#include "imguiTextEdit/TextEditor.h"

#include <memory>


class TextEditTab
{
    private:
    std::weak_ptr<Editor> m_editor;
    std::shared_ptr<RE::Asset::LuaFile> m_fileToEdit;

    TextEditor m_textEditor;
    CategoryPanel m_categoryPanel;
    RulesPanel m_rulesPanel;

    ImVec2 m_size;

    public:
    TextEditTab(std::weak_ptr<Editor> _editor);
    ~TextEditTab();

    void Draw();

    void SetFile(std::weak_ptr<RE::Asset::LuaFile> _file);
    void SaveCurrentFile();

};

#endif