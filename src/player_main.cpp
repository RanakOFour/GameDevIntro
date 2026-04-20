/**
 * @file player_main.cpp
 * @brief Standalone player entry point.
 *
 * Loads a project and starts a scene without any editor UI.  Expects
 * a configuration file called PlayerConfig.json in the working directory,
 * written by the editor's Export feature.  The config specifies the scene
 * path and window dimensions.
 *
 * Build target: GamePlayer (defined in the root CMakeLists.txt).
 */

#include "RanakEngine/RanakEngine.h"
#include "RanakEngine/Asset/LuaFile.h"
#include "RanakEngine/Log.h"

#include "json/json.hpp"

#include "SDL3/SDL.h"
#include <GL/glew.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using json = nlohmann::json;

struct PlayerSceneSettings
{
    float gravityX    =  0.0f;
    float gravityY    = -9.8f;
    float clearColorR = 0.1f;
    float clearColorG = 0.1f;
    float clearColorB = 0.15f;
    float clearColorA = 1.0f;
};

static PlayerSceneSettings g_sceneSettings;

static void RegisterPlayerHelpers(RE::EngineContents& _contents,
                                  PlayerSceneSettings* _settings)
{
    sol::state* l_state = _contents.core->GetLuaContext()->GetState();

    l_state->set_function("LoadCategory",
        [&_contents](const std::string& _name, const std::string& _code)
        {
            auto l_ctx = _contents.core->GetLuaContext();
            if (!l_ctx->GetCategory(_name).lock())
                l_ctx->CreateCategory(_code);
        });

    l_state->set_function("LoadRule",
        [&_contents](const std::string& _name, const std::string& _code)
        {
            auto l_scene = _contents.core->GetScene().lock();
            if (!l_scene) return;

            // Write the rule source to a temp file so LuaContext can load it.
            std::filesystem::path l_tmpDir = std::filesystem::temp_directory_path() / "GamePlayer" / "Rules";
            std::filesystem::create_directories(l_tmpDir);
            std::filesystem::path l_tmpFile = l_tmpDir / (_name + ".lua");
            {
                std::ofstream l_out(l_tmpFile, std::ios::trunc);
                l_out << _code;
            }
            auto l_file = _contents.resources->Load<RE::Asset::LuaFile>(l_tmpFile.string());
            RE::Core::Rule l_rule = _contents.core->GetLuaContext()->CreateRule(l_file);
            l_scene->AddRule(l_rule);
        });

    l_state->set_function("SetSceneSettings",
        [_settings](float gx, float gy, float cr, float cg, float cb, float ca)
        {
            if (_settings)
            {
                _settings->gravityX    = gx;
                _settings->gravityY    = gy;
                _settings->clearColorR = cr;
                _settings->clearColorG = cg;
                _settings->clearColorB = cb;
                _settings->clearColorA = ca;
            }
        });

    l_state->set_function("CreateEntity",
        [&_contents](const std::string& _name, sol::table _categories) -> int
        {
            auto l_scene = _contents.core->GetScene().lock();
            if (!l_scene) return -1;
            int l_id = l_scene->AddEntity();
            l_scene->RenameEntity(l_id, _name);
            for (auto& l_pair : _categories)
            {
                std::string l_catName = l_pair.first.as<std::string>();
                sol::table  l_fields  = l_pair.second.as<sol::table>();
                auto l_cat = _contents.core->GetLuaContext()->GetCategory(l_catName).lock();
                if (l_cat)
                {
                    l_scene->AddToCategory(l_id, l_catName);
                    sol::table l_entityCat = l_scene->GetRegistry()
                        .GetEntityAttributes(l_id)
                        .raw_get<sol::table>(l_catName);
                    for (auto& l_fieldPair : l_fields)
                    {
                        l_entityCat.raw_set(l_fieldPair.first, l_fieldPair.second);
                    }
                }
            }
            return l_id;
        });
}

struct BuiltinEntry { std::string name; std::string source; };

