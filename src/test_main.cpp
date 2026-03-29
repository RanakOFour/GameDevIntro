#include "RanakEngine/RanakEngine.h"

int main()
{
    // Start engine
    auto l_contents = RE::Initialise(true, Vector2(1920, 1080));

    RE::Log::Message("Engine Initialised");

    // Get scene
    auto l_scene = l_contents.core->GetScene().lock();

    // Add entity to scene
    int l_testEntity = l_scene->AddEntity();
    RE::Log::Message("Test entity created. Creating test category");

    // Load category
    auto l_categoryFile = l_contents.resources->Load<RE::Asset::LuaFile>("./resources/Categories/Transform.lua");
    auto l_context = RE::Core::LuaContext::Instance().lock();
    auto l_category = l_context->CreateCategory(l_categoryFile);

    l_scene->AddToCategory(l_testEntity, l_category.lock()->GetName());

    l_categoryFile = l_contents.resources->Load<RE::Asset::LuaFile>("./resources/Categories/Drawable.lua");
    l_category = l_context->CreateCategory(l_categoryFile);

    RE::Log::Message("Adding entity to category");

    // Add new entity to category
    l_scene->AddToCategory(l_testEntity, l_category.lock()->GetName());

    sol::table l_entityData = l_scene->GetRegistry().GetEntityAttributes(l_testEntity);

    // Load new rule
    RE::Log::Message("Creating new rule");

    auto l_ruleFile = l_contents.resources->Load<RE::Asset::LuaFile>("./resources/Rules/Rendering.lua");
    RE::Core::Rule l_newRule = l_context->CreateRule(l_ruleFile);

    // Add rule to scene
    RE::Log::Message("Adding rule to scene");

    l_scene->AddRule(l_newRule);

    l_ruleFile = l_contents.resources->Load<RE::Asset::LuaFile>("./resources/Rules/RotateGuy.lua");
    RE::Core::Rule l_rule2 = l_context->CreateRule(l_ruleFile);
    
    // Add rule to scene
    RE::Log::Message("Adding rule to scene");

    l_scene->AddRule(l_rule2);

    l_ruleFile = l_contents.resources->Load<RE::Asset::LuaFile>("./resources/Rules/ControlCamera.lua");
    RE::Core::Rule l_camRule = l_context->CreateRule(l_ruleFile);
    
    // Add rule to scene
    RE::Log::Message("Adding rule to scene");

    l_scene->AddRule(l_camRule);

    // Run scene
    RE::Log::Message("Starting core");
    l_contents.core->Start();

    RE::Shutdown(l_contents);
    return 0;
}