#ifndef CATEGORY_PANEL_H
#define CATEGORY_PANEL_H

#include "RanakEngine/RanakEngine.h"
#include <vector>
#include <string>
#include <memory>

class Editor;

class CategoryPanel
{
    private:
    std::weak_ptr<Editor> m_editor;
    bool m_showPanel;
    std::vector<std::string> m_availableCategories;
    
    std::string m_newCategoryName;
    std::string m_selectedCategoryFilter;
    
    bool m_showCreateDialog;
    bool m_showLoadDialog;

    void RefreshCategoryList();
    void DrawCreateCategoryDialog();
    void DrawLoadCategoryDialog();

    public:
    CategoryPanel(std::weak_ptr<Editor> _editor);
    ~CategoryPanel();

    void Draw();
    void CreateNewCategory(const std::string& _name);
    void LoadCategoryFromFile(const std::string& _path);
    void AssignCategoryToEntity(int _entityId, const std::string& _categoryName);
};

#endif
