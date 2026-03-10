#include "RanakEngine/Log/LogManager.h"
#include "RanakEngine/Core/CoreManager.h"

#include "GL/glew.h"

#if _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <stdexcept>

namespace RanakEngine::Log
{
    Manager::Manager()
    : m_messageQueue()
    , m_threadMutex()
    {
        
    }

    Manager::~Manager()
    {
        if(m_running)
        {
            m_running = false;
            m_watchThread.join();
        }
    }

    void Manager::Monitor()
    {
        Message l_currentMessage;
        Manager* l_Logger = m_self.lock().get();
        while(!l_Logger)
        {
            // Wait for Manager to be initialised
            #if WIN32
                Sleep(100);
            #else
                usleep(10);
            #endif
            l_Logger = m_self.lock().get();
        }

        auto l_core = Core::Manager::Instance().lock();

        while(l_Logger->m_running)
        {
            if(!l_Logger->m_messageQueue.empty())
            {

                l_Logger->m_threadMutex.lock();
                
                l_currentMessage = l_Logger->m_messageQueue.front();
                l_Logger->m_messageQueue.pop();
                
                l_Logger->m_threadMutex.unlock();

                // Print non-debug messages
                switch(l_currentMessage.severity)
                {
                    case Message::DEBUG:
                    if(l_core->IsDebug())
                    {
                        printf("DEBUG: %s\n", l_currentMessage.contents.c_str());
                    }
                    break;

                    case Message::ERROR:
                    printf("ERROR: %s\n", l_currentMessage.contents.c_str());
                    throw(std::runtime_error(""));
                    break;

                    case Message::NORMAL:
                    printf("LOG: %s\n", l_currentMessage.contents.c_str());
                    break;

                    case Message::WARNING:
                    printf("WARNING: %s\n", l_currentMessage.contents.c_str());
                    break;
                }
            }



            #if WIN32
                Sleep(100);
            #else
                usleep(10);
            #endif
        }
    }

    void Manager::GLDebugMessageCallback(GLenum _source, GLenum _type, GLuint _id,
							 GLenum _severity, GLsizei _length, const GLchar* _message,
							 const void* _userParam)
    {
        if(_severity == (GLenum)0x826B) // GL_DEBUG_SEVERITY_NOTIFICATION
            return;
            
        printf("LOG: GL CALLBACK: from = 0x%x type = 0x%x, severity = 0x%x, message = %s\n",
            _source, _type, _severity, _message );
    };

    std::shared_ptr<Manager> Manager::Init()
    {
        std::shared_ptr<Manager> rtn = std::shared_ptr<Manager>();
        Manager* l_logger = new Manager();
        rtn.reset(l_logger);

        l_logger->m_running = true;
        l_logger->m_watchThread = std::thread(Manager::Monitor);

        rtn->m_self = rtn;
        return rtn;
    }

    std::weak_ptr<Manager> Manager::Instance()
    {
        if(m_self.lock() == nullptr)
        {
            throw(std::runtime_error("ERROR: Manager::Instance(), Manager has not been initialised!\n"));
        }
        
        return m_self;
    }

    void Manager::Stop()
    {
        m_running = false;

        if(m_watchThread.joinable())
        {
            m_watchThread.join();
        }

        // Print out the rest of the queue
        Message l_currentMessage;
        while(!m_messageQueue.empty())
        {
            l_currentMessage = m_messageQueue.front();
            m_messageQueue.pop();

            // Print non-debug messages
            switch(l_currentMessage.severity)
            {
                case Message::DEBUG:
                if(Core::Manager::Instance().lock()->IsDebug())
                {
                    printf("DEBUG: %s\n", l_currentMessage.contents.c_str());
                }
                break;

                case Message::ERROR:
                printf("ERROR: %s\n", l_currentMessage.contents.c_str());
                throw(std::runtime_error(""));
                break;

                case Message::NORMAL:
                printf("LOG: %s\n", l_currentMessage.contents.c_str());
                break;

                case Message::WARNING:
                printf("WARNING: %s\n", l_currentMessage.contents.c_str());
                break;
            }
        }
    }

    void Log::Manager::LogMessage(int _severity, std::string _message)
    {
        Message l_newMessage;
        l_newMessage.severity = (Message::Severity)_severity;
        l_newMessage.contents = _message;

        auto l_self = m_self.lock();

        // Doesn't cause an error if logging isn't started
        if(l_self)
        {
            l_self->m_threadMutex.lock();
            l_self->m_messageQueue.push(Message{(Message::Severity)_severity, _message});
            l_self->m_threadMutex.unlock();
        }
    }
}