static const std::vector<BuiltinEntry>& GetBuiltinCategories()
{
    static const std::vector<BuiltinEntry> l_entries = {
        { "Transform",
          "return Category {\n"
          "    Position = Vector2(0.0),\n"
          "    Layer    = 0,\n"
          "    Rotation = 0.0,\n"
          "    Scale    = Vector2(1.0)\n"
          "}\n" },
        { "Model",
          "return Category {\n"
          "    modelPath = \"\",\n"
          "    asset     = Field(nil, { hidden = true })\n"
          "}\n" },
        { "Texture",
          "return Category {\n"
          "    texturePath = \"\",\n"
          "    asset       = Field(nil, { hidden = true })\n"
          "}\n" },
        { "Shader",
          "return Category {\n"
          "    vertexshaderPath   = \"\",\n"
          "    fragmentshaderPath = \"\",\n"
          "    asset              = Field(nil, { hidden = true })\n"
          "}\n" },
        { "PhysicsBody",
          "return Category {\n"
          "    bodyType    = \"dynamic\",\n"
          "    density     = 1.0,\n"
          "    friction    = 0.3,\n"
          "    restitution = 0.1,\n"
          "    _body       = Field(nil, { hidden = true })\n"
          "}\n" },
        { "UIText",
          "return Category {\n"
          "    text      = \"\",\n"
          "    fontSize  = 16.0,\n"
          "    colorR    = 1.0,\n"
          "    colorG    = 1.0,\n"
          "    colorB    = 1.0,\n"
          "    colorA    = 1.0,\n"
          "    anchorX   = 0.0,\n"
          "    anchorY   = 0.0,\n"
          "    visible   = true\n"
          "}\n" },
        { "UIButton",
          "return Category {\n"
          "    label     = \"Button\",\n"
          "    width     = 120.0,\n"
          "    height    = 40.0,\n"
          "    colorR    = 0.2,\n"
          "    colorG    = 0.4,\n"
          "    colorB    = 0.7,\n"
          "    colorA    = 1.0,\n"
          "    hoverR    = 0.3,\n"
          "    hoverG    = 0.5,\n"
          "    hoverB    = 0.8,\n"
          "    hoverA    = 1.0,\n"
          "    anchorX   = 0.0,\n"
          "    anchorY   = 0.0,\n"
          "    pressed   = Field(false, { hidden = true }),\n"
          "    hovered   = Field(false, { hidden = true }),\n"
          "    visible   = true\n"
          "}\n" },
        { "UIPanel",
          "return Category {\n"
          "    width     = 200.0,\n"
          "    height    = 150.0,\n"
          "    colorR    = 0.12,\n"
          "    colorG    = 0.12,\n"
          "    colorB    = 0.16,\n"
          "    colorA    = 0.9,\n"
          "    anchorX   = 0.0,\n"
          "    anchorY   = 0.0,\n"
          "    visible   = true\n"
          "}\n" },
    };
    return l_entries;
}

static void LoadBuiltinCategories(RE::EngineContents& _contents)
{
    std::filesystem::path l_tmpDir = std::filesystem::temp_directory_path() / "GamePlayer" / "Categories";
    std::filesystem::create_directories(l_tmpDir);

    for (const auto& entry : GetBuiltinCategories())
    {
        std::filesystem::path l_path = l_tmpDir / (entry.name + ".lua");
        {
            std::ofstream l_out(l_path, std::ios::trunc);
            l_out << entry.source;
        }
        auto l_file = _contents.resources->Load<RE::Asset::LuaFile>(l_path.string());
        _contents.core->GetLuaContext()->CreateCategory(l_file);
    }
}

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

static const std::string k_uiRenderingSrc =
R"lua(local UIRendering = Rule {
    categories = { "Transform" },
    fields = {}
}

-- Converts a world-space Vector2 to UI pixel coordinates (Y-down).
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
    local py      = (1.0 - ndcY) * 0.5 * screenH
    return px, py
end

