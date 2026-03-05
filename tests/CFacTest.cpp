#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Resources.h"

namespace REC = RanakEngine::Core;
namespace RER = RanakEngine::Resources;



class CFacDebug
{
    public:
    REC::CategoryFactory factory;
    std::shared_ptr<RER::Manager> manager;
    std::weak_ptr<RER::LuaFile> file;
    REC::Category* category;

    CFacDebug()
    : factory()
    {
        manager = RER::Manager::Init();
        file = manager->Load<RER::LuaFile>("./resources/tests/CFacTest.lua");
        category = factory.Load(file).lock().get();
    };

    void PrintBaseAttributes()
    {
        sol::table& l_table = category->GetBaseData();
        auto l_categoryPairs = l_table.pairs();

        // Iterate key-value pairs of the table
        for(auto l_pair : l_categoryPairs)
        {
            sol::object l_obj = l_pair.first;
            std::cout << "Entry: ";
            switch(l_obj.get_type())
            {
                case sol::type::string:
                std::cout << l_obj.as<std::string>();
                break;

                case sol::type::boolean:
                std::cout << l_obj.as<bool>() ? "True" : "False";
                break;

                case sol::type::number:
                std::cout << l_obj.as<float>();
                break;

                case sol::type::nil:
                std::cout << "Nil";
                break;

                case sol::type::function:
                std::cout << "Function";
                break;

                case sol::type::table:
                std::cout << "Table";
                break;

                default:
                std::cout << "Type is not known";
                break;
            }

            l_obj = l_pair.second;
            std::cout << " Value: ";
            switch(l_obj.get_type())
            {
                case sol::type::string:
                std::cout << l_obj.as<std::string>();
                break;

                case sol::type::boolean:
                std::cout << l_obj.as<bool>() ? "True" : "False";
                break;

                case sol::type::number:
                std::cout << l_obj.as<float>();
                break;
                
                case sol::type::nil:
                std::cout << "Nil";
                break;

                case sol::type::function:
                std::cout << "Function";
                break;

                case sol::type::table:
                std::cout << "Table";
                break;

                default:
                std::cout << "Type is not known";
                break;
            }

            std::cout << std::endl;
        }
    };
};

int main()
{
    CFacDebug l_debug;
    l_debug.PrintBaseAttributes();

    return 1;
}