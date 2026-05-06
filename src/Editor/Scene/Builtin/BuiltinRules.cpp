#include "Editor/Scene/BuiltIn/BuiltinRules.h"

#include "RanakEngine/Asset/LuaFile.h"

#include <filesystem>
#include <fstream>
#include <cstdio>

static const std::string k_editorRenderSrc =
R"lua(local EditorRenderer = Rule {
    categories = {"Transform"},
    fields = {
        Log.Message("EditorRenderer: temp path is " .. Editor.GetTempPath()),
        templateDrawable = {
            shader  = Asset.Shader(Editor.GetTempPath() .. "/Shaders/REDefaultFragShader.fs;" .. Editor.GetTempPath() .. "/Shaders/REDefaultVertShader.vs"),
            Log.Message("EditorRenderer: Attempting to load texture from " .. Editor.GetTempPath() .. "/Textures/REDefaultTexture.png"),
            texture = Asset.Texture(Editor.GetTempPath() .. "/Textures/REDefaultTexture.png"),
            model   = Asset.Model(Editor.GetTempPath() .. "/Models/REDefaultModel.obj")
        },
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

    if self.fields.currentInput and not self.fields.lastFrameInput then
        self.fields.drawOutline = not self.fields.drawOutline
    end
    self.fields.lastFrameInput = self.fields.currentInput

    if self.fields.drawOutline then
        local _entityLayer = _entityData["Transform"].Layer
        _entityData["Transform"].Layer = Core.Camera:getPosition().z - 0.5
        Core.Camera:Draw(_entityData["Transform"],
                         self.fields.templateDrawable.model,
                         self.fields.templateDrawable.texture,
                         self.fields.templateDrawable.shader)
        _entityData.Transform.Layer = _entityLayer
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
    local body = Physics.CreateBody(transform.Position, transform.Rotation, phys.bodyType)
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

-- Converts a world-space Vector2 to UI pixel coordinates (Y-down).
-- Uses orthographic camera parameters directly so the result is always correct
-- regardless of the internal camera projection matrix state.
local function WorldToUI(worldPos)
    local screenW = UI.GetScreenWidth()
    local screenH = UI.GetScreenHeight()
    if screenW <= 0 or screenH <= 0 then return 0, 0 end
    local camW    = Core.Camera:getCameraWidth()
    local camH    = camW / (screenW / screenH)
    local cam     = Core.Camera:getPosition()
    local ndcX    = (worldPos.x - cam.x) / (camW * 0.5)
    local ndcY    = (worldPos.y - cam.y) / (camH * 0.5)
    local px      = (ndcX + 1.0) * 0.5 * screenW
    local py      = (1.0 - ndcY) * 0.5 * screenH   -- Y-down: 0=top, screenH=bottom
    return px, py
end

function UIRendering:Update(_entityData)
    local t = _entityData["Transform"]

    -- Convert world position to UI screen-space (Y-down pixels).
    local baseX, baseY = WorldToUI(t.Position)

    -- UIButton interaction
    local btn = _entityData["UIButton"]
    if btn ~= nil and btn.visible then
        local x = baseX + btn.anchor.x * UI.GetScreenWidth()
        local y = baseY + btn.anchor.y * UI.GetScreenHeight()
        btn.hovered = UI.IsHovered(x, y, btn.width, btn.height)
        btn.pressed = UI.IsClicked(x, y, btn.width, btn.height)
    end
end

function UIRendering:Draw(_entityData)
    local t = _entityData["Transform"]

    -- Convert world position to UI screen-space (Y-down pixels).
    local baseX, baseY = WorldToUI(t.Position)

    -- UIPanel: filled rectangle
    local panel = _entityData["UIPanel"]
    if panel ~= nil and panel.visible then
        local x = baseX + panel.anchor.x * UI.GetScreenWidth()
        local y = baseY + panel.anchor.y * UI.GetScreenHeight()
        UI.DrawRect(x, y, panel.width, panel.height,
                    panel.colour.x, panel.colour.y, panel.colour.z, panel.colour.w)
    end

    -- UIButton: rect with hover highlight + centered label
    local btn = _entityData["UIButton"]
    if btn ~= nil and btn.visible then
        local x = baseX + btn.anchor.x * UI.GetScreenWidth()
        local y = baseY + btn.anchor.y * UI.GetScreenHeight()
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

    -- UIImage: textured quad
    local img = _entityData["UIImage"]
    if img ~= nil and img.visible then
        local x = baseX + img.anchor.x * UI.GetScreenWidth()
        local y = baseY + img.anchor.y * UI.GetScreenHeight()
        if img.asset ~= nil then
            UI.DrawImage(img.asset:GetID(), x, y, img.width, img.height,
                         img.tint.x, img.tint.y, img.tint.z, img.tint.w)
        end
    end

    -- UIText: text rendering
    local txt = _entityData["UIText"]
    if txt ~= nil and txt.visible then
        local x = baseX + txt.anchor.x * UI.GetScreenWidth()
        local y = baseY + txt.anchor.y * UI.GetScreenHeight()
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

        if (!std::filesystem::exists(l_path))
        {
            std::ofstream l_fileWriter(l_path);
            l_fileWriter << entry.source;
            l_fileWriter.close();
            printf("BuiltinRules: Created %s\n", l_path.string().c_str());
        }

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