function UIRendering:Update(_entityData)
    local t = _entityData["Transform"]

    -- Convert world position to UI screen-space (Y-down pixels).
    local baseX, baseY = WorldToUI(t.Position)

    local btn = _entityData["UIButton"]
    if btn ~= nil and btn.visible then
        local x = baseX + btn.anchorX * UI.GetScreenWidth()
        local y = baseY + btn.anchorY * UI.GetScreenHeight()
        btn.hovered = UI.IsHovered(x, y, btn.width, btn.height)
        btn.pressed = UI.IsClicked(x, y, btn.width, btn.height)
    end
end

function UIRendering:Draw(_entityData)
    local t = _entityData["Transform"]

    -- Convert world position to UI screen-space (Y-down pixels).
    local baseX, baseY = WorldToUI(t.Position)

    local panel = _entityData["UIPanel"]
    if panel ~= nil and panel.visible then
        local x = baseX + panel.anchorX * UI.GetScreenWidth()
        local y = baseY + panel.anchorY * UI.GetScreenHeight()
        UI.DrawRect(x, y, panel.width, panel.height,
                    panel.colorR, panel.colorG, panel.colorB, panel.colorA)
    end

    local btn = _entityData["UIButton"]
    if btn ~= nil and btn.visible then
        local x = baseX + btn.anchorX * UI.GetScreenWidth()
        local y = baseY + btn.anchorY * UI.GetScreenHeight()
        local r, g, b, a
        if btn.hovered then
            r, g, b, a = btn.hoverR, btn.hoverG, btn.hoverB, btn.hoverA
        else
            r, g, b, a = btn.colorR, btn.colorG, btn.colorB, btn.colorA
        end
        UI.DrawRect(x, y, btn.width, btn.height, r, g, b, a)
        UI.DrawText(x + btn.width * 0.5, y + btn.height * 0.5,
                    1.0, 1.0, 1.0, 1.0,
                    btn.label, 16.0, true)
    end

    local img = _entityData["UIImage"]
    if img ~= nil and img.visible then
        local x = baseX + img.anchorX * UI.GetScreenWidth()
        local y = baseY + img.anchorY * UI.GetScreenHeight()
        if img.asset ~= nil then
            UI.DrawImage(img.asset:GetID(), x, y, img.width, img.height,
                         img.tintR, img.tintG, img.tintB, img.tintA)
        end
    end

    local txt = _entityData["UIText"]
    if txt ~= nil and txt.visible then
        local x = baseX + txt.anchorX * UI.GetScreenWidth()
        local y = baseY + txt.anchorY * UI.GetScreenHeight()
        UI.DrawText(x, y,
                    txt.colorR, txt.colorG, txt.colorB, txt.colorA,
                    txt.text, txt.fontSize, false)
    end
end

return UIRendering
)lua";

static void LoadPlayerRules(RE::EngineContents& _contents)
{
    std::filesystem::path l_tmpDir = std::filesystem::temp_directory_path() / "GamePlayer" / "Rules";
    std::filesystem::create_directories(l_tmpDir);

    struct RuleEntry { std::string name; const std::string& source; };
    std::vector<RuleEntry> l_rules = {
        { "Rendering",    k_renderingSrc },
        { "PhysicsSync",  k_physicsSyncSrc },
        { "UIRendering",  k_uiRenderingSrc },
    };

    auto l_scene = _contents.core->GetScene().lock();
    if (!l_scene) return;

    for (const auto& entry : l_rules)
    {
        std::filesystem::path l_path = l_tmpDir / (entry.name + ".lua");
        {
            std::ofstream l_out(l_path, std::ios::trunc);
            l_out << entry.source;
        }
        auto l_file = _contents.resources->Load<RE::Asset::LuaFile>(l_path.string());
        RE::Core::Rule l_rule = _contents.core->GetLuaContext()->CreateRule(l_file);
        l_scene->AddRule(l_rule);
    }
}

