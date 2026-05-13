#include "Editor/Scene/BuiltIn/BuiltInTutorials.h"
#include "Editor/Core/TutorialRegistry.h"

std::vector<BuiltinTutorials::Entry> BuiltinTutorials::GetEntries()
{
    return {
        {
            "Getting Started",
            R"(
                local tutorial = {}
                tutorial.title = "Getting Started"
                tutorial.steps = {
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
                },)"
                R"({
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
                }
            }

            return tutorial
            )"
        },
        {
            "Asteroids",
            R"(
                return {
    title = "Build Asteroids – An Introduction to Programming & Game Making",
    steps = {
        {
            title       = "Welcome!",
            force_state = "scene_tab",
            body        = [[Welcome to the editor!

In this tutorial you will build the classic arcade game **Asteroids** –
completely from scratch.

More importantly, this tutorial is designed for people who have
**never programmed before**.  We will explain every concept before we
use it, so nothing should feel like magic.

By the end, you will have:

  - A working game you built yourself.
  - An understanding of the core ideas behind all programming.
  - Practical experience with this editor and the Lua language.

There is no rush.  Read every step carefully.  If something does not
make sense, re-read it – it will click.

Press **Next >** when you are ready to begin.]],
        },
        {
            title       = "What is Programming?",
            force_state = "scene_tab",
            body        = [[Before we touch the editor, let's answer the most important question:

**What is programming?**

A computer can only do exactly what it is told, in exact detail.
Programming is the act of writing those instructions.

Think of it like a recipe:

  - The **ingredients** are data (numbers, positions, timers).
  - The **recipe steps** are the instructions the computer follows.
  - The **dish** is your running game.

You write your recipe in a **programming language** – a special
language designed to give instructions to a computer.  We will be
using one called **Lua**, which was designed to be simple and easy to
learn.

Do not worry about memorising any of this yet.  We will introduce
each idea one piece at a time as we need it.

Press **Next >** to continue.]],
        },
        {
            title       = "Variables – Storing Information",
            force_state = "scene_tab",
            body        = [[The first building block of programming is the **variable**.

A variable is a **named container** that holds a value.  Think of a
labelled box: the label is the name, the contents are the value.

In Lua it looks like this:

  local speed = 5.0

This creates a box labelled "speed" and puts the value 5.0 inside it.
Later in the code you can:

  - **Read** it:    position.x + speed * dt
  - **Change** it:  speed = 10.0

Common types of values:

  Number   –  5.0,  -3,  200     (whole or decimal numbers)
  Boolean  –  true  or  false
  String   –  "hello"            (text, surrounded by quotes)

Throughout this tutorial, whenever you see a number in code, remember:
it is just a labelled box holding a value that you can change later.

Press **Next >** to continue.]],
        },
        {
            title       = "Functions – Reusable Instructions",
            force_state = "scene_tab",
            body        = [[The second big idea is the **function**.

A function is a **named group of instructions** you can run whenever
you want, just by using its name.

In Lua:

  local function greet()
      print("Hello!")
  end

  greet()   -- this runs the instructions inside

Functions can accept **inputs** (called parameters) and give back an
**output** (called a return value):

  local function add(a, b)
      return a + b
  end

  local result = add(3, 5)   -- result is now 8

In our game, every Rule has an **Update** function.  The engine calls
that function automatically every frame.  You write what should
happen inside it.

Press **Next >** to continue.]],
        },
        {
            title       = "Conditions – Making Decisions",
            force_state = "scene_tab",
            body        = [[The third key idea is the **condition** (also called an if statement).

A condition lets your code make a decision:

  if IO.GetKeyDown('w') then
      -- this only runs when W is held down
      position.y = position.y + 1
  end

You can add an alternative path with else:

  if lives > 0 then
      print("Still alive!")
  else
      print("Game over!")
  end

Conditions test whether something is true or false:

  x > 5    – is x greater than 5?
  x == 0   – is x equal to zero?  (two equals signs, not one)
  x <= 10  – is x less than or equal to 10?

That is the core of all programming:

  **Variables**  – hold data.
  **Functions**  – group instructions.
  **Conditions** – make decisions.

Everything else in this tutorial builds on these three ideas.

Press **Next >** to learn how games use them.]],
        },
        {
            title       = "The Game Loop",
            force_state = "scene_tab",
            body        = [[Here is something crucial about how games work:

**Games run in a loop.**

Every fraction of a second the engine:

  1. Checks for keyboard and mouse input.
  2. Runs all of your Update functions.
  3. Draws everything on screen.
  4. Goes back to step 1.

This repeats roughly 60 times per second.  Each pass is called a
**frame**.

This is why we multiply movement by **delta time** (written `dt` in
code).  `dt` is the duration of the most recent frame in seconds –
about 0.016 at 60 fps.

Multiplying by `dt` keeps movement at a consistent speed regardless
of how fast the computer is running:

  -- Moves 5 units per SECOND, not per frame
  position.x = position.x + 5.0 * dt

Without `* dt`, the object would move 5 units per *frame* –
that is 300 units per second on a 60 fps machine!

Press **Next >** to meet the editor.]],
        },
        {
            title       = "Meet the Editor",
            force_state = "scene_tab",
            body        = [[Now let's explore the editor window.

The window is divided into several areas:

**Tabs** (top of the window):
  - **Scene** – your game world lives here.
  - **Text Editor** – where you write Lua scripts.
  - **Console** – shows messages and errors from your scripts.

**Entity List** (left side):
  - Lists every object in your scene.
  - Click an entry to select it.

**Viewport** (large centre area):
  - The 3D view of your game world.
  - Right-click here to get a context menu.

**Panels** (bottom or sides):
  - **Entity Properties** – shows data for the selected entity.
  - **Categories** – manage data templates.
  - **Rules** – manage behaviour scripts.

Take a moment to look at each of these areas.

Press **Next >** to look at each area in more detail.]],
        },
        {
            title       = "The Entity List",
            force_state = "scene_tab",
            highlight   = "Entity List",
            body        = [[Let's look at the **Entity List** on the left.

Right now it is empty – there are no objects in the scene yet.

In game engines, every object in the world is called an **Entity**.
Entities can be:

  - The player's ship.
  - A bullet.
  - An asteroid.
  - An invisible manager that spawns new objects.

An entity by itself does nothing.  It only becomes interesting when
you attach **Categories** (data) and **Rules** (behaviour) to it.

You can:

  - **Click** an entity to select it.
  - **Right-click** the viewport to create new ones.
  - **Double-click** an entity's name in the list to rename it.

Press **Next >** to look at Entity Properties.]],
        },
        {
            title       = "Entity Properties",
            force_state = "scene_tab",
            highlight   = "Entity Properties",
            body        = [[The **Entity Properties** panel shows the data attached to the
currently selected entity.

Every entity automatically gets a **Transform** category which holds:

  - **Position** – where the entity is in the world (x, y).
  - **Rotation** – which direction it faces, in degrees.
  - **Scale** – its size along x and y.

When you create your own Categories (coming soon), their fields also
appear here.  This is how you inspect and tweak values while building
your game.

Think of this panel as the "property sheet" for the selected object.

Press **Next >** to look at the viewport.]],
        },
        {
            title       = "The Viewport",
            force_state = "scene_tab",
            body        = [[The **Viewport** is the large area where your game world is displayed.

While in the **editor**:
  - You can orbit the camera to view the scene from different angles.
  - Right-click to open a context menu with actions like Create Entity.

While the game is **playing**:
  - The viewport shows the game as the player would see it.
  - Your Lua scripts control everything that moves.

The coordinate system works like this:

  - (0, 0) is the centre of the screen.
  - Positive X goes right.
  - Positive Y goes up.

For Asteroids, everything happens in a flat 2D plane, so we only care
about X and Y.

Press **Next >** to start building the game!]],
        },
        {
            title       = "Create the Ship Entity",
            force_state = "scene_tab",
            event       = "wait_state",
            wait_state  = "entity_selected",
            highlight   = "Entity List",
            body        = [[Time to create our first entity – the player's spaceship.

  1. **Right-click** anywhere in the viewport.
  2. Choose **Create Entity** from the menu.
     A new entry appears in the Entity List.
  3. **Click** that entity in the Entity List to select it.

Once selected, its Transform data will appear in Entity Properties.

You can also double-click its name in the Entity List to rename it
to "Ship" – this keeps things organised but is optional.

The tutorial advances automatically once an entity is selected.]],
        },
        {
            title       = "Setting the Ship's Transform",
            force_state = "scene_tab",
            body        = [[With the ship entity selected, find the **Entity Properties** panel.

You will see the Transform category with Position, Rotation and Scale.
Set them to:

  Position:  (0, 0)        ← centre of the screen
  Rotation:  90            ← pointing upward
  Scale:     (0.3, 0.5)    ← a narrow, tall rectangle

**Why these values?**

  - Position (0, 0) starts the ship in the middle.
  - Rotation 90° means the front faces upward (positive Y direction).
  - Scale (0.3, 0.5) makes it look like a small ship.

Click into each field in Entity Properties and type the new values.

Press **Next >** once you have set them.]],
        },
        {
            title       = "What are Categories?",
            force_state = "scene_tab",
            body        = [[You have used the built-in Transform category.  Now let's understand
what categories really are.

A **Category** is a collection of named variables grouped under one
label, which can be attached to any entity.

Think of it like a form with labelled fields:

  Ship form:
  ┌──────────────────────────────┐
  │ speed:          5.0          │
  │ rotSpeed:     200.0          │
  │ shootTimer:     0.0          │
  │ shootInterval:  0.25         │
  └──────────────────────────────┘

When you attach this "form" to an entity, that entity gains all those
fields.  A different entity (an asteroid) has a completely different
form with different fields.

You define categories in **Lua files** and load them into the editor.
We will write four categories for the game:

  - **Ship**     – speed, rotation speed, shooting timer.
  - **Bullet**   – velocity and lifetime.
  - **Asteroid** – velocity and radius.
  - **Spawner**  – spawn interval and cap.

Press **Next >** to switch to the Text Editor.]],
        },
        {
            title       = "Opening the Text Editor",
            force_state = "text_tab",
            body        = [[Click the **Text Editor** tab at the top of the window.
(Your view may have switched automatically.)

This is a built-in code editor where you will write all your Lua
scripts.  Useful shortcuts:

  Ctrl+N – create a new file.
  Ctrl+S – save the current file.
  Ctrl+Z – undo.

In the Text Editor you write Lua code, save it into your project
folder, and the engine loads it from there.

In the next few steps we will write the four category files.  Take
your time – there is no hurry.

Press **Next >** when you are ready to write your first file.]],
        },
        {
            title       = "A Note About Lua Category Files",
            force_state = "text_tab",
            body        = [[Before writing the first category, here is how category files work.

Each file uses a special keyword provided by the engine:

  return Category { ... }

The `{ ... }` part is a **table** – Lua's way of grouping values
together (similar to the "form" described earlier).

Inside the table, each line is a field definition:

  fieldName = defaultValue,

The `=` sets the starting (default) value.  Every entity that gets
this category starts with these defaults.  You can change the values
in Entity Properties afterwards.

The `--` starts a **comment** – text that the computer ignores.
Comments are notes for the humans reading the code:

  speed = 5.0,   -- how fast the ship moves (units per second)

Press **Next >** to write the first category file.]],
        },
        {
            title       = "Creating Ship.lua",
            force_state = "text_tab",
            body        = [[Press **Ctrl+N** to create a new file.

Type or paste the code below, then save it (**Ctrl+S**) inside your
project's **Categories/** folder as **Ship.lua**:

  return Category {
      speed         = 5.0,      -- how fast the ship moves (units/sec)
      rotSpeed      = 200.0,    -- rotation speed in degrees/sec
      shootTimer    = 0.0,      -- internal cooldown (starts ready)
      shootInterval = 0.25,     -- minimum seconds between shots
  }

**What each field does:**

  speed         – distance moved per second when W is held.
  rotSpeed      – degrees rotated per second with A or D.
  shootTimer    – a countdown that prevents firing too fast.
  shootInterval – the gap between shots (0.25 sec = max 4 per second).

Save the file, then press **Next >**.]],
        },
        {
            title       = "Creating Bullet.lua",
            force_state = "text_tab",
            body        = [[Create another new file (**Ctrl+N**) and save it as
**Categories/Bullet.lua**:

  return Category {
      velX     = 0.0,   -- horizontal velocity
      velY     = 0.0,   -- vertical velocity
      lifetime = 2.0,   -- seconds before the bullet is removed
      speed    = 12.0,  -- how fast the bullet travels
      entityId = Field(0, { hidden = true }),
  }

**About entityId:**

Each entity has a unique ID number so the engine can find it.
When a bullet is created, we store its own ID inside this field so
the rules can later say "remove *this specific* bullet".

It is marked `hidden` because you never need to edit it yourself –
the rules manage it automatically.

Save the file and press **Next >**.]],
        },
        {
            title       = "Creating Asteroid.lua",
            force_state = "text_tab",
            body        = [[Create a new file and save it as **Categories/Asteroid.lua**:

  return Category {
      velX     = 0.0,   -- horizontal velocity
      velY     = 0.0,   -- vertical velocity
      radius   = 1.0,   -- collision size (used for hit detection)
      entityId = Field(0, { hidden = true }),
  }

**What radius does:**

When checking whether a bullet has hit an asteroid, we measure the
distance between them.  If the distance is less than the asteroid's
`radius`, it is a hit.  A larger radius means the asteroid is
easier to hit.

Save the file and press **Next >**.]],
        },
        {
            title       = "Creating Spawner.lua",
            force_state = "text_tab",
            body        = [[Create a new file and save it as **Categories/Spawner.lua**:

  return Category {
      spawnTimer    = 0.0,   -- countdown until the next asteroid appears
      spawnInterval = 2.0,   -- seconds between spawns
      maxAsteroids  = 12,    -- maximum number of asteroids at one time
  }

**How the spawner works:**

We will attach this category to a special invisible entity.
Every frame, `spawnTimer` counts down.  When it reaches zero, a new
asteroid is created and the timer resets to `spawnInterval`.)"

R"(`maxAsteroids` stops the game from creating too many asteroids and
grinding to a halt.

Save the file and press **Next >**.]],
        },
        {
            title       = "What are Rules?",
            force_state = "text_tab",
            body        = [[You have written the data (categories).  Now we need **behaviour**.

A **Rule** is a Lua script that runs every frame for every entity
that has the matching categories attached.

The structure of every rule looks like this:

  local MyRule = Rule {
      categories = { "Transform", "MyCategory" },
  }

  function MyRule:Update(_entityData)
      -- This runs every frame for each matching entity.
      local tf  = _entityData["Transform"]
      local cat = _entityData["MyCategory"]
  end

  return MyRule

Key points:

  - `categories` lists which categories an entity MUST have for
    this rule to run on it.
  - `_entityData["Transform"]` gives you that entity's Transform
    fields (Position, Rotation, Scale).
  - Changes you make to `tf.Position` are applied immediately.

We will write four rules.  Press **Next >** to start the first one.]],
        },
        {
            title       = "A Little Maths: Working Out Directions",
            force_state = "text_tab",
            body        = [[The ship needs to move in the direction it faces.

To work out that direction from a rotation angle, we use **sine**
and **cosine** (sin and cos).  You do not need to understand the
maths deeply – just the key idea:

  If the ship faces angle θ (in radians):
    dirX = cos(θ)   ← how much to move horizontally
    dirY = sin(θ)   ← how much to move vertically

To move the ship forward by `speed` units per second:

  position.x = position.x + dirX * speed * dt
  position.y = position.y + dirY * speed * dt

The engine does not expose the standard Lua maths library, so we
define our own sin/cos using a **Taylor series** – a technique for
approximating these functions with arithmetic.  The code is
pre-written for you; just copy it as-is.

We also need to convert **degrees to radians** first (the engine
stores rotation in degrees, but sin/cos work in radians):

  radians = degrees × π ÷ 180

The engine provides `Math.DegToRad()` to handle this.

Press **Next >** to write ShipControl.lua.]],
        },
        {
            title       = "Writing ShipControl.lua",
            force_state = "text_tab",
            body        = [[Create a new file and save it as **Rules/ShipControl.lua**.

Paste the full script below.  Each section has comments explaining it:

-- Simple sine and cosine using a Taylor series.
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

-- ShipControl runs on any entity that has both Transform and Ship.
local ShipControl = Rule {
    categories = { "Transform", "Ship" },
}

function ShipControl:Update(_entityData)
    local tf   = _entityData["Transform"]   -- built-in position/rotation/scale
    local ship = _entityData["Ship"]        -- our custom category data
    local dt   = Core.DeltaTime()           -- seconds since last frame

    -- Rotate with A/D
    if IO.GetKeyDown('a') then
        tf.Rotation = tf.Rotation + ship.rotSpeed * dt
    end
    if IO.GetKeyDown('d') then
        tf.Rotation = tf.Rotation - ship.rotSpeed * dt
    end

    -- Calculate forward direction from current rotation.
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

    -- Screen wrapping: when the ship leaves one edge, it appears on the opposite.
    local camW  = Core.Camera:getCameraWidth()
    local scrn  = IO.GetScreenSize()
    local halfW = camW / 2
    local halfH = halfW / (scrn.x / scrn.y)   -- aspect ratio
    local px = tf.Position.x
    local py = tf.Position.y
    if px >  halfW then px = -halfW end
    if px < -halfW then px =  halfW end
    if py >  halfH then py = -halfH end
    if py < -halfH then py =  halfH end
    tf.Position = Vector2(px, py)

    -- Shooting with Space (can fire repeatedly at the shoot interval)
    ship.shootTimer = ship.shootTimer - dt
    if IO.GetKeyDown(' ') and ship.shootTimer <= 0 then
        ship.shootTimer = ship.shootInterval

        -- Create a new bullet entity.
        local bId = Core.CurrentScene:addEntity()
        Core.CurrentScene:addToCategory(bId, "Bullet")
        local bData = Core.CurrentScene:getAttributesOf(bId)
        local bTf   = bData["Transform"]
        local bBul  = bData["Bullet"]

        -- Place bullet just in front of the ship.
        bTf.Position = Vector2(
            tf.Position.x + dirX * 0.6,
            tf.Position.y + dirY * 0.6
        )
        bTf.Rotation = tf.Rotation
        bTf.Scale    = Vector2(0.15, 0.15)
        bBul.velX     = dirX * bBul.speed
        bBul.velY     = dirY * bBul.speed
        bBul.entityId = bId   -- store the ID for later removal
    end
end

return ShipControl

Save the file and press **Next >**.]],
        },
        {
            title       = "Understanding ShipControl",
            force_state = "text_tab",
            body        = [[Let's walk through ShipControl section by section.

**Rotation (A / D keys)**

  tf.Rotation = tf.Rotation + ship.rotSpeed * dt

Each frame while A is held, the rotation increases by
200° × 0.016 ≈ 3.2°.  That is a smooth, controlled spin.

**Thrust (W key)**

  tf.Position = Vector2(pos.x + dirX * speed * dt, ...)

`dirX` and `dirY` come from converting the rotation angle with
cos and sin.  The ship moves in exactly the direction it faces.

**Screen wrapping**

  if px > halfW then px = -halfW end

When the ship crosses the right edge, it teleports to the left
edge.  The same check is applied to all four edges.

**Shooting**

`shootTimer` counts down from `shootInterval` each frame.
When Space is pressed AND the timer has reached zero, a new bullet
entity is created, positioned just ahead of the ship, and launched
in the ship's forward direction.  The timer then resets, enforcing
the firing cooldown.

Press **Next >** to write BulletLogic.lua.]],
        },
        {
            title       = "Writing BulletLogic.lua",
            force_state = "text_tab",
            body        = [[Create a new file and save it as **Rules/BulletLogic.lua**:

  local BulletLogic = Rule {
      categories = { "Transform", "Bullet" },
  }

  function BulletLogic:Update(_entityData)
      local tf = _entityData["Transform"]
      local b  = _entityData["Bullet"]
      local dt = Core.DeltaTime()

      -- Move the bullet along its stored velocity.
      tf.Position = Vector2(
          tf.Position.x + b.velX * dt,
          tf.Position.y + b.velY * dt
      )

      -- Count down lifetime; remove the bullet when it expires.
      b.lifetime = b.lifetime - dt
      if b.lifetime <= 0 then
          Core.CurrentScene:removeEntity(b.entityId)
          return
      end

      -- Check for collision with every asteroid.
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
                  -- Hit! Destroy the asteroid and the bullet.
                  Core.CurrentScene:removeEntity(aAst.entityId)
                  Core.CurrentScene:removeEntity(b.entityId)
                  return
              end
          end
      end
  end

  return BulletLogic

**About the collision check:**

  dx*dx + dy*dy < radius*radius

We compare the *squared* distance to the *squared* radius instead of
computing a square root.  This gives the same result but is faster.
(Pythagoras: distance² = dx² + dy²)

Save the file and press **Next >**.]],
        },
        {
            title       = "Writing AsteroidLogic.lua",
            force_state = "text_tab",
            body        = [[Create a new file and save it as **Rules/AsteroidLogic.lua**:

  local AsteroidLogic = Rule {
      categories = { "Transform", "Asteroid" },
  }

  function AsteroidLogic:Update(_entityData)
      local tf  = _entityData["Transform"]
      local ast = _entityData["Asteroid"]
      local dt  = Core.DeltaTime()

      -- Drift in a straight line.
      tf.Position = Vector2(
          tf.Position.x + ast.velX * dt,
          tf.Position.y + ast.velY * dt
      )

      -- Screen wrap (with margin so asteroids appear fully off-screen first).
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

      -- Collision with the player ship.
      local ships = Core.CurrentScene:getEntitiesWith({"Ship"})
      if #ships > 0 then
          local sData = Core.CurrentScene:getAttributesOf(ships[1])
          if sData then
              local sTf = sData["Transform"]
              local dx  = tf.Position.x - sTf.Position.x
              local dy  = tf.Position.y - sTf.Position.y
              local hitDist = ast.radius + 0.25   -- approximate ship size
              if dx * dx + dy * dy < hitDist * hitDist then
                  -- Ship hit – reset to centre and destroy the asteroid.
                  sTf.Position = Vector2(0, 0)
                  Core.CurrentScene:removeEntity(ast.entityId)
                  return
              end
          end
      end
  end

  return AsteroidLogic

When an asteroid hits the ship, the ship resets to the centre and
the asteroid is destroyed.  Simple, but it works!

Save the file and press **Next >**.]],
        },
        {
            title       = "Writing AsteroidSpawner.lua",
            force_state = "text_tab",
            body        = [[Create the last rule file and save it as **Rules/AsteroidSpawner.lua**:

  local PI = 3.14159265358979

  local function sin(x)
      x = x % (2 * PI)
      if x > PI then x = x - 2 * PI end
      local x2 = x * x
      return x * (1 - x2 / 6 * (1 - x2 / 20 * (1 - x2 / 42)))
  end
  local function cos(x) return sin(x + PI / 2) end

  -- A simple number generator (engine does not provide math.random).
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

      local existing = Core.CurrentScene:getEntitiesWith({"Asteroid"})
      if #existing >= sp.maxAsteroids then return end

      local camW  = Core.Camera:getCameraWidth()
      local scrn  = IO.GetScreenSize()
      local halfW = camW / 2
      local halfH = halfW / (scrn.x / scrn.y)

      -- Pick a random edge and a random position along it.
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

      -- Aim roughly toward the centre with a random spread.
      local toCX = -px;  local toCY = -py
      local len  = Vector2(toCX, toCY):Length()
      toCX = toCX / len;  toCY = toCY / len
      local spread = randomRange(-0.7, 0.7)
      local cs = cos(spread);  local sn = sin(spread)
      local speed  = randomRange(1.5, 3.5)
      local vx = (toCX * cs - toCY * sn) * speed
      local vy = (toCX * sn + toCY * cs) * speed
      local radius = randomRange(0.5, 1.5)

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

**About the random number generator:**

The "Lehmer" generator multiplies a seed by a large prime and uses
the remainder.  It is not cryptographically secure, but perfectly
fine for games.  Changing `local _seed = 42` to any other number
gives a completely different sequence of asteroids.

All the code is written!  Save and press **Next >**.]],
        },
        {
            title       = "Opening the Categories Panel",
            force_state = "scene_tab",
            event       = "click_panel",
            highlight   = "Categories",
            body        = [[Switch back to the **Scene** tab and find the **Categories** panel.

Click it to open it.  The tutorial advances automatically once you
do so.

(If you are not sure where it is, look at the bottom and sides of
the window.  Panel positions can be rearranged.)]],
        },
        {
            title       = "Loading Your Categories",
            force_state = "scene_tab",
            body        = [[Inside the Categories panel, load all four category files:

  1. Click the **Load** button.
  2. Navigate to your project's **Categories/** folder.
  3. Select **Ship.lua** and click Open.
  4. Repeat for **Bullet.lua**, **Asteroid.lua**, and **Spawner.lua**.

After loading, all four should appear in the categories list.

**What is happening?**

The engine reads each Lua file, builds the category definition, and
makes it available to attach to entities.  Until you load a category
file, the engine does not know it exists.

Press **Next >** when all four are loaded.]],
        },
        {
            title       = "Attach the Ship Category",
            force_state = "scene_tab",
            body        = [[Now attach the **Ship** category to your ship entity.

  1. Click the ship entity in the **Entity List** to select it.
  2. In the Categories panel, click **Ship**.
  3. Click **Assign to Entity**.

You should now see the Ship fields (speed, rotSpeed, etc.) appear
inside **Entity Properties** alongside the Transform.

**Try it:** Change `speed` to 10.0 and see the difference when you
play later.  Change it back to 5.0 afterwards if you prefer.

Press **Next >** after the assignment.]],
        },
        {
            title       = "Create the Spawner Entity",
            force_state = "scene_tab",
            body        = [[The AsteroidSpawner rule needs an entity to run on.  Let's create
an invisible one.

  1. **Right-click** the viewport → **Create Entity**.
  2. Select the new entity in the Entity List.
  3. Double-click its name and rename it "Spawner".
  4. In Entity Properties set its Scale to **(0, 0)** so it is invisible.
  5. In the Categories panel, select **Spawner** and click
     **Assign to Entity**.

**Why an invisible entity?**

The AsteroidSpawner rule only runs on entities that have the Spawner
category.  By making its Scale (0, 0) it takes up no visible space
and cannot be collided with – it is just a container for the
spawner data.)"

R"(Press * *Next >**when done.]],
        },
        {
            title       = "Opening the Rules Panel",
            force_state = "scene_tab",
            event       = "click_panel",
            highlight   = "Rules",
            body        = [[Find the **Rules** panel and click it to open it.

The tutorial advances automatically when you click the panel.

Rules is where you load all the behaviour scripts you wrote.
Once loaded, the engine automatically applies each rule to every
entity that has the matching categories.]],
        },
        {
            title       = "Loading Your Rules",
            force_state = "scene_tab",
            body        = [[Inside the Rules panel, load all four rule files:

  1. Click **Load**.
  2. Navigate to your project's **Rules/** folder.
  3. Load **ShipControl.lua**.
  4. Repeat for **BulletLogic.lua**, **AsteroidLogic.lua**,
     and **AsteroidSpawner.lua**.

Once all four appear in the rules list, the engine will run them
every frame.

**A quick recap of what each rule does:**

  ShipControl     – input, thrust, shooting, screen wrap for the ship.
  BulletLogic     – moves bullets, expires them, detects asteroid hits.
  AsteroidLogic   – moves asteroids, wraps them, detects ship hit.
  AsteroidSpawner – creates new asteroids at random edges over time.

Press **Next >** when all four are loaded.]],
        },
        {
            title       = "Play Your Game!",
            force_state = "scene_tab",
            body        = [[Everything is in place.  Let's play!

Press the **Play** button in the toolbar (or **Ctrl+P**).

Controls:

  A / D    – rotate left / right
  W        – thrust forward
  Space    – fire bullets

Asteroids will start appearing after a couple of seconds and drift
toward the centre.  Shoot them before they reach you!

**If something goes wrong:**

  1. Press **Stop** to end the game.
  2. Click the **Console** tab – it shows any Lua error messages.
  3. The error will say which file and line number the problem is on.
  4. Switch to the **Text Editor**, fix the issue, save, and try again.

This cycle – run, read the error, fix, repeat – is called
**debugging**, and it is a completely normal part of programming.
Every developer does it.

Press **Next >** when you are done playing.]],
        },
        {
            title       = "Experiment: Tweak the Values",
            force_state = "scene_tab",
            body        = [[Before we finish, try experimenting with the values.

This is one of the most important skills in game development.
Change a number, test it, decide if it feels better or worse.
Professional developers do this constantly.

**Suggested experiments:**

  1. Select the ship entity.  In Entity Properties, change
     **Ship → speed** from 5.0 to 15.0.  Play and feel the difference.

  2. Change **Ship → rotSpeed** from 200.0 to 50.0.
     Is the game harder or easier to control?

  3. Change **Spawner → spawnInterval** from 2.0 to 0.5 to make
     things hectic.

  4. Change **Spawner → maxAsteroids** from 12 to 30 for chaos.

  5. Open **Rules/ShipControl.lua**, find `bBul.speed` and try
     increasing the bullet speed.  How does it feel?

  6. In **Categories/Ship.lua**, change `shootInterval` to 0.05.
     Now you have a machine gun!

After editing a script you do not need to reload – just save,
stop the game, and press Play again.

Press **Next >** when you are done experimenting.]],
        },
        {
            title       = "Congratulations!",
            force_state = "scene_tab",
            body        = [[You just built a complete Asteroids game from scratch – and learned
the fundamentals of programming along the way!

**Programming concepts you have used:**

  Variables   – named containers that hold data.
  Functions   – named groups of reusable instructions.
  Conditions  – if/else decisions that change behaviour.
  Loops       – the `for` loop that checks every asteroid.
  Delta time  – keeping movement speed frame-rate independent.

**Editor skills you have practised:**

  Creating and configuring entities.
  Writing and loading custom Categories.
  Writing and loading Rules.
  Reading error messages from the Console.
  Tuning values to change how the game feels.

**Ideas for extending the game:**

  Add a score – create a GameState category and display it in a
  Rule's Draw function.

  Split large asteroids into two smaller ones when hit.

  Add lives – give the player three chances before game over.

  Make asteroids gradually speed up to increase difficulty.

  Give the ship momentum instead of instant stop-and-go movement.

The best way to keep learning is to keep building.
Pick one of the ideas above and try to implement it yourself!]],
        },

    }
    }
            )"
        }
    };
};

void BuiltinTutorials::Load(TutorialRegistry& _registry)
{
    for (const auto& _tutorial : GetEntries())
    {
        _registry.RegisterTutorial(
            _tutorial.name,
            _tutorial.source
        );
    }
}