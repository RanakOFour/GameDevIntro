#ifndef CATEGORYFACTORY_H
#define CATEGORYFACTORY_H

#include <memory>
#include <map>
#include <bitset>
#include <vector>

#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Resources/LuaFile.h"

namespace RanakEngine::Core
{
    class CategoryFactory
    {
        private:
        std::map<std::bitset<1024>, std::shared_ptr<Category>> m_signatures;
        std::map<Resources::LuaFile, std::shared_ptr<Category>> m_categories;
        std::map<std::string, std::shared_ptr<Category>> m_byName;

        public:
        CategoryFactory();
        ~CategoryFactory();

        std::weak_ptr<Category> Load(std::weak_ptr<Resources::LuaFile> _file);
        std::weak_ptr<Category> GetByName(std::string _name);
        std::weak_ptr<Category> GetBySignature(std::bitset<1024> _signature);
        std::vector<std::weak_ptr<Category>> GetAllCategories(std::bitset<1024> _combinedSignature);
        void DeleteCategory(std::string _name);
    };
}

#endif