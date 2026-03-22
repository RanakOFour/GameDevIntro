#ifndef CATEGORY_PANEL_H
#define CATEGORY_PANEL_H

#include "RanakEngine/RanakEngine.h"
#include "Editor/Panel.h"

#include "imguiTextEdit/TextEditor.h"

#include <vector>
#include <string>
#include <memory>

class Editor;

class CategoryPanel : public Panel
{
    private:
    std::shared_ptr<TextEditor> m_textEditor;
    std::vector<std::string> m_availableCategories;
    int m_selectedCategory;
    std::shared_ptr<RE::Asset::LuaFile> m_selectedCategoryOrigin;
    
    std::string m_newCategoryName;
    std::string m_selectedCategoryFilter;

    std::string m_loadCategoryFilePath;
    
    bool m_showCreateDialog;
    bool m_showLoadDialog;

    void RefreshCategoryList();
    void DrawCreateCategoryDialog();
    void DrawLoadCategoryDialog();

    void TextEditorCallback();

    public:
    CategoryPanel(std::weak_ptr<Editor> _editor);
    ~CategoryPanel();

    void Draw();
    void SelectCategory(int _idx);
    std::string GetCategoryAt(int _idx);
    void CreateNewCategory(const std::string& _name);
    void LoadCategoryFromFile(const std::string& _path);
    void AssignCategoryToEntity(int _entityId, const std::string& _categoryName);

    bool IsDialogOpen();
};

#endif
