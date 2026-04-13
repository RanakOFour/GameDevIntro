-- resources/Tutorials/GettingStarted.lua
--
-- A tutorial is a table with an optional top-level "title" and a "steps" array.
-- Each step has:
--   title       (string)  – displayed in bold/yellow at the top of the panel
--   body        (string)  – scrollable description text; use \n for line breaks
--   image       (string)  – optional path to a texture shown above the body
--   highlight   (string)  – optional key registered by a panel widget (see TutorialPanel.h)
--   event       (string)  – "next" (default), "click_region", "click_panel", or "wait_state"
--   force_state (string)  – editor state to apply immediately on entering this step
--   wait_state  (string)  – condition checked each frame when event="wait_state"

return {
    title = "Getting Started",
    steps = {
        {
            title       = "Welcome to the Editor",
            force_state = "scene_tab",
            body        = [[Welcome! This editor is built around ECR:
  Entities    – objects that exist in your scene.
  Categories  – tables of named fields attached to entities (like components).
  Rules       – Lua scripts that run logic on every entity that has certain categories.

This tutorial walks you through the basics interactively.
Press "Next >" when you're ready to begin.]],
        },
        {
            title       = "The Entity List",
            force_state = "scene_tab",
            event       = "click_panel",
            highlight   = "Entity List",
            body        = [[The "Entity List" panel (top-left) shows every entity in the scene.

From here you can:
  • See all entities at a glance.
  • Click an entity's name to select it.
  • Use the context menu (right-click in the scene) to create or delete entities.

Click anywhere on the panel to continue.]],
        },
        {
            title       = "Creating an Entity",
            force_state = "scene_tab",
            event       = "wait_state",
            wait_state  = "entity_selected",
            body        = [[Right-click anywhere in the scene viewport and choose "Create Entity".

The entity will appear in the Entity List immediately.  Every new entity
automatically receives a Transform category with:
  Position  – world-space location (Vector2)
  Rotation  – rotation in degrees
  Scale     – size in X and Y (Vector2)
  Layer     – draw order (number)

Click the new entity in the Entity List to select it.
The tutorial will advance automatically once an entity is selected.]],
        },
        {
            title       = "Entity Properties",
            force_state = "scene_tab",
            event       = "click_panel",
            highlight   = "Entity Properties",
            body        = [[The "Entity Properties" panel opens automatically when you select an entity.

It shows every category the entity belongs to, along with each field and its
current value.  You can edit values directly:
  • Numbers  → type or scroll to change.
  • Booleans → checkbox.
  • Strings  → text input (with a file picker for fields named *Path).
  • Vectors  → drag handle or type into each component.

Click anywhere on the panel to continue.]],
        },
        {
            title       = "The Categories Panel",
            force_state = "scene_tab",
            event       = "click_panel",
            highlight   = "Categories",
            body        = [[The "Categories" panel lets you manage component data.

From here you can:
  • Load an existing category .lua file from disk.
  • Create a new blank category file in the project.
  • Assign a loaded category to the currently selected entity.
  • Remove a category from an entity.

Categories are loaded from your project's Categories/ folder and from the
built-in editor categories (read-only).

Click anywhere on the panel to continue.]],
        },
        {
            title       = "Anatomy of a Category File",
            force_state = "text_tab",
            body        = [[A category file is a plain Lua file that returns a Category table.

Example:
  return Category {
      Health    = 100,
      MaxHealth = 100,
      Alive     = true,
  }

The filename becomes the category's name automatically — no need to repeat it
inside the file.  Fields can be numbers, booleans, strings, Vector2, Vector3,
or Vector4.

The editor has switched to the Text Editor tab so you can open or create a
category file.  Press "Next >" when you're ready to continue.]],
        },
        {
            title       = "Writing Your First Rule",
            force_state = "text_tab",
            body        = [[Rules contain your game's logic.  Open the Rules panel and load a .lua file,
or create a new one in the Text Editor.

A minimal rule looks like this:

  local MyRule = Rule {
      categories = { "Transform", "Health" },
  }

  function MyRule:Update(_entityData)
      local tf = _entityData["Transform"]
      local hp = _entityData["Health"]
      -- move, damage, animate...
  end

  return MyRule

The rule's Update() is called once per entity that has ALL listed categories.]],
        },
        {
            title       = "Hot Reloading",
            force_state = "text_tab",
            body        = [[You can edit categories and rules while the editor is running.

  1. Open the file in the Text Editor tab.
  2. Make your changes.
  3. Press Ctrl+S (or the Save button) to save.

The category definition reloads immediately — new fields are added with
default values, and existing fields keep their current entity values.

Rules reload on the next scene Init (press Stop then Play again).]],
        },
        {
            title       = "You're Ready!",
            force_state = "scene_tab",
            body        = [[That covers the basics of the editor.

More tutorials are available under the Tutorials menu:
  • Creating Categories – deep dive into the Category system.
  • Writing Rules       – patterns and tips for Rule logic.

Have fun building your game!]],
        },
    }
}
