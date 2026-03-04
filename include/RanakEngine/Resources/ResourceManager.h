#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <memory>
#include <map>

#include "RanakEngine/Log.h"
#include "RanakEngine/Resources/Resource.h"

namespace RanakEngine::Resources
{
    class Manager
    {
        private:
        static inline std::weak_ptr<Resources::Manager> m_self;
        std::map<std::string, std::shared_ptr<Resource>> m_resourceMap;

        Manager() : m_resourceMap() {};
        public:
        ~Manager() {};
        static std::shared_ptr<Resources::Manager> Init();
        static std::weak_ptr<Resources::Manager> Instance();
        
        template<typename T>
        std::weak_ptr<T> Load(std::string _path)
        {
            std::filesystem::path l_fsPath(_path);
            if(!std::filesystem::exists(l_fsPath))
            {
                Logger::LogMessage(Log::Message::WARNING, "Resources::Load<" + std::string(typeid(T).name()) + ">, File does not exist: " + _path);
                return std::weak_ptr<T>();
            }

            if(m_resourcesMap[_path] == nullptr)
            {
                std::shared_ptr<T> l_newResource;
                
                l_newResource = std::make_shared<T>(_path);
                
                m_resourcesMap[_path] = std::static_pointer_cast<Resource>(l_newResource);
            }
            
            return std::static_pointer_cast<T>(m_resourcesMap[_path]);
        };
    };
}

#endif