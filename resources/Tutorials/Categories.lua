-- resources/Tutorials/Categories.lua
return {
    title = "Creating Categories",
    steps = {
        {
            title = "What is a Category?",
            body  = [[A Category is the ECR equivalent of a component.

It is a named set of fields with default values.  When an entity joins a category,
it receives its own independent copy of those fields.

Categories are defined as Lua files and live in resources/Categories/.]],
        },
        {
            title = "The Category Syntax",
            body  = [[A category file returns a Category table:

  return Category {
      Speed    = 5.0,
      Jumping  = false,
      JumpForce = 10.0,
  }

Supported field types:
  number   (integer or float)
  boolean
  string
  nil      (represents an unset / asset reference)
  Vector2  (e.g. Vector2.new(1.0, 0.0))
  nested tables

The filename (without extension) is used as the category name.]],
        },
        {
            title = "Field Metadata with Field()",
            body  = [[For richer editor support you can wrap a default value with Field():

  return Category {
      Speed = Field(5.0,  { label = "Move Speed", min = 0, max = 100 }),
      Layer = Field(0,    { label = "Draw Layer", min = -100, max = 100 }),
  }

This does NOT change how Rules access the data – they still read entity.Speed normally.
The opts table is available to editor panels for displaying sliders with ranges,
custom labels, and future validation.]],
        },
        {
            title = "Loading a Category",
            body  = [[To make a category available in the editor:

  1. Write the .lua file and save it in resources/Categories/.
  2. Open the Category panel.
  3. Click "Load" and pick your file.

The category is now registered in the engine and can be assigned to entities.]],
            highlight = "Categories",
            event     = "click_region",
        },
        {
            title = "Assigning a Category to an Entity",
            body  = [[Select an entity in the Entity List, then open the Category panel.

Click the category name in the list – this will add the category to the selected entity,
giving it a personal copy of all the default fields.

You can then view and edit those field values in the Properties panel.]],
        },
        {
            title = "Hot Reloading a Category",
            body  = [[Open a category file in the Text Editor tab.

Make changes (add/remove/rename fields), then save with Ctrl+S.

The engine will:
  1. Remove all entities from the old category definition.
  2. Apply the new definition.
  3. Re-add each entity, restoring any field values that still exist.

This means you can iterate on your data layout without restarting the editor.]],
        },
    }
}
