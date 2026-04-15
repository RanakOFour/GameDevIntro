-- resources/Tutorials/Asteroids.lua
--
-- A step-by-step tutorial that guides the user through building the classic
-- arcade game Asteroids inside the editor.  Covers custom categories, rules
-- with player input, runtime entity spawning/destruction, distance-based
-- collision detection, and screen wrapping.

return {
    title = "Build Asteroids",
    steps = {
        ------------------------------------------------------------------
        -- 1. Introduction
        ------------------------------------------------------------------
        {
            title       = "Introduction",
            force_state = "scene_tab",
            body        = [[In this tutorial we will build the classic arcade game Asteroids.

You will learn how to:
  - Create custom Categories to define game objects.
  - Write Rules with player input, movement, and shooting.
  - Spawn and destroy entities at runtime.
  - Detect collisions using distance checks.
  - Wrap objects around the screen edges.

The game features:
  - A ship that rotates (A/D) and thrusts (W).
  - Bullets fired with the Space bar.
  - Asteroids that spawn from screen edges and drift inward.
  - Screen wrapping for all objects.

We will create four categories and four rules:

  Categories: Ship, Bullet, Asteroid, Spawner
  Rules:      ShipControl, BulletLogic, AsteroidLogic, AsteroidSpawner

Press "Next >" when you are ready to begin.]],
        },

        ------------------------------------------------------------------
        -- 2. Create the ship entity
        ------------------------------------------------------------------
        {
            title       = "Creating the Ship Entity",
            force_state = "scene_tab",
            event       = "wait_state",
            wait_state  = "entity_selected",
            body        = [[First, create the player's ship.

  1. Right-click anywhere in the scene viewport.
  2. Choose "Create Entity".
  3. Click the new entity in the Entity List to select it.

Once selected, open the Entity Properties panel and set:

  Position:  (0, 0)       centre of the screen
  Rotation:  90           pointing upward
  Scale:     (0.3, 0.5)   tall rectangle so "forward" is visible

The tutorial advances automatically once an entity is selected.]],
        },

        ------------------------------------------------------------------
        -- 3. Creating the categories
        ------------------------------------------------------------------
        {
            title       = "Creating the Categories",
            force_state = "text_tab",
            body        = [[Switch to the Text Editor and create four files inside your project's
Categories/ folder.  Each file defines one category.

1) Ship.lua -- player ship data

  return Category {
      speed         = 5.0,
      rotSpeed      = 200.0,
      shootTimer    = 0.0,
      shootInterval = 0.25,
  }

2) Bullet.lua -- projectile data

  return Category {
      velX     = 0.0,
      velY     = 0.0,
      lifetime = 2.0,
      speed    = 12.0,
      entityId = Field(0, { hidden = true }),
  }

3) Asteroid.lua -- enemy rock data

  return Category {
      velX     = 0.0,
      velY     = 0.0,
      radius   = 1.0,
      entityId = Field(0, { hidden = true }),
  }

4) Spawner.lua -- spawner configuration

  return Category {
      spawnTimer    = 0.0,
      spawnInterval = 2.0,
      maxAsteroids  = 12,
  }

The "entityId" fields use Field(0, { hidden = true }) so rules can store
each entity's ID at spawn time for later removal.  They are hidden from
the Properties panel because the user never edits them directly.

Create and save (Ctrl+S) each file, then press "Next >".]],
        },

        ------------------------------------------------------------------
        -- 4. ShipControl rule
        ------------------------------------------------------------------
        {
            title       = "The ShipControl Rule",
            force_state = "text_tab",
            body        = [[Create ShipControl.lua in your project's Rules/ folder.

Because the engine does not open Lua's math library, we define our own
sine and cosine with a Taylor series.  Paste the entire file below:

  local PI = 3.14159265358979

  local function sin(x)
      x = x % (2 * PI)
      if x > PI then x = x - 2 * PI end
      local x2 = x * x
      return x * (1 - x2 / 6 * (1 - x2 / 20 * (1 - x2 / 42)))
  end

  local function cos(x)
      return sin(x + PI / 2)
  end

  local ShipControl = Rule {
      categories = { "Transform", "Ship" },
  }

  function ShipControl:Update(_entityData)
      local tf   = _entityData["Transform"]
      local ship = _entityData["Ship"]
      local dt   = Core.DeltaTime()

      -- Rotate with A / D
      if IO.GetKeyDown('a') then
          tf.Rotation = tf.Rotation + ship.rotSpeed * dt
      end
      if IO.GetKeyDown('d') then
          tf.Rotation = tf.Rotation - ship.rotSpeed * dt
      end

      -- Forward direction from current rotation
      local rad  = Math.DegToRad(tf.Rotation)
      local dirX = cos(rad)
      local dirY = sin(rad)

      -- Thrust forward with W
      if IO.GetKeyDown('w') then
          tf.Position = Vector2(
              tf.Position.x + dirX * ship.speed * dt,
              tf.Position.y + dirY * ship.speed * dt
          )
      end

      -- Screen wrapping
      local camW  = Core.Camera:getCameraWidth()
      local scrn  = IO.GetScreenSize()
      local halfW = camW / 2
      local halfH = halfW / (scrn.x / scrn.y)
      local px = tf.Position.x
      local py = tf.Position.y
      if px >  halfW then px = -halfW end
      if px < -halfW then px =  halfW end
      if py >  halfH then py = -halfH end
      if py < -halfH then py =  halfH end
      tf.Position = Vector2(px, py)

      -- Shoot with Space (auto-fire while held)
      ship.shootTimer = ship.shootTimer - dt
      if IO.GetKeyDown(' ') and ship.shootTimer <= 0 then
          ship.shootTimer = ship.shootInterval

          local bId = Core.CurrentScene:addEntity()
          Core.CurrentScene:addToCategory(bId, "Bullet")
          local bData = Core.CurrentScene:getAttributesOf(bId)
          local bTf   = bData["Transform"]
          local bBul  = bData["Bullet"]

          bTf.Position = Vector2(
              tf.Position.x + dirX * 0.6,
              tf.Position.y + dirY * 0.6
          )
          bTf.Rotation = tf.Rotation
          bTf.Scale    = Vector2(0.15, 0.15)
          bBul.velX     = dirX * bBul.speed
          bBul.velY     = dirY * bBul.speed
          bBul.entityId = bId
      end
  end

  return ShipControl

Key points:
  - sin/cos use a Taylor expansion (accurate to ~6 decimal places).
  - Math.DegToRad converts Transform.Rotation (degrees) to radians.
  - Bullets are spawned as new entities with the Bullet category.
  - Each bullet's entityId is saved so BulletLogic can remove it later.

Save the file and press "Next >".]],
        },

        ------------------------------------------------------------------
        -- 5. BulletLogic rule
        ------------------------------------------------------------------
        {
            title       = "The BulletLogic Rule",
            force_state = "text_tab",
            body        = [[Create BulletLogic.lua in your Rules/ folder.

This rule runs on every entity that has both Transform and Bullet.
It moves the bullet, counts down its lifetime, and checks for collisions
with asteroids using a simple distance test.

  local BulletLogic = Rule {
      categories = { "Transform", "Bullet" },
  }

  function BulletLogic:Update(_entityData)
      local tf = _entityData["Transform"]
      local b  = _entityData["Bullet"]
      local dt = Core.DeltaTime()

      -- Move
      tf.Position = Vector2(
          tf.Position.x + b.velX * dt,
          tf.Position.y + b.velY * dt
      )

      -- Expire
      b.lifetime = b.lifetime - dt
      if b.lifetime <= 0 then
          Core.CurrentScene:removeEntity(b.entityId)
          return
      end

      -- Collision with asteroids
      local asteroids = Core.CurrentScene:getEntitiesWith({"Asteroid"})
      for i = 1, #asteroids do
          local aId   = asteroids[i]
          local aData = Core.CurrentScene:getAttributesOf(aId)
          if aData then
              local aTf  = aData["Transform"]
              local aAst = aData["Asteroid"]
              local dx = tf.Position.x - aTf.Position.x
              local dy = tf.Position.y - aTf.Position.y
              if dx * dx + dy * dy < aAst.radius * aAst.radius then
                  Core.CurrentScene:removeEntity(aAst.entityId)
                  Core.CurrentScene:removeEntity(b.entityId)
                  return
              end
          end
      end
  end

  return BulletLogic

The collision test compares the squared distance between the bullet and
each asteroid against the asteroid's squared radius.  Squaring avoids an
expensive square-root call.

Save the file and press "Next >".]],
        },

        ------------------------------------------------------------------
        -- 6. AsteroidLogic rule
        ------------------------------------------------------------------
        {
            title       = "The AsteroidLogic Rule",
            force_state = "text_tab",
            body        = [[Create AsteroidLogic.lua in your Rules/ folder.

Asteroids drift in a straight line, wrap around the screen, and respawn
the ship at the centre if they collide with it.

  local AsteroidLogic = Rule {
      categories = { "Transform", "Asteroid" },
  }

  function AsteroidLogic:Update(_entityData)
      local tf  = _entityData["Transform"]
      local ast = _entityData["Asteroid"]
      local dt  = Core.DeltaTime()

      -- Move
      tf.Position = Vector2(
          tf.Position.x + ast.velX * dt,
          tf.Position.y + ast.velY * dt
      )

      -- Screen wrap (offset by radius so they fully exit before reappearing)
      local camW  = Core.Camera:getCameraWidth()
      local scrn  = IO.GetScreenSize()
      local halfW = camW / 2
      local halfH = halfW / (scrn.x / scrn.y)
      local m  = ast.radius
      local px = tf.Position.x
      local py = tf.Position.y
      if px >  halfW + m then px = -halfW - m end
      if px < -halfW - m then px =  halfW + m end
      if py >  halfH + m then py = -halfH - m end
      if py < -halfH - m then py =  halfH + m end
      tf.Position = Vector2(px, py)

      -- Collision with the player ship
      local ships = Core.CurrentScene:getEntitiesWith({"Ship"})
      if #ships > 0 then
          local sData = Core.CurrentScene:getAttributesOf(ships[1])
          if sData then
              local sTf = sData["Transform"]
              local dx  = tf.Position.x - sTf.Position.x
              local dy  = tf.Position.y - sTf.Position.y
              local hitDist = ast.radius + 0.25
              if dx * dx + dy * dy < hitDist * hitDist then
                  sTf.Position = Vector2(0, 0)
                  Core.CurrentScene:removeEntity(ast.entityId)
                  return
              end
          end
      end
  end

  return AsteroidLogic

When an asteroid touches the ship the asteroid is destroyed and the ship
teleports back to (0, 0).  A "hitDist" of radius + 0.25 accounts for
the ship's approximate size.

Save the file and press "Next >".]],
        },

        ------------------------------------------------------------------
        -- 7. AsteroidSpawner rule
        ------------------------------------------------------------------
        {
            title       = "The AsteroidSpawner Rule",
            force_state = "text_tab",
            body        = [[Create AsteroidSpawner.lua in your Rules/ folder.

This rule runs on a dedicated spawner entity.  Every few seconds it places
a new asteroid just outside a random screen edge with a velocity aimed
roughly toward the centre.

It includes its own sin/cos helpers and a simple pseudo-random number
generator (the engine does not provide math.random).

  local PI = 3.14159265358979

  local function sin(x)
      x = x % (2 * PI)
      if x > PI then x = x - 2 * PI end
      local x2 = x * x
      return x * (1 - x2 / 6 * (1 - x2 / 20 * (1 - x2 / 42)))
  end
  local function cos(x) return sin(x + PI / 2) end

  local _seed = 42
  local function random()
      _seed = (_seed * 16807) % 2147483647
      return _seed / 2147483647
  end
  local function randomRange(lo, hi)
      return lo + random() * (hi - lo)
  end

  local AsteroidSpawner = Rule {
      categories = { "Transform", "Spawner" },
  }

  function AsteroidSpawner:Update(_entityData)
      local sp = _entityData["Spawner"]
      local dt = Core.DeltaTime()

      sp.spawnTimer = sp.spawnTimer - dt
      if sp.spawnTimer > 0 then return end
      sp.spawnTimer = sp.spawnInterval

      -- Respect the asteroid cap
      local existing = Core.CurrentScene:getEntitiesWith({"Asteroid"})
      if #existing >= sp.maxAsteroids then return end

      -- Random edge position
      local camW  = Core.Camera:getCameraWidth()
      local scrn  = IO.GetScreenSize()
      local halfW = camW / 2
      local halfH = halfW / (scrn.x / scrn.y)
      local side = random()
      local px, py
      if side < 0.25 then
          px = randomRange(-halfW, halfW);  py =  halfH + 1
      elseif side < 0.5 then
          px = randomRange(-halfW, halfW);  py = -halfH - 1
      elseif side < 0.75 then
          px = -halfW - 1;  py = randomRange(-halfH, halfH)
      else
          px =  halfW + 1;  py = randomRange(-halfH, halfH)
      end

      -- Velocity aimed toward centre with random spread
      local toCX = -px
      local toCY = -py
      local len  = Vector2(toCX, toCY):Length()
      toCX = toCX / len
      toCY = toCY / len
      local spread = randomRange(-0.7, 0.7)
      local cs = cos(spread)
      local sn = sin(spread)
      local speed  = randomRange(1.5, 3.5)
      local vx = (toCX * cs - toCY * sn) * speed
      local vy = (toCX * sn + toCY * cs) * speed

      -- Random size
      local radius = randomRange(0.5, 1.5)

      -- Spawn
      local aId = Core.CurrentScene:addEntity()
      Core.CurrentScene:addToCategory(aId, "Asteroid")
      local aData = Core.CurrentScene:getAttributesOf(aId)
      local aTf   = aData["Transform"]
      local aAst  = aData["Asteroid"]
      aTf.Position  = Vector2(px, py)
      aTf.Scale     = Vector2(radius, radius)
      aAst.velX     = vx
      aAst.velY     = vy
      aAst.radius   = radius
      aAst.entityId = aId
  end

  return AsteroidSpawner

The random() function is a Lehmer generator seeded with 42.  Each game
session produces the same sequence, but the varying player-driven spawn
timing makes it feel different every time.

The velocity direction starts as "toward centre", then is rotated by a
random angle (-0.7 to 0.7 radians, roughly -40 to +40 degrees) so
asteroids don't all converge on the exact same point.

Save the file and press "Next >".]],
        },

        ------------------------------------------------------------------
        -- 8. Open the Categories panel
        ------------------------------------------------------------------
        {
            title       = "Loading the Categories",
            force_state = "scene_tab",
            event       = "click_panel",
            highlight   = "Categories",
            body        = [[Switch back to the Scene tab and open the Categories panel.

We need to load the four category files we created so the editor knows
about them.

Click the Categories panel to continue.]],
        },

        ------------------------------------------------------------------
        -- 9. Assembling the game
        ------------------------------------------------------------------
        {
            title       = "Assembling the Game",
            force_state = "scene_tab",
            body        = [[Almost there!  Follow these steps to connect everything:

1) Load categories
   In the Categories panel click "Load" and browse to your project's
   Categories/ folder.  Load each file:
     Ship.lua   Bullet.lua   Asteroid.lua   Spawner.lua

2) Assign Ship to the ship entity
   Select your ship entity in the Entity List.  In the Categories panel
   click "Ship" then "Assign to Entity".  The Ship fields should appear
   in the Entity Properties panel.

3) Create a Spawner entity
   Right-click the viewport and choose "Create Entity".
   Select the new entity and set its Scale to (0, 0) so it is invisible.
   Assign the "Spawner" category to it via the Categories panel.

