#include "Editor/Scene/BuiltIn/BuiltinRules.h"

#include "RanakEngine/Asset/LuaFile.h"

#include <filesystem>
#include <fstream>
#include <cstdio>

static const std::string k_editorRenderSrc =
R"lua(local EditorRenderer = Rule {
    categories = {"Transform"},
    fields = {
        drawOutline    = true,
        lastFrameInput = false,
        currentInput   = false
    }
}

function EditorRenderer:Update(_entityData)
    local currentFrameInput = IO.GetKeyDown('o')
    if currentFrameInput ~= nil then
        self.fields.currentInput = currentFrameInput
    end
end

function EditorRenderer:Draw(_entityData)

    if(_entityData.Texture ~= nil or _entityData.model ~= nil) then
        Core.Camera:Draw(_entityData)
    end

    -- Toggle outline with 'o' key
    if self.fields.currentInput and not self.fields.lastFrameInput then
        self.fields.drawOutline = not self.fields.drawOutline
    end
    self.fields.lastFrameInput = self.fields.currentInput

    if self.fields.drawOutline then
        local tf = _entityData["Transform"]
        if tf ~= nil then
            local worldPos = tf.Position
            local scale    = tf.Scale

            local topLeft  = Core.Camera:WorldToScreenPoint(worldPos - scale)
            local botRight = Core.Camera:WorldToScreenPoint(worldPos + scale)

            -- Camera returns Y-up (0 = bottom, screenH = top).
            -- UI functions expect Y-down (0 = top, screenH = bottom).
            -- Compute AABB in Y-down: top edge = screenH - max(y1,y2), left = min(x1,x2).
            local screenH = UI.GetScreenHeight()
            local x = Math.Min(topLeft.x, botRight.x)
            local y = screenH - Math.Max(topLeft.y, botRight.y)
            local w = Math.Abs(botRight.x - topLeft.x)
            local h = Math.Abs(botRight.y - topLeft.y)

            local phys = _entityData["PhysicsBody"]
            local drawn = false
            if phys ~= nil then
                if phys.shape == "circle" then
                    local cx = x + w * 0.5
                    local cy = y + h * 0.5
                    local r  = Math.Max(w, h) * 0.5
                    UI.DrawCircleOutline(cx, cy, r, 1.0, 0.2, 0.2, 1.0, 0.5)
                    drawn = true
                elseif phys.shape == "capsule" then
                    UI.DrawCapsuleOutline(x, y, w, h, 1.0, 0.2, 0.2, 1.0, 0.5)
                    drawn = true
                end
            end
            if not drawn then
                UI.DrawRectOutline(x, y, w, h, 1.0, 0.2, 0.2, 1.0, 0.5)
            end
        end
    end
end

return EditorRenderer
)lua";

static const std::string k_physicsSyncSrc =
R"lua(local PhysicsSync = Rule {
    categories = { "Transform", "PhysicsBody" },
    fields = {}
}

function PhysicsSync:Init(_entityData)
    local transform = _entityData["Transform"]
    local phys      = _entityData["PhysicsBody"]

    local extras = {
        gravityScale    = phys.gravityScale,
        linearDamping   = phys.linearDamping,
        angularDamping  = phys.angularDamping,
        fixedRotation   = phys.fixedRotation,
        linearVelocity  = phys.linearVelocity,
        angularVelocity = phys.angularVelocity,
    }

    local body = Physics.CreateBody(transform.Position, transform.Rotation, phys.bodyType, extras)

    if phys.shape == "circle" then
        local radius = Math.Max(transform.Scale.x, transform.Scale.y)
        Physics.AddCircleShape(body, radius, phys.density, phys.friction, phys.restitution)
        phys._body = body
        Log.Message("PhysicsSync: created circle body at " .. transform.Position:ToString()
                    .. " radius " .. tostring(radius))
        return
    elseif phys.shape == "capsule" then
        local halfW = transform.Scale.x
        local halfH = transform.Scale.y
        -- Capsule: box body (main segment) + two circle end-caps
        local axisLen = halfH - halfW
        if axisLen < 0 then axisLen = 0 end
        Physics.AddBoxShape(body, halfW, axisLen, phys.density, phys.friction, phys.restitution)
        -- Note: proper capsule needs shape offsets; this is a simplified approximation.
        -- Box2D 3.0 doesn't have native capsule, so compose from box + circles.
    end

    -- square (default, also falls through from capsule)
    local halfW = transform.Scale.x
    local halfH = transform.Scale.y
    Physics.AddBoxShape(body, halfW, halfH, phys.density, phys.friction, phys.restitution)
    phys._body = body
    Log.Message("PhysicsSync: created body at " .. transform.Position:ToString()
                .. " half-extents (" .. tostring(halfW) .. ", " .. tostring(halfH) .. ")")
end

function PhysicsSync:Update(_entityData)
    local transform = _entityData["Transform"]
    local phys      = _entityData["PhysicsBody"]
    if phys._body ~= nil and phys._body:IsValid() then
        transform.Position = phys._body:GetPosition()
        transform.Rotation = Math.RadToDeg(phys._body:GetAngle())
    end
