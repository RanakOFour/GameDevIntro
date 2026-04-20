#include "Editor/BuiltinRules.h"

#include "RanakEngine/Asset/LuaFile.h"

#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <cstdio>

static const std::string k_editorRenderSrc =
R"lua(local EditorRender = Rule {
    categories = {"Transform"},
    fields = {
        templateDrawable = {
            shader  = Asset.Shader("./resources/Shaders/default/frag.fs;./resources/Shaders/default/vert.vs"),
            texture = Asset.Texture("./resources/Textures/EditorTexture.png"),
            model   = Asset.Model("./resources/Models/FlatTexture.obj")
        },
        drawOutline    = true,
        lastFrameInput = false,
        currentInput   = false
    }
}

function EditorRender:Update(_entityData)
    local currentFrameInput = IO.GetKeyDown('o')
    if currentFrameInput ~= nil then
        self.fields.currentInput = currentFrameInput
    end
end

function EditorRender:Draw(_entityData)

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

return EditorRender
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
R"lua(local Rendering = Rule {
    categories = { "Transform" },
    fields = {
        templateDrawable = {
            shader  = Asset.Shader("./resources/Shaders/default/frag.fs;./resources/Shaders/default/vert.vs"),
            model   = Asset.Model("./resources/Models/FlatTexture.obj")
        },
    }
}

function Rendering:Update(_entityData)
end

function Rendering:Draw(_entityData)
    Core.Camera:Draw(_entityData)
end

return Rendering
)lua";

static const std::string k_uiRenderingSrc =
R"lua(local UIRendering = Rule {
    categories = { "Transform" },
    fields = {}
}

function UIRendering:Update(_entityData)
    local t = _entityData["Transform"]

    -- UIButton interaction
    local btn = _entityData["UIButton"]
    if btn ~= nil and btn.visible then
        local x = t.Position.x + btn.anchorX * UI.GetScreenWidth()
        local y = t.Position.y + btn.anchorY * UI.GetScreenHeight()
        btn.hovered = UI.IsHovered(x, y, btn.width, btn.height)
        btn.pressed = UI.IsClicked(x, y, btn.width, btn.height)
    end
end

function UIRendering:Draw(_entityData)
    local t = _entityData["Transform"]

    -- UIPanel: filled rectangle
    local panel = _entityData["UIPanel"]
    if panel ~= nil and panel.visible then
        local x = t.Position.x + panel.anchorX * UI.GetScreenWidth()
        local y = t.Position.y + panel.anchorY * UI.GetScreenHeight()
        UI.DrawRect(x, y, panel.width, panel.height,
                    panel.colorR, panel.colorG, panel.colorB, panel.colorA)
    end

    -- UIButton: rect with hover highlight + centered label
    local btn = _entityData["UIButton"]
    if btn ~= nil and btn.visible then
        local x = t.Position.x + btn.anchorX * UI.GetScreenWidth()
        local y = t.Position.y + btn.anchorY * UI.GetScreenHeight()
        local r, g, b, a
        if btn.hovered then
            r, g, b, a = btn.hoverR, btn.hoverG, btn.hoverB, btn.hoverA
        else
            r, g, b, a = btn.colorR, btn.colorG, btn.colorB, btn.colorA
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
        local x = t.Position.x + img.anchorX * UI.GetScreenWidth()
        local y = t.Position.y + img.anchorY * UI.GetScreenHeight()
        if img.asset ~= nil then
            UI.DrawImage(img.asset:GetID(), x, y, img.width, img.height,
                         img.tintR, img.tintG, img.tintB, img.tintA)
        end
    end

    -- UIText: text rendering
    local txt = _entityData["UIText"]
    if txt ~= nil and txt.visible then
        local x = t.Position.x + txt.anchorX * UI.GetScreenWidth()
        local y = t.Position.y + txt.anchorY * UI.GetScreenHeight()
        UI.DrawText(x, y,
                    txt.colorR, txt.colorG, txt.colorB, txt.colorA,
                    txt.text, txt.fontSize, false)
    end
end

return UIRendering
)lua";


const std::vector<BuiltinRules::Entry>& BuiltinRules::GetEntries()
{
    static const std::vector<Entry> l_entries = {
        { "EditorRender", k_editorRenderSrc },
        { "PhysicsSync",  k_physicsSyncSrc  },
        { "UIRendering",  k_uiRenderingSrc  },
    };
    return l_entries;
}

std::string BuiltinRules::GetDataDir()
{
#if defined(_WIN32)
    const char* l_appDataRaw = std::getenv("APPDATA");
    const std::string l_appData = l_appDataRaw ? l_appDataRaw : "";
    std::filesystem::path l_base = l_appData.empty()
        ? std::filesystem::path(".")
        : std::filesystem::path(l_appData);
#else
    const char* l_xdgRaw  = std::getenv("XDG_DATA_HOME");
    const char* l_homeRaw = std::getenv("HOME");
    const std::string l_xdg  = l_xdgRaw  ? l_xdgRaw  : "";
    const std::string l_home = l_homeRaw ? l_homeRaw : "";
    std::filesystem::path l_base;
    if (!l_xdg.empty())
        l_base = std::filesystem::path(l_xdg);
    else if (!l_home.empty())
        l_base = std::filesystem::path(l_home) / ".local" / "share";
    else
        l_base = std::filesystem::path(".");
#endif
    return (l_base / "GameDevIntro" / "Rules").string();
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
    std::filesystem::path l_dir(GetDataDir());

    if (!std::filesystem::exists(l_dir))
        std::filesystem::create_directories(l_dir);

    for (const auto& entry : GetEntries())
    {
        std::filesystem::path l_path = l_dir / (entry.name + ".lua");

        if (!std::filesystem::exists(l_path))
        {
            std::ofstream l_out(l_path);
            if (!l_out)
            {
                printf("[BuiltinRules] WARNING: could not write %s\n",
                       l_path.string().c_str());
                continue;
            }
            l_out << entry.source;
            printf("BuiltinRules: Created %s\n", l_path.string().c_str());
        }

        auto l_file = _contents.resources->Load<RE::Asset::LuaFile>(l_path.string());
        RE::Core::Rule l_rule = _contents.core->GetLuaContext()->CreateRule(l_file);
        _contents.core->GetScene().lock()->AddRule(l_rule);
    }
}
