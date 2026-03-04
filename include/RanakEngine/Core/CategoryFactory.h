#ifndef CATEGORYFACTORY_H
#define CATEGORYFACTORY_H

#include <memory>
#include <map>
#include <bitset>

#include "RanakEngine/Core/Category.h"


namespace RanakEngine::Core
{
    class CategoryFactory
    {
        private:
        std::map<std::shared_ptr<Category>, std::bitset<1024>> m_signatures;
        std::map

        public:

    };
}

#endif