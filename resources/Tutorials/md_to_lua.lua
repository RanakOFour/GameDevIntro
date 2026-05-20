-- md_to_lua.lua
--
-- Reverse-converts an Asteroids-style tutorial markdown document back into
-- the Lua table format consumed by TutorialPanel.
--
-- Usage from another script:
--   local m = dofile("resources/Tutorials/md_to_lua.lua")
--   m.convert("Asteroids.md", "Asteroids.lua")
--
-- Usage from the shell:
--   lua resources/Tutorials/md_to_lua.lua Asteroids.md Asteroids.lua

local M = {}

local META_KEYS = { "force_state", "event", "wait_state", "highlight" }

-- Parse a metadata line of the form
--     _force_state: scene_tab · highlight: Entity List_
-- into a table { force_state = "scene_tab", highlight = "Entity List" }.
local function parseMeta(line)
    local inner = line:match("^_(.+)_$")
    if not inner then return nil end
    local out = {}
    -- Split on " · " (middle-dot used by lua_to_md.lua).
    for part in inner:gmatch("([^·]+)") do
        local k, v = part:match("^%s*([%w_]+):%s*(.-)%s*$")
        if k and v and v ~= "" then out[k] = v end
    end
    return out
end

-- Pick the shortest long-bracket level (e.g. "", "=", "==") that does not
-- collide with any "]<level>]" substring inside the body.
local function pickBracketLevel(body)
    local level = ""
    while body:find("%]" .. level .. "%]", 1, false) do
        level = level .. "="
    end
    return level
end

-- Parse a markdown document into { title = ..., steps = { {title, body, ...}, ... } }.
-- defaultTitle is used when the markdown has no top-level "# Title" line.
function M.parse(mdText, defaultTitle)
    local lines = {}
    for line in (mdText .. "\n"):gmatch("([^\n]*)\n") do
        lines[#lines+1] = line
    end

    local doc = { steps = {} }
    local i = 1

    -- Optional top-level "# Title" — but stop searching once we hit the first
    -- "## " step heading so we never accidentally consume a step as a title.
    for j = 1, #lines do
        if lines[j]:match("^##%s") then break end
        local t = lines[j]:match("^#%s+(.+)$")
        if t and not lines[j]:match("^##") then
            doc.title = t
            i = j + 1
            break
        end
    end
    if not doc.title then
        doc.title = defaultTitle or ""
    end

    -- Walk steps.
    while i <= #lines do
        local stepTitle = lines[i]:match("^##%s+%d+%.%s+(.+)$")
                       or lines[i]:match("^##%s+(.+)$")
        if stepTitle then
            local step = { title = stepTitle }
            i = i + 1

            -- Skip blank lines after the heading.
            while i <= #lines and lines[i] == "" do i = i + 1 end

            -- Optional metadata line.
            if i <= #lines then
                local meta = parseMeta(lines[i])
                if meta then
                    for k, v in pairs(meta) do step[k] = v end
                    i = i + 1
                    while i <= #lines and lines[i] == "" do i = i + 1 end
                end
            end

            -- Collect body lines until the next heading (or EOF). Fenced
            -- ```lua / ``` blocks are re-indented by two spaces and the
            -- fences are stripped, matching the original lua-source style.
            local bodyLines = {}
            local inCode = false
            while i <= #lines do
                local line = lines[i]
                if line:match("^##%s") or line:match("^#%s") then break end
                if not inCode and line:match("^```") then
                    inCode = true
                elseif inCode and line == "```" then
                    inCode = false
                else
                    if inCode then
                        if line == "" then
                            bodyLines[#bodyLines+1] = ""
                        else
                            bodyLines[#bodyLines+1] = "  " .. line
                        end
                    else
                        bodyLines[#bodyLines+1] = line
                    end
                end
                i = i + 1
            end

            -- Trim leading and trailing blank lines from the body.
            while #bodyLines > 0 and bodyLines[1] == "" do
                table.remove(bodyLines, 1)
            end
            while #bodyLines > 0 and bodyLines[#bodyLines] == "" do
                bodyLines[#bodyLines] = nil
            end

            step.body = table.concat(bodyLines, "\n")
            doc.steps[#doc.steps+1] = step
        else
            i = i + 1
        end
    end

    return doc
end

-- Render a parsed doc table back to Lua source matching the original style.
function M.render(doc)
    local out = {}
    local function add(s) out[#out+1] = s end

    add("return {")
    add(string.format('    title = "%s",', doc.title or ""))
    add("    steps = {")

    for _, step in ipairs(doc.steps or {}) do
        add("        {")
        add(string.format('            %-11s = "%s",', "title", step.title or ""))
        for _, k in ipairs(META_KEYS) do
            if step[k] and step[k] ~= "" then
                add(string.format('            %-11s = "%s",', k, step[k]))
            end
        end
        local body = step.body or ""
        local lvl  = pickBracketLevel(body)
        local open = "[" .. lvl .. "["
        local cls  = "]" .. lvl .. "]"
        add(string.format("            %-11s = %s%s%s,", "body", open, body, cls))
        add("        },")
    end

    add("")
    add("    }")
    add("}")
    add("")

    return table.concat(out, "\n")
end

-- Read mdPath, convert, write to luaPath. Returns the doc table.
-- opts.defaultTitle: title to use when the markdown has no "# Title" heading.
--                    Defaults to the markdown file's stem (e.g. "Asteroids").
function M.convert(mdPath, luaPath, opts)
    local f, err = io.open(mdPath, "r")
    if not f then error("md_to_lua: cannot read " .. mdPath .. ": " .. (err or "")) end
    local text = f:read("*a")
    f:close()

    opts = opts or {}
    local fallbackTitle = opts.defaultTitle
                       or mdPath:match("([^/\\]+)%.[Mm][Dd]$")
                       or mdPath
    local doc = M.parse(text, fallbackTitle)
    local lua = M.render(doc)

    local outF, outErr = io.open(luaPath, "w")
    if not outF then error("md_to_lua: cannot write " .. luaPath .. ": " .. (outErr or "")) end
    outF:write(lua)
    outF:close()

    return doc
end

-- CLI entry point: only runs when invoked as a script, not when required.
if arg and arg[0] and arg[0]:match("md_to_lua%.lua$") then
    if not arg[1] or not arg[2] then
        io.stderr:write("usage: lua md_to_lua.lua <input.md> <output.lua>\n")
        os.exit(1)
    end
    local doc = M.convert(arg[1], arg[2])
    io.write(string.format("Wrote %s: title=%q, steps=%d\n",
        arg[2], doc.title or "", #(doc.steps or {})))
end

return M
