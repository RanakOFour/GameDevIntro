#ifndef PROPERTIESPANEL_H
#define PROPERTIESPANEL_H

#include "Editor/Panel.h"
#include "RanakEngine/RanakEngine.h"

#include "imgui/imgui.h"

#include <map>
#include <optional>

/**
 * @class PropertiesPanel
 * @brief Editor panel that displays and edits the category fields of the selected entity.
 *
 * Appears near the selected entity's screen position.  Shows each Category the entity
 * belongs to as a collapsible tree of editable fields.  Also provides a button to
 * assign the entity to additional Categories.  Position and size are controlled
 * programmatically so the panel floats next to the entity.
 */
class PropertiesPanel : public Panel
{
	private:
	std::map<std::string, std::string> m_stringValueMap; ///< Scratchpad for string field edits.
	std::map<int, std::string> m_entityNameMap;          ///< Optional display name per entity ID.
	int m_lastSelectedEntity = -1;                       ///< Tracks selection changes for cache invalidation.

	bool m_showAddToCategory; ///< Whether the "Assign to Category" sub-panel is expanded.

	bool        m_showPathDialog;      ///< Whether the path file-chooser dialog is in progress.
	std::string m_pendingPathKey;      ///< Key into m_stringValueMap for the field awaiting a path.
	std::string m_pendingPathProperty; ///< Attribute name to write the chosen path back into.
	sol::table  m_pendingPathTable;    ///< The entity's category attribute table to update on confirm.

	bool   m_setPosition; ///< True when SetPosition() has been called and not yet consumed.
	ImVec2 m_position;    ///< Requested next-frame window position.
	ImVec2 m_size;        ///< Last rendered window size (reported back by ImGui).

	std::optional<std::reference_wrapper<RE::Core::EntityRegistry>> m_registry; ///< Ref to the scene registry.

	/**
	 * @brief Draws all category fields for the given entity.
	 * @param _entityId ID of the entity whose properties to display.
	 */
	void DrawEntityProperties(int _entityId);
	/**
	 * @brief Draws the editable fields of a single category table.
	 * @param _entityId     ID of the owning entity.
	 * @param _categoryName Name of the category.
	 * @param _attributes   Lua table of field name → value pairs.
	 */
	void DrawCategoryAttributes(int _entityId, std::string _categoryName, sol::table _attributes);

	/** @brief Renders the file-chooser dialog for path properties and applies the result. */
	void DrawPathDialog();

	/** @brief Renders the floating properties window. */
	void Draw() override;

	public:
	/**
	 * @brief Constructs the panel.
	 * @param _editor Reference to the owning Editor.
	 */
	PropertiesPanel(Editor& _editor);
	~PropertiesPanel();

	/**
	 * @brief Requests that the panel move to a new screen position next frame.
	 * @param _pos Screen-space position in pixels.
	 */
	void SetPosition(ImVec2 _pos);
	/** @brief Returns the last rendered size of the panel window. */
	ImVec2 GetSize();

	/** @brief Clears cached entity data (call after scene reload to prevent stale entries). */
	void Reset();
};

#endif