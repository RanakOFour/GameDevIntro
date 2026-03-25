#ifndef CATEGORY_PANEL_H
#define CATEGORY_PANEL_H

#include "RanakEngine/RanakEngine.h"
#include "Editor/Panel.h"

#include "imgui/imgui.h"

#include <vector>
#include <string>
#include <memory>

class Editor;

class CategoryPanel : public Panel
{
    private:
    std::vector<std::string> m_loadedCategories;

    std::shared_ptr<RE::Asset::LuaFile> m_selectedCategoryOrigin;
    int m_selectedCategory;
    
    std::string m_newCategoryName;
    
    bool m_showCreateDialog;
    bool m_showLoadDialog;

    void RefreshCategoryList();
    void DrawCreateCategoryDialog();
    void DrawLoadCategoryDialog();

    public:
    CategoryPanel() {};
    CategoryPanel(std::weak_ptr<Editor> _editor);
    ~CategoryPanel();

    void Draw();
    void DrawAsChild(ImGuiChildFlags _flags);
    
    void SelectCategory(int _idx);
    std::string GetCategoryAt(int _idx);

    void CreateNewCategory(const std::string _name);
    void LoadCategoryFromFile(const std::string _path);
    void AssignCategoryToEntity(const int _entityId, const std::string _categoryName);

    std::weak_ptr<RE::Asset::LuaFile> GetSelectedFile();

    bool IsDialogOpen();
};

#endif
