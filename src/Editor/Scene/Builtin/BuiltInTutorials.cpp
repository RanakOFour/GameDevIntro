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
            R"(return {
    title = "Asteroids",
    steps = {
        {
            title       = "Welcome!",
            force_state = "scene_tab",
            body        = [[Welcome to the editor.

In this tutorial you will build the classic arcade game Asteroids from scratch.
You will fly a small ship, dodge rocks drifting across the screen, and shoot
them down before they hit you. This tutorial is designed for someone who has
never written code before. Every idea is introduced before it is used.
If a sentence does not click the first time, read it again, then keep going.
Ideas become familiar with repetition.

By the end you will have:
- A working game you built yourself.
- A working mental model of how programs are written.
- Hands-on practice with this editor and the Lua language.

When you are ready, click the Next button at the bottom of this panel.]],
        },
        {
            title       = "What exactly are we doing?",
            force_state = "scene_tab",
            body        = [[Before we do anything, it is important to understand what programming 'does' so we know exactly what we are doing when we are 'programming'.

A 'program' is a set of steps that solve a specific problem.
For this tutorial, our problem is 'playing asteroids'. So when we are programming,
we are writing instructions for the computer to do that will make it play our game.

Programming broadly contains 3 types of instructions:

1. Creating variables - data that can change (hence 'variable') and be referenced later
2. Making decisions e.g. "If the player presses W, move the ship forward." 
3. Manipulating existing variables - To move the ship forward, change it's position variable

We will be writing instructions in a language called Lua. Lua was designed to be simple
to read, which makes it a friendly first language.

Click Next when you are ready.]],
        },
        {
            title       = "How a Game Runs: The Frame Loop",
            force_state = "scene_tab",
            body        = [[There is one extra idea you need before we touch the editor: how a game actually runs. 
A game is not a single script that executes top to bottom. Instead,
90% of the instructions occur in a large loop. Each pass through the
loop is called a frame, and on every frame the program:

1. Reads the keyboard, mouse and other I/O (Input/Output) devices.
2. Update the game to respond those inputs.
3. Draws the result on screen.

That is it, forever, until the game stops. Each iteration of the loop is
called a 'frame'. The game engine used in this application is limited to
running at 60 fps (frames per second).

There are times when it makes sense to think about game mechanics in terms
of frames, and other times when it is better to think in seconds.
For this, the engine gives you a number called DeltaTime (Often shortened to
just 'dt') that says how long the most recent frame took. A typical value is
around 0.016 seconds.

To move something 5 units per second, you write:

position.x = position.x + 5 * dt

If you forgot the '* dt' part, the object would move 5 units every frame,
which is 300 units per second at 60 fps. That is the difference between 
walking and teleporting.

Inside this engine, deltatime is hidden behind a function, called Core.DeltaTime(),
so the actual code would look like this:

position.x = position.x + 5 * Core.DeltaTime()

Click Next for an overview of the editor.]],
        },
        {
            title       = "A Tour of the Editor",
            force_state = "scene_tab",
            body        = [[The window in front of you has two distinct modes; the Scene View where you will build your game; and the Text Editor where you will write Lua scripts that form the logic of your game.

In Scene View, there are also several panels:
- "Entity List" shows every object currently in the scene.
- "Entity Properties" displays the data attached to the selected entity.
- "Category List" shows data templates you have loaded.
- "Rules List" shows behaviour scripts you have loaded.

Most of these panels can be opened by right clicking the screen and
opening them from the context menu. The Entity Properties panel is
opened when selecting an entity on the screen with the mouse.

Click Next when you are ready.]],
        },
        {
            title       = "The Entity List",
            force_state = "scene_tab",
            highlight   = "Entity List",
            body        = [[Look at the "Entity List" panel.

In this engine, every object in the game is called an entity.
An entity can be the player's ship, a bullet, a chunk of rock,
or even an invisible object whose only job is to spawn other entities.

Right now there are no entities. You will change this. 
Three things you can do with the Entity List:
- Click an entity's name to select it.
- Right-click in the viewport and pick "Create Entity" to make one.
- Double-click an entity's name to rename it.

An entity by itself does nothing. It is just a name.
It comes to life only when you attach data and behavior to it.
That is the next idea we will cover.

Click Next to continue.]],
        },
        {
            title       = "Entity Properties and the Transform",
            force_state = "scene_tab",
            highlight   = "Entity Properties",
            body        = [[When you select an entity, the "Entity Properties" panel shows the data attached to it.

Every entity is automatically given one piece of data called the Transform. It contains:
- Position where the entity is in the world, as (x, y).
- Rotation which way it faces, in degrees.
- Scale how big it is along the x and y axes.

You can click into any of these fields and type a new value.
The viewport updates immediately to reflect the change.
Later you will define your own data alongside the Transform,
and those fields will appear in this same panel.]],
        },
        {
            title       = "The Viewport and Coordinates",
            force_state = "scene_tab",
            body        = [[The viewport is the large area in the middle of the window.
It shows your game from the camera's point of view.
The coordinate system works like graph paper:
- (0, 0) is the centre of the screen.
- Positive x goes to the right.
- Positive y goes up.

So Position (3, 0) means three units to the right of centre,
and Position (0, -2) means two units below centre. When you
press the Play button in the toolbar, the editor switches into
play mode and your scripts start running. There are no loaded
scripts currently, so nothing will happen. Press Stop to return to editing.]],
        },
        {
            title       = "Entities, Categories, and Rules",
            force_state = "scene_tab",
            body        = [[This engine builds games out of three pieces.
Entities are one of them.
The other two are what we are about to use.

- Categories: A labelled set of data attached to entities.
- Rules: instructions that manipulate the data in certain entities.

The Transform you saw earlier is a category. You can define your own.
You can also create rules. Rules are Lua scripts that run every frame
for every entity that has a particular set of categories.
Read that one more time. The pattern is: entities belong to categories, and 
rules run on entities that have certain category combinations. 

For Asteroids we will define four categories:

- Ship: The player's stats and shooting cooldown.
- Bullet: a bullet's velocity and remaining lifetime.
- Asteroid: an asteroid's velocity and size.
- Spawner: data for the script that creates new asteroids.

And four rules to act on them, one per category. We will write the categories first, then the rules. 

Click Next.]],
        },
        {
            title       = "Variables, in Plain Lua",
            force_state = "scene_tab",
            body        = [[Before writing the first category file, one tiny piece of syntax.
A variable is a named slot that holds a value. In Lua you create one like this:

local speed = 5.0
  
That single line says: "make a slot called speed, and put 5.0 inside it."
Later you can read it or change it:

-- Example of a read
position.x = position.x + speed * dt

-- Example of a change
speed = 10.0

The word local means "this slot is only visible inside the script that creates it."
You will see it on almost every variable.

There are several kinds of values we will use:
- Numbers: 5.4, -3, 200 (whole or decimal)
- Booleans: true or false
- Strings: "hello" (text in double (") or single (') quotes)
- Tables: a group of named variables, written with { ... }

A category file is just one big table. You will see in a moment.]],
        },
        {
            title       = "Create the Ship Entity",
            force_state = "scene_tab",
            event       = "wait_state",
            wait_state  = "entity_selected",
            highlight   = "Entity List",
            body        = [[Time to create the first entity — the player's ship.
1. Right-click anywhere in the viewport.
2. Choose "Create Entity" from the menu.

A new entry appears in the Entity List. Click that entry in the Entity List to select it.
Once the entity is selected, the Transform will show up in Entity Properties. Set its values to:

Position (0, 0) centre of the screen
Rotation 0 pointing upward
Scale (0.3, 0.5) a small, tall rectangle

You can also double-click the entity's name to rename it to Ship. The tutorial advances automatically once you have an entity selected.]],
        },
)"
            R"(        {
            title       = "Open the Text Editor",
            force_state = "text_tab",
            body        = [[Look at the "Text Editor" tab. The view may have
switched to it already. This is where you write Lua scripts.

The basics:
- "Ctrl+N" creates a new file.
- "Ctrl+S" saves the current file.
- "Ctrl+Z" undoes your last change.

Your project has two folders we care about:
- Categories: where category files live.
- Rules: where rule scripts live.

When you save a file into one of those folders, the engine
can load it from the panel on the Scene tab. You will do that shortly.]],
        },
        {
            title       = "Write Ship.lua",
            force_state = "text_tab",
            body        = [[Press "Ctrl+N" for a new file. Type the code below,
then save the file as Ship.lua inside the Categories folder.

return Category {
	speed = 5.0,
	rotSpeed = 200.0,
	shootTimer = 0.0,
	shootInterval = 0.25,
}

What each line means:
- return Category { ... } : Tells the engine: "this file defines a category."
- speed = 5.0 : How fast the ship moves, in units per second.
- rotSpeed = 200.0 : How quickly the ship rotates, in degrees per second.
- shootTimer = 0.0 : A countdown the rule will use to limit firing rate. Starts at zero, meaning "ready to fire right away."
- shootInterval = 0.25 : The minimum gap between shots, in seconds. 0.25 means at most four shots per second.

The comma at the end of each line is required. The text after
the last value (return Category) is what ties it all together. Save the file, then click Next.]],
        },
        {
            title       = "Write Bullet.lua",
            force_state = "text_tab",
            body        = [[Create another new file ("Ctrl+N") and save it as Bullet.lua inside the Categories folder.

return Category {
	velocity = Vector2(0.0)
	lifetime = 2.0,
	speed = 12.0,
	entityId = Field(0, { hidden = true }),
}

Velocity is the bullet's velocity. The ship's Rule will fill
these in when it spawns the bullet. Lifetime is the number of seconds
the bullet lives before disappearing. Without this, bullets would fly
forever and slowly choke the game. EntityId is the bullet's own ID number.
Every entity has a unique ID, and we store it here so the rule can later
say "remove this specific bullet." Field(0, { hidden = true }) is a special
form that means "default to zero, and do not show this field in the Entity
Properties panel" - you never set it by hand.

Save and click Next.]],
        },
        {
            title       = "Write Asteroid.lua",
            force_state = "text_tab",
            body        = [[Create a new file and save it as Asteroid.lua inside the Categories folder.

return Category {
	velocity = Vector2(0.0),
	radius = 1.0,
	entityId = Field(0, { hidden = true }),
}

Radius is the asteroid's size for hit detection. When a bullet gets
closer to the asteroid than this distance, we count it as a hit. A
larger radius means an easier target.
EntityId is again the asteroid's own ID, used when destroying it.

Save and click Next.]],
        },
        {
            title       = "Write Spawner.lua",
            force_state = "text_tab",
            body        = [[Create the last category file and save it as Spawner.lua inside the Categories folder.

return Category {
	spawnTimer = 0.0,
	spawnInterval = 4.0,
	maxAsteroids = 12,
}

SpawnTimer is the countdown until the next asteroid appears.
The spawner rule will decrease it every frame and reset it to
spawnInterval when it hits zero. spawnInterval is the gap between
spawns, in seconds. MaxAsteroids is a safety limit. Without it the
spawner would keep adding asteroids forever and the game would slow to a crawl.

We will attach this category to an invisible entity
in a moment. The entitys only purpose is to give the
spawner rule something to run on.

Save and click Next.]],
        },
        {
            title       = "What a Rule Looks Like",
            force_state = "text_tab",
            body        = [[You have written the data. Now we add the behaviour. A rule is a Lua script with this shape:

local MyRule = Rule {
	categories = { "Transform", "MyCategory" },
}

function MyRule:Update(_entityData)
	local tf = _entityData["Transform"]
	local cat = _entityData["MyCategory"]
-- runs every frame for each matching entity
end

function MyRule:Draw(_entityData)

-- Also runs every frame, but only during the draw phase (After Update)
end

return MyRule

The categories list says: "this rule only runs on entities that
have all of these categories at once." If an entity is missing
one of them, the engine skips it.

MyRule:Update is a function, which means it is a callable piece of code.
The ':' between MyRule and Update is shorthand for writing the function as:
MyRule.Update(MyRule, _entityData).

The words in then brackets are called 'parameters'. The _entityData parameter
is a table containing the fields of every category the entity has. You pull out
the parts you need with brackets, like _entityData["Transform"].

Click Next.]],
        },
        {
            title       = "A Pinch of Maths: Direction From Angle",
            force_state = "text_tab",
            body        = [[The ship needs to move in the direction it is facing.
Given a rotation angle, we get the direction with two functions called sine
and cosine (sin and cos). You do not need to understand the maths behind them. Just the following:

direction = Vector2(Math.Cos(shipAngle), Math.Sin(shipAngle))

Then to move forward:
position = position + direction * speed * dt

Click Next to write ShipControl.lua.]],
        },
)"
            R"(        {
            title       = "Write ShipControl.lua",
            force_state = "text_tab",
            body        = [[Create a new file and save it as ShipControl.lua inside the Rules folder.

local ShipControl = Rule {
	categories = { "Transform", "Ship" },
}

function ShipControl:Update(_entityData)
	local tf = _entityData["Transform"]
	local ship = _entityData["Ship"]
	local dt = Core.DeltaTime()

	if IO.GetKeyDown('a') then
		tf.Rotation = tf.Rotation + ship.rotSpeed * dt
	end

	if IO.GetKeyDown('d') then
		tf.Rotation = tf.Rotation - ship.rotSpeed * dt
	end

	
	local direction = Vector2(Math.Cos(shipAngle), Math.Sin(shipAngle))

	if IO.GetKeyDown('w') then
		tf.Position = tf.Position + (direction * ship.speed * dt)
	end

-- We need to make sure that when the ship leaves the screen, it reappears on the other side
	local camW = Core.Camera:getCameraWidth()
	local screenSize = IO.GetScreenSize()

	local halfW = camW / 2
	local halfH = halfW / (screenSize.x / screenSize.y)

	local px, py = tf.Position.x, tf.Position.y

	-- Flip ship if it goes off the screen on the X axis
	if px > halfW then px = -halfW end
	if px < -halfW then px = halfW end

	-- Flip ship if it goes off the screen on the Y axis
	if py > halfH then py = -halfH end
	if py < -halfH then py = halfH end

	tf.Position = Vector2(px, py)

	ship.shootTimer = ship.shootTimer - dt

	if(IO.GetKeyDown(' ') and ship.shootTimer <= 0) then
		ship.shootTimer = ship.shootInterval
		local bId = Core.CurrentScene:addEntity()
		Core.CurrentScene:addToCategory(bId, "Bullet")
		local bData = Core.CurrentScene:getAttributesOf(bId)
	
		local bTf = bData["Transform"]
		local bBul = bData["Bullet"]
		
		-- Spawn bullet slightly ahead of the ship
		bTf.Position = tf.Position + direction * 0.6
		bTf.Rotation = tf.Rotation
		bTf.Scale = Vector2(0.15, 0.15)

		bBul.velocity = direction * bBul.speed
		bBul.entityId = bId
	end
end

return ShipControl

Click next when ready]],
        },
        {
            title       = "Reading ShipControl, Section by Section",
            force_state = "text_tab",
            body        = [[Take a moment to map the script to its four jobs.
- Rotation. Holding A or D adds or subtracts rotSpeed times dt every frame.
  With rotSpeed at 200, a 60 fps frame rotates the ship by about 3.3 degrees.

- Thrust. Holding W moves the ship along its facing direction, computed from
  cos and sin of the rotation. The ship moves where its nose points, not where
  the camera points.

- Screen wrap. We ask the camera for its width and the window for its aspect ratio,
  then compute half-extents in world units. If the ship crosses an edge, we teleport
  it to the opposite edge.

- Shooting. shootTimer counts down every frame. When Space is held and the timer has
  reached zero, we create a new entity, attach the Bullet category to it, place it slightly
  ahead of the ship, and copy the ship's forward direction into the bullet's velocity. The timer
  is reset to shootInterval, enforcing the cooldown.


Click Next to write the bullet's rule.]],
        },
        {
            title       = "Write BulletLogic.lua",
            force_state = "text_tab",
            body        = [[Create a new file and save it as BulletLogic.lua inside the Rules folder.

local BulletLogic = Rule {
	categories = { "Transform", "Bullet" },
}

function BulletLogic:Update(_entityData)
	local tf = _entityData["Transform"]
	local b = _entityData["Bullet"]
	local dt = Core.DeltaTime()
	tf.Position = tf.Position + b.velocity * dt
	b.lifetime = b.lifetime - dt
	
	if b.lifetime <= 0 then
		Core.CurrentScene:removeEntity(b.entityId)
		return
	end
	
	local asteroids = Core.CurrentScene:getEntitiesWith({"Asteroid"})
	-- #asteroids means 'the number of asteroids'
	for i = 1, #asteroids do
		local aId = asteroids[i]
		local aData = Core.CurrentScene:getAttributesOf(aId)
		if aData then
			local aTf = aData["Transform"]
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

The collision check uses squared distances:

dx * dx + dy * dy < radius * radius

That avoids a square root and gives the same answer as comparing the real
distance to the radius. It is a tiny speed-up that you will see in almost every game engine.

Save and click Next.]],
        },
        {
            title       = "Write AsteroidLogic.lua",
            force_state = "text_tab",
            body        = [[Create a new file and save it as AsteroidLogic.lua inside the Rules folder.

local AsteroidLogic = Rule {
	categories = { "Transform", "Asteroid" },
}

function AsteroidLogic:Update(_entityData)
	local tf = _entityData["Transform"]
	local ast = _entityData["Asteroid"]
	local dt = Core.DeltaTime()
	tf.Position = tf.Position + ast.velocity * dt

local camW = Core.Camera:getCameraWidth()
local scrn = IO.GetScreenSize()
local halfW = camW / 2
local halfH = halfW / (scrn.x / scrn.y)
local m = ast.radius

local px, py = tf.Position.x, tf.Position.y

if px > halfW + m then px = -halfW - m end
if px < -halfW - m then px = halfW + m end

if py > halfH + m then py = -halfH - m end
if py < -halfH - m then py = halfH + m end

tf.Position = Vector2(px, py)
local ships = Core.CurrentScene:getEntitiesWith({"Ship"})

if #ships > 0 then
	local sData = Core.CurrentScene:getAttributesOf(ships[1])
	if sData then
		local sTf = sData["Transform"]
		local dx = tf.Position.x - sTf.Position.x
		local dy = tf.Position.y - sTf.Position.y
		local hit = ast.radius + 0.25
		
		if dx * dx + dy * dy < hit * hit then
			sTf.Position = Vector2(0, 0)
			Core.CurrentScene:removeEntity(ast.entityId)
			return
		end
	end
end

end

return AsteroidLogic

Two jobs:
- Drift in a straight line, wrapping at the screen edge.
  The extra m margin lets the asteroid leave the screen fully
  before reappearing on the other side, which looks nicer than popping.

- Check whether this asteroid has touched the ship. If so, snap the ship
  back to the centre and remove the asteroid.

Save and click Next.]],
        },
)"
            R"(        {
            title       = "Write AsteroidSpawner.lua",
            force_state = "text_tab",
            body        = [[Create the last rule file and save it as AsteroidSpawner.lua inside the Rules folder.

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
			local camW = Core.Camera:getCameraWidth()
			local scrn = IO.GetScreenSize()
			local halfW = camW / 2
			local halfH = halfW / (scrn.x / scrn.y)
			local side = random()
			local px, py
			
			if side < 0.25 then
				px = randomRange(-halfW, halfW); py = halfH + 1
			elseif side < 0.5 then
				px = randomRange(-halfW, halfW); py = -halfH - 1
			elseif side < 0.75 then
				px = -halfW - 1; py = Math.RandomRange(-halfH, halfH)
			else
				px = halfW + 1; py = Math.RandomRange(-halfH, halfH)
			end
			
			local len = Vector2(toCX, toCY):Length()
			local toCX = -px / len
			local toCY = -py / len
			
			local spread = Math.RandomRange(-0.7, 0.7)
			local cs = Math.Cos(spread)
			local sn = sin(spread)
			
			local speed = Math.RandomRange(1.5, 3.5)
			
			local radius = Math.RandomRange(0.5, 1.5)
			local aId = Core.CurrentScene:addEntity()
			Core.CurrentScene:addToCategory(aId, "Asteroid")
			local aData = Core.CurrentScene:getAttributesOf(aId)
			
			local aTf = aData["Transform"]
			local aAst = aData["Asteroid"]
			aTf.Position = Vector2(px, py)
			aTf.Scale = Vector2(radius, radius)
			aAst.velocity = Vector2((toCX * cs - toCY * sn) * speed,
								    (toCX * sn + toCY * cs) * speed
									)
			aAst.radius = radius
			aAst.entityId = aId
		end
		
return AsteroidSpawner

Save and click Next.]],
        },
        {
            title       = "Load the Categories",
            force_state = "scene_tab",
            event       = "click_panel",
            highlight   = "Categories",
            body        = [[Switch back to the "Scene" tab and find the "Categories" panel. Click on it to open it. The tutorial will advance once you do. If you cannot find the panel, look along the edges of the window. Panels can be rearranged, so its exact position depends on your layout.

Once you have it open, you will see a list which is currently empty. That is where your category files will appear.]],
        },
        {
            title       = "Attach Ship to the Ship Entity",
            force_state = "scene_tab",
            body        = [[Now we tell the engine that the entity you created earlier is a ship.
1. Click the ship entity in the "Entity List" to select it.
2. In the "Categories" panel, click the Ship category.
3. Click "Assign to Entity".

Look at "Entity Properties". The Ship fields (speed, rotSpeed, shootTimer, shootInterval)
should now appear next to the Transform. The entity has both categories attached. Take a
moment to look at the values. These are the defaults you wrote in Ship.lua. You can edit
any of them right here, without touching the script.

Click Next.]],
        },
        {
            title       = "Create the Spawner Entity",
            force_state = "scene_tab",
            body        = [[The spawner rule needs an entity to live on. Let's create one.
1. Right-click the viewport and choose "Create Entity".
2. Select the new entity in the "Entity List".
3. Double-click its name and rename it to Spawner.
4. In "Entity Properties", set its Scale to (0, 0). The entity becomes invisible and unable to collide.
5. In the "Categories" panel, click Spawner and "Assign to Entity".

Why an invisible entity? The AsteroidSpawner rule only runs on entities that
have the Spawner category, but those entities do not need to be visible. By giving 
it zero scale we get a hidden container whose only job is to carry the spawner data
and schedule new asteroids.

Click Next.]],
        },
        {
            title       = "Load the Rules",
            force_state = "scene_tab",
            event       = "click_panel",
            highlight   = "Rules",
            body        = [[Find the "Rules" panel and click it to open. The tutorial
advances once you do. The Rules panel works just like the Categories panel
where you load the script files, and the engine starts applying each rule to
every entity with matching categories on every frame.]],
        },
        {
            title       = "What the Rules do",
            force_state = "scene_tab",
            body        = [[A quick reminder of what each one does:
- ShipControl reads input and drives the ship.
- BulletLogic moves bullets, expires them, hits asteroids.
- AsteroidLogic moves asteroids and detects ship collisions.
- AsteroidSpawner creates new asteroids over time.]],
        },
        {
            title       = "Play Your Game",
            force_state = "scene_tab",
            body        = [[Everything is in place. Press the "Play" button in the toolbar,
or use "Ctrl+P". Controls:

- "A" / "D" rotate left / right
- "W" thrust forward
- "Space" fire

Asteroids will start appearing after a couple of seconds and drift toward
the middle of the screen. Shoot them before they reach you. If one hits the ship,
you respawn in the centre and the asteroid disappears. If something looks wrong:
1. Press "Stop" to leave play mode.
2. Click the "Console" tab. Lua errors appear there with the filename and line number where the problem was found.
3. Switch back to the "Text Editor", fix the issue, save, and press "Play" again.

This cycle - run, read the error, fix, repeat - is called debugging.
Every programmer does it, including the ones who have been at it for decades.
It is not a sign that you are doing it wrong; it is just how programs are written.

Click Next when you are done playing.]],
        },
        {
            title       = "Experiment, Then a Recap",
            force_state = "scene_tab",
            body        = [[Before we close, change some numbers and see what happens.
Tweaking values until a game feels right is one of the most important
skills in game development - professional teams spend weeks on it.

Some experiments to try:
- Select the ship. Change Ship.speed from 5.0 to 15.0. Play.
- Change Ship.rotSpeed from 200.0 to 50.0. Easier or harder?
- In the Spawner entity, drop spawnInterval to 0.5 for chaos.
- Bump maxAsteroids from 12 to 30.
- In Categories/Ship.lua, change shootInterval to 0.05. Machine gun mode.

You do not have to reload anything after editing a script - just save, press "Stop", and press "Play" again.

Concepts you used in this tutorial:
- Variables and tables, the building blocks of data.
- Functions, including the Update function the engine calls on every frame.
- Conditions (if / else) that decide what to do.
- Loops (for) that step through every asteroid.
- Delta time, keeping movement frame-rate independent.
- Entities, categories, and rules — this engine's ECR model.

Where to go next:
- Add a score by creating a GameState category and updating it when a bullet destroys an asteroid.
- Split a large asteroid into two smaller ones when hit.
- Give the ship three lives and a game-over state.
- Make asteroids gradually speed up to ramp up difficulty.
- Give the ship momentum instead of instant stop-and-go movement.

You built a game. Pick one idea above and build a little more.]],
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
