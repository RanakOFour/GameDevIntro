#include "RanakEngine/RanakEngine.h"

int main()
{
    // Start engine
    auto l_contents = RE::Initialise(true, Vector2(1280, 720));

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

    l_scene->AddEntityToCategory(l_testEntity, l_category.lock()->GetSignature());

    l_categoryFile = l_contents.resources->Load<RE::Asset::LuaFile>("./resources/Categories/Drawable.lua");
    l_context = RE::Core::LuaContext::Instance().lock();
    l_category = l_context->CreateCategory(l_categoryFile);

    RE::Log::Message("Adding entity to category");

    // Add new entity to category
    l_scene->AddEntityToCategory(l_testEntity, l_category.lock()->GetSignature());

    sol::table l_entityData = l_scene->GetRegistry()->GetEntityAttributes(l_testEntity);
    l_entityData.raw_get<sol::table>("Drawable").raw_set("texturePath", "./resources/textures/triangle.png");

    // Load new rule
    RE::Log::Message("Creating new rule");

    auto l_ruleFile = l_contents.resources->Load<RE::Asset::LuaFile>("./resources/Rules/Rendering.lua");
    RE::Core::Rule l_newRule = l_context->RunScript<RE::Core::Rule>(l_ruleFile);

    // Add rule to scene
    RE::Log::Message("Adding rule to scene");

    l_scene->AddRule(l_newRule);

    // Run scene
    RE::Log::Message("Starting core");
    l_contents.core->Start();

    RE::Shutdown(l_contents);
    return 0;
}