-- resources/Tutorials/Rules.lua
return {
    title = "Writing Rules",
    steps = {
        {
            title = "What is a Rule?",
            body  = [[A Rule is a Lua script that runs game logic.

It declares which categories an entity must have, and then defines
Init, Update, and/or Draw functions that run once per matching entity each frame.

Rules live in resources/Rules/.]],
        },
        {
            title = "The Rule Syntax",
            body  = [[A rule file returns a Rule table:

  local MyRule = Rule {
      categories = { "Transform", "Physics" },
      fields = {
          -- rule-level state, shared across all entities
          gravity = -9.8,
      },
  }

  function MyRule:Init(_entityData)
      -- called once when the scene starts
  end

  function MyRule:Update(_entityData)
      -- called every frame; _entityData holds the entity's category tables
  end

  function MyRule:Draw(_entityData)
      -- called every frame after Update; use for rendering
  end

  return MyRule]],
        },
        {
            title = "Accessing Entity Data",
            body  = [[Inside Update/Draw, _entityData is a table keyed by category name:

  function MyRule:Update(_entityData)
      local tf  = _entityData["Transform"]
      local vel = _entityData["Physics"]

      -- Read a field:
      local spd = vel.Speed

      -- Write a field (the change persists):
      tf.Position.x = tf.Position.x + spd
  end

Changes to _entityData are live – they affect the entity's stored data immediately.
There is no need to write back to a separate structure.]],
        },
        {
            title = "Rule-Level Fields",
            body  = [[Rule fields are shared state for the rule itself, not per-entity.

  local Spawner = Rule {
      categories = { "Transform" },
      fields = {
          spawnTimer = 0.0,
          spawnInterval = 3.0,
      },
  }

  function Spawner:Update(_entityData)
      self.fields.spawnTimer = self.fields.spawnTimer + 0.016
      if self.fields.spawnTimer >= self.fields.spawnInterval then
          self.fields.spawnTimer = 0.0
          -- spawn something ...
      end
  end

Access rule fields through self.fields inside the method body.]],
        },
        {
            title = "Available Global APIs",
            body  = [[The following globals are available in all rule scripts:

  Core.Camera   – get/set position, projection, draw entities
  IO            – keyboard input (GetKeyDown), audio (PlayAudio)
  Asset         – load textures, shaders, models, audio at runtime
  Log           – Log.Message(str), Log.Warning(str), Log.Error(str)
  Vector2       – Vector2.new(x, y), arithmetic, ToString()
  Category      – read-only; used in category file definitions
  Rule          – used to construct rule tables (the Rule { } syntax)

The autocomplete in the Text Editor (Ctrl+Space) lists all available functions.]],
        },
        {
            title = "Loading a Rule",
            body  = [[To add a rule to the running scene:

  1. Write the .lua file and save it in resources/Rules/.
  2. Open the Rules panel.
  3. Click "Load" and pick your file.

The rule becomes active immediately and will start processing entities
that match its category list on the next frame.

Use the Toggle button in the Rules panel to enable/disable a rule at runtime.]],
            highlight = "Rules",
            event     = "click_region",
        },
    }
}
