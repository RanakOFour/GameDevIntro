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
            if(_path.find_first_of(';') == _path.npos)
            {
                std::filesystem::path l_fsPath(_path);
                if(!std::filesystem::exists(l_fsPath))
                {
                    Log::Error("Asset::Load<" + std::string(typeid(T).name()) + ">, File does not exist: " + _path);
                    return std::weak_ptr<T>();
                }
            }
            else
            {
                int l_splitPos = _path.find_first_of(';');

                std::string l_path1 = _path.substr(0, l_splitPos);
                std::string l_path2 = _path.substr(l_splitPos + 1, _path.size() - l_splitPos);

                if(!std::filesystem::exists(l_path1))
                {
                    Log::Error("Asset::Load<" + std::string(typeid(T).name()) + ">, File does not exist: " + _path);
                    return std::weak_ptr<T>();
                }

                if(!std::filesystem::exists(l_path2))
                {
                    Log::Error("Asset::Load<" + std::string(typeid(T).name()) + ">, File does not exist: " + _path);
                    return std::weak_ptr<T>();
                }
            }
            

            if(m_resourceMap.find(_path) == m_resourceMap.end())
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