#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <memory>
#include <map>
#include <filesystem>

#include "RanakEngine/Log.h"
#include "RanakEngine/Asset/AssetFile.h"

namespace RanakEngine::Asset
{
    class Manager
    {
        private:
        static inline std::weak_ptr<Asset::Manager> m_self;
        std::map<std::string, std::shared_ptr<AssetFile>> m_resourceMap;

        Manager();
        public:
        ~Manager();
        static std::shared_ptr<Asset::Manager> Init();
        static std::weak_ptr<Asset::Manager> Instance();
        
        template<typename T>
        std::weak_ptr<T> Load(std::string _path)
        {
            std::filesystem::path l_fsPath(_path);
            if(!std::filesystem::exists(l_fsPath))
            {
                Log::Error("Asset::Load<" + std::string(typeid(T).name()) + ">, File does not exist: " + _path);
                return std::weak_ptr<T>();
            }

            if(m_resourceMap[_path] == nullptr)
            {
                std::shared_ptr<T> l_newAsset;
                
                l_newAsset = std::make_shared<T>(_path);
                
                m_resourceMap[_path] = std::static_pointer_cast<AssetFile>(l_newAsset);
            }
            
            return std::static_pointer_cast<T>(m_resourceMap[_path]);
        };
    };
}

#endif