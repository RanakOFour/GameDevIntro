#include "RanakEngine/Resources/ResourceManager.h"

namespace RanakEngine::Resources
{
    std::shared_ptr<Resources::Manager> Resources::Manager::Init()
    {
        std::shared_ptr<Resources::Manager> l_toReturn;
        Resources::Manager* l_manager = new Resources::Manager();
        l_toReturn.reset(l_manager);

        l_toReturn->m_self = l_toReturn;

        return l_toReturn;
    }

    std::weak_ptr<Resources::Manager> Resources::Manager::Instance()
    {
        return m_self;
    }
}