4) Load rules
   Open the Rules panel (right-click the viewport and choose
   "Show Rules List" if it is not visible).
   Click "Load" and browse to your Rules/ folder.  Load:
     ShipControl.lua
     BulletLogic.lua
     AsteroidLogic.lua
     AsteroidSpawner.lua

5) Press Play!
   Use the Play button in the toolbar.

     A / D    rotate left / right
     W        thrust forward
     Space    fire

If something goes wrong, press Stop, check the Console panel for error
messages, fix the Lua file in the Text Editor tab, save, and try again.

Press "Next >" when you are done playing.]],
        },

        ------------------------------------------------------------------
        -- 10. Congratulations
        ------------------------------------------------------------------
        {
            title       = "Congratulations!",
            force_state = "scene_tab",
            body        = [[You have built Asteroids!

What you learned:
  - Creating custom Categories to define per-entity data.
  - Writing Rules that run Update() once per matching entity each frame.
  - Spawning and destroying entities at runtime with addEntity(),
    addToCategory(), and removeEntity().
  - Detecting collisions with squared-distance checks.
  - Screen wrapping using camera width and aspect ratio.
  - Implementing sin, cos, and a random number generator in Lua when
    the standard math library is unavailable.

Ideas for extending the game:
  - Add a score counter (create a GameState category and display the
    score with Log.Message or a HUD rule).
  - Split large asteroids into two smaller ones when hit.
  - Track lives: count ship-asteroid collisions and end the game after
    a set number.
  - Add Model or Texture categories to give objects real sprites.
  - Gradually decrease spawnInterval over time to ramp up difficulty.

Have fun experimenting!]],
        },
    }
}
