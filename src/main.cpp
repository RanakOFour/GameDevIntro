#include "RanakEngine/RanakEngine.h"

int main()
{
    auto l_contents = RE::Initialise(true, Vector2(1280, 720));

    RE::Log::Message("Engine Initialised");

    auto l_scene = l_contents.core->GetScene().lock();
    int l_testEntity = l_scene->AddEntity();
    RE::Log::Message("Test entity created. Creating test category");

    auto l_categoryFile = l_contents.resources->Load<RE::Asset::LuaFile>("./resources/tests/CategoryCreation.lua");
    auto l_context = RE::Core::LuaContext::Instance().lock();
    auto l_category = l_context->CreateCategory(l_categoryFile);

    RE::Log::Message("Adding entity to category");

    l_scene->AddEntityToCategory(l_testEntity, l_category.lock()->GetSignature());

    RE::Log::Message("Creating new rule");

    auto l_ruleFile = l_contents.resources->Load<RE::Asset::LuaFile>("./resources/tests/RuleCreation.lua");
    RE::Core::Rule l_newRule = l_context->RunScript<RE::Core::Rule>(l_ruleFile);

    RE::Log::Message("Adding rule to scene");

    l_scene->AddRule(l_newRule);

    RE::Log::Message("Starting core");

    l_contents.core->Start();

    RE::Shutdown(l_contents);
    return 0;
}