-- resources/Tutorials/GettingStarted.lua
--
-- A tutorial is a table with an optional top-level "title" and a "steps" array.
-- Each step has:
--   title       (string)  – displayed in bold/yellow at the top of the panel
--   body        (string)  – scrollable description text; use \n for line breaks
--   image       (string)  – optional path to a texture shown above the body
--   highlight   (string)  – optional key registered by a panel widget (see TutorialPanel.h)
--   event       (string)  – "next" (default), "click_region", or "wait_state"
--   force_state (string)  – editor state to apply immediately on entering this step
--   wait_state  (string)  – condition checked each frame when event="wait_state"

return {
    title = "Getting Started",
    steps = {
        {
            title       = "Welcome to the Editor",
            force_state = "scene_tab",
            body        = [[Welcome! This editor is built around a system called ECR:
  Entities – objects that exist in your scene.
  Categories – tables of named fields attached to entities (like components).
  Rules – Lua scripts that run logic on every entity that has certain categories.

This tutorial walks you through the basics.
Press "Next >" whenever you are ready to continue.]],
        },
        {
            title       = "The Scene View",
            force_state = "scene_tab",
            body        = [[The large viewport in the centre is your scene.

  • Right-click anywhere in the scene to open the context menu.
  • Use the context menu to create new entities.
  • Click an entity in the Entity List (top-left) to select it and see its properties.]],
        },
        {
            title       = "Creating an Entity",
            force_state = "scene_tab",
            event       = "wait_state",
            wait_state  = "entity_selected",
            body        = [[Right-click in the scene and choose "Create Entity".

A new entity will appear. Every entity automatically receives a Transform category with:
  Position  – where it sits in the world (Vector2)
  Layer     – draw order (number)
  Rotation  – rotation in degrees (number)
  Scale     – size in X and Y (Vector2)

Select the entity in the Entity List to inspect and edit its Transform values.
The tutorial will advance automatically once you select an entity.]],
        },
        {
            title       = "Opening the Category Panel",
            force_state = "scene_tab",
            event       = "wait_state",
            wait_state  = "panel_open:Categories",
            body        = [[Categories define what data an entity holds.

Open the Category panel by right-clicking the scene and choosing
"Show Category List", or from the panels already docked in the UI.

From there you can:
  • Load an existing category .lua file from disk.
  • Create a new blank category.
  • Assign a loaded category to the selected entity.

The tutorial will advance automatically once the Categories panel is open.]],
            highlight   = "Categories",
        },
        {
            title       = "Anatomy of a Category File",
            force_state = "text_tab",
            body        = [[A category file is a plain Lua file that returns a Category table:

  return Category {
      Health   = 100,
      MaxHealth = 100,
      Alive    = true,
  }

The filename becomes the category's name automatically – no need to repeat it inside the file.

Try opening or creating a category file in the Text Editor tab now.
The editor has switched to the Text Editor tab for you.]],
        },
        {
            title       = "Writing Your First Rule",
            force_state = "text_tab",
            body        = [[Rules contain the logic of your game.  Open the Rules panel and load a .lua file,
or create a new one in the Text Editor tab.

A minimal rule looks like this:

  local MyRule = Rule {
      categories = { "Transform", "Health" },
  }

  function MyRule:Update(_entityData)
      local tf = _entityData["Transform"]
      local hp = _entityData["Health"]
      -- your logic here
  end

  return MyRule

The rule runs once per entity that has ALL listed categories.]],
        },
        {
            title       = "Hot Reloading",
            force_state = "text_tab",
            body        = [[You can edit category and rule files while the editor is running.

  1. Open the file in the Text Editor tab.
  2. Make your changes.
  3. Press Ctrl+S (or the Save button) to save.

The category definition will reload immediately.  Any entities that were members
of the category will have their data updated – new fields are added with default values,
and fields that still exist keep their current values.]],
        },
        {
            title       = "You're Ready!",
            force_state = "scene_tab",
            body        = [[That covers the basics of the editor.

More tutorials are available under the Tutorials menu:
  • Creating Categories – deep dive into the Category system
  • Writing Rules      – patterns and tips for Rule logic

Have fun building your game!]],
        },
    }
}
