#ifndef CATEGORY_PANEL_H
#define CATEGORY_PANEL_H

#include "RanakEngine/RanakEngine.h"
#include "Editor/Panel.h"

#include "imgui/imgui.h"

#include <vector>
#include <string>
#include <memory>

class Editor;

/**
 * @class CategoryPanel
 * @brief Editor panel for loading, creating, and assigning Categories.
 *
 * Displays every Category registered in the engine.  From here the user
 * can create a new blank Category (from a name), load an existing .lua file
 * from disk, or assign the selected Category to the currently selected entity.
 * Also provides a DrawAsChild() variant used inside the TextEditTab layout.
 */
class CategoryPanel : public Panel
{
    private:
    std::vector<std::string> m_loadedCategories; ///< Names of all categories registered with the engine.

    std::shared_ptr<RE::Asset::LuaFile> m_selectedCategoryOrigin; ///< Source .lua file of the selected category.
    int m_selectedCategory; ///< Index into m_loadedCategories of the currently selected entry.

    std::string m_newCategoryName; ///< Name typed in the create-category dialog.

    bool m_showCreateDialog; ///< Whether the "Create New Category" modal is open.
    bool m_showLoadDialog;   ///< Whether the file-open dialog is in progress.

    /** @brief Re-queries the engine for the current list of registered category names. */
    void RefreshCategoryList();
    /** @brief Renders the modal dialog for creating a new category from a name. */
    void DrawCreateCategoryDialog();
    /** @brief Renders the file-chooser dialog for loading a category .lua file. */
    void DrawLoadCategoryDialog();

    void Draw() override;

    public:
    CategoryPanel() {};
    /**
     * @brief Constructs the panel.
     * @param _editor Weak pointer to the owning Editor.
     */
    CategoryPanel(std::weak_ptr<Editor> _editor);
    ~CategoryPanel();

    /**
     * @brief Selects the category at the given list index.
     * @param _idx Index into the loaded-categories list.
     */
    void SelectCategory(int _idx);
    /**
     * @brief Returns the name of the category at the given index.
     * @param _idx Index into the loaded-categories list.
     */
    std::string GetCategoryAt(int _idx);

    /**
     * @brief Creates and registers a new blank Category with the given name.
     * @param _name Name for the new category (also used as the .lua filename).
     */
    void CreateNewCategory(const std::string _name);
    /**
     * @brief Loads a Category from a .lua file and registers it with the engine.
     * @param _path Filesystem path to the .lua category file.
     */
    void LoadCategoryFromFile(const std::string _path);
    /**
     * @brief Adds the given entity to the named Category.
     * @param _entityId  ID of the entity to add.
     * @param _categoryName Name of the target Category.
     */
    void AssignCategoryToEntity(const int _entityId, const std::string _categoryName);

    /**
     * @brief Returns the LuaFile that was used to load the currently selected category.
     * @return Weak pointer to the source LuaFile, or expired if none is selected.
     */
    std::weak_ptr<RE::Asset::LuaFile> GetSelectedFile();

    /** @brief Returns true while either the create or load dialog is open. */
    bool IsDialogOpen();
};

#endif