end

return PhysicsSync
)lua";

static const std::string k_renderingSrc =
R"lua(local DefaultRenderer = Rule {
    categories = { "Transform" },
    fields = {}
}

function DefaultRenderer:Update(_entityData)
end

function DefaultRenderer:Draw(_entityData)
    Core.Camera:Draw(_entityData)
end

return DefaultRenderer
)lua";

static const std::string k_uiRenderingSrc =
R"lua(local UIRendering = Rule {
    categories = { "Transform" },
    fields = {}
}

function UIRendering:Update(_entityData)
    -- UIButton interaction: position comes from the UIButton category
    -- directly as a screen-space coordinate (pixels, Y-down).
    local btn = _entityData["UIButton"]
    if btn ~= nil and btn.visible then
        local x = btn.position.x + btn.anchor.x * UI.GetScreenWidth()
        local y = btn.position.y + btn.anchor.y * UI.GetScreenHeight()
        btn.hovered = UI.IsHovered(x, y, btn.width, btn.height)
        btn.pressed = UI.IsClicked(x, y, btn.width, btn.height)
    end
end

function UIRendering:Draw(_entityData)
    -- UIPanel: filled rectangle in screen-space
    local panel = _entityData["UIPanel"]
    if panel ~= nil and panel.visible then
        local x = panel.position.x + panel.anchor.x * UI.GetScreenWidth()
        local y = panel.position.y + panel.anchor.y * UI.GetScreenHeight()
        UI.DrawRect(x, y, panel.width, panel.height,
                    panel.colour.x, panel.colour.y, panel.colour.z, panel.colour.w)
    end

    -- UIButton: rect with hover highlight + centered label
    local btn = _entityData["UIButton"]
    if btn ~= nil and btn.visible then
        local x = btn.position.x + btn.anchor.x * UI.GetScreenWidth()
        local y = btn.position.y + btn.anchor.y * UI.GetScreenHeight()
        local r, g, b, a
        if btn.hovered then
            r, g, b, a = btn.hover.x, btn.hover.y, btn.hover.z, btn.hover.w
        else
            r, g, b, a = btn.colour.x, btn.colour.y, btn.colour.z, btn.colour.w
        end
        UI.DrawRect(x, y, btn.width, btn.height, r, g, b, a)
        -- Label centered inside the button
        UI.DrawText(x + btn.width * 0.5, y + btn.height * 0.5,
                    1.0, 1.0, 1.0, 1.0,
                    btn.label, 16.0, true)
    end

    -- UIText: text rendering in screen-space
    local txt = _entityData["UIText"]
    if txt ~= nil and txt.visible then
        local x = txt.position.x + txt.anchor.x * UI.GetScreenWidth()
        local y = txt.position.y + txt.anchor.y * UI.GetScreenHeight()
        UI.DrawText(x, y,
                    txt.colour.x, txt.colour.y, txt.colour.z, txt.colour.w,
                    txt.text, txt.fontSize, false)
    end
end

return UIRendering
)lua";


const std::vector<BuiltinRules::Entry>& BuiltinRules::GetEntries()
{
    static const std::vector<Entry> l_entries = {
        { "EditorRenderer",  k_editorRenderSrc },
        { "DefaultRenderer", k_renderingSrc    },
        { "PhysicsSync",     k_physicsSyncSrc  },
        { "UIRendering",     k_uiRenderingSrc  },
    };
    
    return l_entries;
}

bool BuiltinRules::IsBuiltin(const std::string& _name)
{
    for (const auto& entry : GetEntries())
    {
        if (_name == entry.name)
            return true;
    }
    return false;
}

void BuiltinRules::Load(RE::EngineContents& _contents)
{
    std::filesystem::path l_dir(RanakEngine::Asset::GetTempDir() / "Rules");

    if (!std::filesystem::exists(l_dir))
    {
        std::filesystem::create_directories(l_dir);
    }

    auto l_scene = _contents.core->GetScene().lock();

    for (const auto& entry : GetEntries())
    {
        std::filesystem::path l_path = l_dir / (entry.name + ".lua");

        // Always overwrite the cached file so edits to the built-in source
        // strings in this .cpp take effect immediately on the next run.
        std::ofstream l_fileWriter(l_path, std::ios::trunc);
        l_fileWriter << entry.source;
        l_fileWriter.close();
        printf("BuiltinRules: Updated %s\n", l_path.string().c_str());

        auto l_file = _contents.resources->Load<RE::Asset::LuaFile>(l_path.string());
        RE::Core::Rule l_rule = _contents.core->GetLuaContext()->CreateRule(l_file);
        l_scene->AddRule(l_rule);

        if(entry.name == "DefaultRenderer")
        {
            // DefaultRenderer must be added to the scene before EditorRender so that it gets drawn first.
            // This ensures EditorRender's outline is visible on top of the default rendering.
            auto l_defRendererPtr = l_scene->GetRule("DefaultRenderer");
            l_defRendererPtr->SetActive(false);
        }
    }
}
