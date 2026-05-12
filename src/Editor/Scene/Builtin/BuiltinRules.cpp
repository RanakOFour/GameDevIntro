#include "Editor/Scene/BuiltIn/BuiltinRules.h"

#include "RanakEngine/Asset/LuaFile.h"

#include <filesystem>
#include <fstream>
#include <cstdio>

const std::string k_editorRenderSrc =
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
                    UI.DrawCircleOutline(Vector2(cx, cy), r, Vector4(1.0, 0.2, 0.2, 1.0), 0.5)
                    drawn = true
                elseif phys.shape == "capsule" then
                    UI.DrawCapsuleOutline(Vector2(x, y), Vector2(w, h), Vector4(1.0, 0.2, 0.2, 1.0), 0.5)
                    drawn = true
                end
            end
            if not drawn then
                UI.DrawRectOutline(Vector2(x, y), Vector2(w, h), Vector4(1.0, 0.2, 0.2, 1.0), 0.5)
            end
        end
    end
end

return EditorRenderer
)lua";

const std::string k_physicsSyncSrc =
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

const std::string k_uiPanelRenderingSrc =
R"lua(local UIPanelRendering = Rule {
    categories = { "UIPanel" },
    fields = {}
}

function UIPanelRendering:Draw(_entityData)
    local panel = _entityData["UIPanel"]
    if panel ~= nil and panel.visible then
        local x = panel.position.x + panel.anchor.x * UI.GetScreenWidth()
        local y = panel.position.y + panel.anchor.y * UI.GetScreenHeight()
        UI.DrawRect(Vector2(x, y), Vector2(panel.width, panel.height), panel.colour)
    end
end

return UIPanelRendering
)lua";

const std::string k_uiButtonRenderingSrc =
R"lua(local UIButtonRendering = Rule {
    categories = { "UIButton" },
    fields = {}
}

function UIButtonRendering:Update(_entityData)
    local btn = _entityData["UIButton"]
    if btn ~= nil and btn.visible then
        local x = (((btn.position.x + btn.anchor.x) * 0.5) + 1) * UI.GetScreenWidth()
        local y = (((btn.position.y + btn.anchor.y) * 0.5) + 1) * UI.GetScreenHeight()
        btn.hovered = UI.IsHovered(Vector2(x, y), Vector2(btn.width, btn.height))
        btn.pressed = UI.IsClicked(Vector2(x, y), Vector2(btn.width, btn.height))
    end
end

function UIButtonRendering:Draw(_entityData)
    local btn = _entityData["UIButton"]
    if btn ~= nil and btn.visible then
        local btnPosition = btn.position + btn.anchor
        local btnSize = Vector2(btn.width, btn.height)

        local fill
        if btn.hovered then
            fill = btn.hover
        else
            fill = btn.colour
        end

        Log.Message("Drawing rect at " .. btnPosition:ToString() .. " with size " .. btnSize:ToString())
        UI.DrawRect(btnPosition, btnSize, fill)

        -- Convert button-centre pixel coords (Y-down) to NDC for DrawText.
        UI.DrawText(btnPosition, Vector4(1.0, 1.0, 1.0, 1.0), btn.label, 16.0, true)
    end
end

return UIButtonRendering
)lua";

const std::string k_uiTextRenderingSrc =
R"lua(local UITextRendering = Rule {
    categories = { "UIText" },
    fields = {}
}

function UITextRendering:Draw(_entityData)
    local txt = _entityData["UIText"]
    if txt ~= nil and txt.visible then
        -- txt.position is NDC: (-1,-1) bottom-left, (1,1) top-right.
        -- DrawText accepts NDC directly; do not centre — position IS the anchor.
        UI.DrawText(Vector2(txt.position.x, txt.position.y),
                    txt.colour, txt.text, txt.fontSize, true)
    end
end

return UITextRendering
)lua";


const std::vector<BuiltinRules::Entry>& BuiltinRules::GetEntries()
{
    static const std::vector<Entry> l_entries = {
        { "EditorRenderer",     k_editorRenderSrc       },
        { "DefaultRenderer",    k_renderingSrc          },
        { "PhysicsSync",        k_physicsSyncSrc        },
        { "UIPanelRendering",   k_uiPanelRenderingSrc   },
        { "UIButtonRendering",  k_uiButtonRenderingSrc  },
        { "UITextRendering",    k_uiTextRenderingSrc    },
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
    if (!l_scene)
    {
        printf("BuiltinRules::Load: ABORT — no active scene to add rules to.\n");
        return;
    }
    printf("BuiltinRules::Load: target scene=%p\n", (void*)l_scene.get());

    for (const auto& entry : GetEntries())
    {
        std::filesystem::path l_path = l_dir / (entry.name + ".lua");

        // Always overwrite the cached file so edits to the built-in source
        // strings in this .cpp take effect immediately on the next run.
        std::ofstream l_fileWriter(l_path, std::ios::trunc);
        l_fileWriter << entry.source;
        l_fileWriter.close();

        auto l_file = _contents.resources->Load<RE::Asset::LuaFile>(l_path.string());
        RE::Core::Rule l_rule = _contents.core->GetLuaContext()->CreateRule(l_file);
        l_scene->AddRule(l_rule);

        auto l_added = l_scene->GetRule(entry.name);
        unsigned long l_catBits = l_added ? l_added->GetCategories().count() : 0ul;
        printf("BuiltinRules::Load: added rule '%s' catBits=%lu (file %s)\n",
               entry.name.c_str(), l_catBits, l_path.string().c_str());

        if(entry.name == "DefaultRenderer")
        {
            // DefaultRenderer must be added to the scene before EditorRender so that it gets drawn first.
            // This ensures EditorRender's outline is visible on top of the default rendering.
            auto l_defRendererPtr = l_scene->GetRule("DefaultRenderer");
            if (l_defRendererPtr) l_defRendererPtr->SetActive(false);
        }
    }
}
