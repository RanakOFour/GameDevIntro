
#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Core/LuaContext.h"

#include "RanakEngine/Asset/LuaFile.h"

#include "RanakEngine/Log.h"

namespace RanakEngine::Core
{
    CategoryFactory::CategoryFactory()
    : m_signatureToCategory()
    , m_nameToSignature()
    {
    }

    CategoryFactory::~CategoryFactory()
    {

    }

    std::weak_ptr<Category> CategoryFactory::RegisterCategory(Category _category)
    {
        std::string l_categoryName = _category.GetName();
        sol::table l_baseAttributes = _category.GetBaseData();

        auto l_newCategoryPtr = std::make_shared<Category>(l_categoryName, l_baseAttributes);

        std::bitset<1024> l_newSignature;
        l_newSignature.set(m_size);

        m_nameToSignature.insert({l_newCategoryPtr->m_name, l_newSignature});

        l_newCategoryPtr->m_signature = l_newSignature;

        m_signatureToCategory.insert({l_newSignature, l_newCategoryPtr});

        m_size++;

        return l_newCategoryPtr;
    }

    std::weak_ptr<Category> CategoryFactory::RegisterCategory(sol::table _definitionTable)
    {
        // Give category default name of Category{0}, where {0} is m_size, or the registered name is not null
        std::string l_categoryName = _definitionTable.raw_get<std::string>("name");
        if(l_categoryName == "")
        {
            l_categoryName = "Category" + std::to_string(m_size);
        }

        Log::Message("Registering category " + l_categoryName + "...\n");

        sol::table l_baseAttributes = _definitionTable.raw_get<sol::table>("baseAttributes");

        auto l_newCategoryPtr = std::make_shared<Category>(l_categoryName, l_baseAttributes);

        std::bitset<1024> l_newSignature;
        l_newSignature.set(m_size);

        m_nameToSignature.insert({l_newCategoryPtr->m_name, l_newSignature});

        l_newCategoryPtr->m_signature = l_newSignature;

        m_signatureToCategory.insert({l_newSignature, l_newCategoryPtr});

        m_size++;

        return l_newCategoryPtr;
    }

    std::weak_ptr<Category> CategoryFactory::GetByName(std::string _name)
    {
        if(m_nameToSignature.find(_name) != m_nameToSignature.end())
        {
            std::bitset<1024> l_signature = m_nameToSignature[_name];
            return m_signatureToCategory[l_signature];
        }

        
        return std::weak_ptr<Category>();
    }

    std::weak_ptr<Category> CategoryFactory::GetBySignature(std::bitset<1024> _signature)
    {
        if(m_signatureToCategory.find(_signature) == m_signatureToCategory.end())
        {
            return std::weak_ptr<Category>();
        }

        return m_signatureToCategory[_signature];
    }

    std::vector<std::weak_ptr<Category>> CategoryFactory::GetCategories(std::bitset<1024> _combinedSignature)
    {
        std::vector<std::weak_ptr<Category>> l_toReturn;
        for(auto l_pair : m_signatureToCategory)
        {
            if((l_pair.first & _combinedSignature).any())
            {
                l_toReturn.push_back(l_pair.second);
            }
        }

        return l_toReturn;
    }

    void CategoryFactory::RenameCategory(std::bitset<1024> _signature, std::string _name)
    {
        std::string l_oldName = m_signatureToCategory[_signature]->GetName();
        m_signatureToCategory[_signature]->m_name = _name;

        m_nameToSignature.erase(l_oldName);
        m_nameToSignature[_name] = m_signatureToCategory[_signature]->m_signature;
    }
}