int main(int argc, char* argv[])
{
    try
    {
        // Read config
        std::string l_configPath = "PlayerConfig.json";
        if (argc > 1)
            l_configPath = argv[1];

        if (!std::filesystem::is_regular_file(l_configPath))
        {
            printf("GamePlayer: Config file not found: %s\n", l_configPath.c_str());
            return 1;
        }

        json l_config;
        {
            std::ifstream l_f(l_configPath);
            l_f >> l_config;
        }

        std::string l_scenePath = l_config.value("scene", "");
        int l_width  = l_config.value("windowWidth",  1280);
        int l_height = l_config.value("windowHeight", 720);
        std::string l_title = l_config.value("title", "Game");

        if (l_scenePath.empty())
        {
            printf("GamePlayer: No scene specified in config.\n");
            return 1;
        }

        if (!std::filesystem::is_regular_file(l_scenePath))
        {
            printf("GamePlayer: Scene file not found: %s\n", l_scenePath.c_str());
            return 1;
        }

        // Init engine
        RE::EngineContents l_engine = RE::Initialise(false, Vector2((float)l_width, (float)l_height));

        // Set window title
        auto l_window = l_engine.io->GetWindow().lock();
        SDL_SetWindowTitle(l_window->GetSDLWindow(), l_title.c_str());

        // Load built-in categories
        LoadBuiltinCategories(l_engine);

        // Load scene
        {
            std::ifstream l_f(l_scenePath);
            std::string l_script((std::istreambuf_iterator<char>(l_f)),
                                  std::istreambuf_iterator<char>());
            l_f.close();

            RegisterPlayerHelpers(l_engine, &g_sceneSettings);

            auto l_scene = std::make_shared<RE::Core::Scene>();
            l_engine.core->SetScene(l_scene);

            sol::state* l_state = l_engine.core->GetLuaContext()->GetState();
            l_state->set("Scene", l_scene);

            auto l_result = l_state->safe_script(l_script, sol::script_pass_on_error);
            if (!l_result.valid())
            {
                sol::error l_err = l_result;
                printf("GamePlayer: Scene load error:\n%s\n", l_err.what());
                RE::Shutdown(l_engine);
                return 1;
            }

            l_state->set("Scene", sol::nil);
        }

        // Add player rules (Rendering + PhysicsSync)
        LoadPlayerRules(l_engine);

        // Apply gravity
        l_engine.physics->SetGravity(Vector2(g_sceneSettings.gravityX, g_sceneSettings.gravityY));

        // Init all rules
        auto l_scene = l_engine.core->GetScene().lock();
        l_scene->Init();

        // Main loop
        const float l_targetFrame = 1.0f / 60.0f;
        const Uint64 l_perfFreq = SDL_GetPerformanceFrequency();
        Uint64 l_frameStart = SDL_GetPerformanceCounter();

        while (!l_engine.io->GetQuitSignal())
        {
            // Input
            std::vector<SDL_Event> l_events = l_engine.io->UpdateInputs();

            // Timing
            Uint64 l_now = SDL_GetPerformanceCounter();
            float l_dt = (l_now - l_frameStart) / static_cast<float>(l_perfFreq);

            // Update
            l_scene->Update(l_dt);

            // Render
            glClearColor(g_sceneSettings.clearColorR,
                         g_sceneSettings.clearColorG,
                         g_sceneSettings.clearColorB,
                         g_sceneSettings.clearColorA);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (auto l_ui = RE::UI::GetRenderer().lock())
            {
                l_ui->BeginFrame(static_cast<float>(l_width), static_cast<float>(l_height));
                l_scene->Draw();
                l_ui->EndFrame();
            }

            l_window->Swap();

            // Frame timing
            float l_elapsed = (SDL_GetPerformanceCounter() - l_frameStart) / static_cast<float>(l_perfFreq);
            float l_remaining = l_targetFrame - l_elapsed;
            if (l_remaining > 0.001f)
                SDL_Delay((Uint32)((l_remaining - 0.001f) * 1000.0f));
            while ((SDL_GetPerformanceCounter() - l_frameStart) / static_cast<float>(l_perfFreq) < l_targetFrame)
            { /* spin */ }
            l_frameStart = SDL_GetPerformanceCounter();
        }

        // Cleanup
        RE::Shutdown(l_engine);
    }
    catch (std::exception& e)
    {
        printf("GamePlayer failed: %s\n", e.what());
        return 1;
    }

    return 0;
}
