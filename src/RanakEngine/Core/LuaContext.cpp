#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/Rule.h"

namespace RanakEngine::Core
{
    LuaContext::LuaContext()
    : m_state()
    , m_loadedScripts()
    {
        m_state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::table);
        m_categoryFactory = std::make_shared<CategoryFactory>();
        Category::DefineUsertype(m_state);
        Rule::DefineUsertype(m_state);
    }

    LuaContext::~LuaContext()
    {

    }

    std::shared_ptr<LuaContext> LuaContext::Init()
    {
        std::shared_ptr<LuaContext> l_toReturn = std::shared_ptr<LuaContext>();
        LuaContext* l_wrapper = new LuaContext();
        l_toReturn.reset(l_wrapper);
        l_toReturn->m_self = l_toReturn;

        return l_toReturn;
    };

    std::weak_ptr<LuaContext> LuaContext::Instance()
    {
        return m_self;
    }

    void LuaContext::LoadScript(std::weak_ptr<Resources::LuaFile> _file)
    {
        auto l_file = _file.lock();
        std::string l_path = l_file->GetPath();
        std::string l_code = l_file->GetCode();

        m_loadedScripts[l_path] = m_state.load_buffer(l_code.data(), l_code.size());
        if(!m_loadedScripts[l_path].valid())
        {
            sol::error l_err = m_loadedScripts[l_path];
            printf("Could not load script \"%s\"!\n%s\n", l_path.c_str(), l_err.what());
            m_loadedScripts.erase(l_path);
        }
    }

    sol::table LuaContext::CreateTable()
    {
        return m_state.create_table();
    }

    std::weak_ptr<Category> LuaContext::CreateCategory(std::weak_ptr<Resources::LuaFile> _file)
    {
        sol::table l_categoryTable = RunScript<sol::table>(_file);
        return m_categoryFactory->RegisterCategory(l_categoryTable);
    }

    std::weak_ptr<Category> LuaContext::GetCategory(std::bitset<1024> _signature)
    {
        return m_categoryFactory->GetBySignature(_signature);
    }
}