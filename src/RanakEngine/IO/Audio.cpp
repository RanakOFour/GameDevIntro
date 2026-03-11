#include "RanakEngine/IO/Audio.h"
#include "RanakEngine/Asset/Audio.h"
#include "RanakEngine/Log.h"

#include "SDL3/SDL.h"

namespace RanakEngine::IO
{
    Audio::Audio()
    : m_audioDevice(0)
    {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
        {
            Log::Error("Failed to initialize SDL Audio: " + std::string(SDL_GetError()));
        }
        else
        {
            // Open default audio device
            m_audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
            if (m_audioDevice == 0)
            {
                Log::Error("Failed to open audio device: " + std::string(SDL_GetError()));
            }
            else
            {
                SDL_ResumeAudioDevice(m_audioDevice);
                Log::Debug("Audio device initialized");
            }
        }
    }

    Audio::~Audio()
    {
        StopAllAudio();
        
        if (m_audioDevice != 0)
        {
            SDL_CloseAudioDevice(m_audioDevice);
        }
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

    bool Audio::Play(std::weak_ptr<Asset::Audio> _audio, bool _loop)
    {
        Asset::Audio* l_audio = _audio.lock().get();
        if (!l_audio)
        {
            Log::Warning("PlayAudio: Audio resource is null");
            return false;
        }
        
        // Create audio stream from the loaded audio data
        SDL_AudioStream* l_stream = SDL_CreateAudioStream(
            &l_audio->GetSpec(),
            &l_audio->GetSpec()
        );
        
        if (!l_stream)
        {
            Log::Error("Failed to create audio stream: " + std::string(SDL_GetError()));
            return false;
        }
        
        // Add audio data to stream
        if (SDL_PutAudioStreamData(l_stream, 
            l_audio->GetBuffer(), 
            l_audio->GetBufferSize()) < 0)
        {
            Log::Error("Failed to queue audio data");
            SDL_DestroyAudioStream(l_stream);
            return false;
        }
        
        // Bind stream to device and play
        if (SDL_BindAudioStream(m_audioDevice, l_stream) < 0)
        {
            Log::Error("Failed to bind audio stream");
            SDL_DestroyAudioStream(l_stream);
            return false;
        }
        
        // Store stream for later control
        m_activeStreams[l_audio->GetPath()] = l_stream;
        
        Log::Debug("Now playing: " + l_audio->GetPath());
        
        return true;
    }

    void Audio::Stop(const std::string& _audioName)
    {
        auto stream = m_activeStreams.find(_audioName);
        if (stream != m_activeStreams.end())
        {
            SDL_UnbindAudioStream(stream->second);
            SDL_DestroyAudioStream(stream->second);
            m_activeStreams.erase(stream);
        }
    }

    void Audio::Pause(const std::string& _audioName)
    {
        auto stream = m_activeStreams.find(_audioName);
        if (stream != m_activeStreams.end())
        {
            SDL_PauseAudioStreamDevice(stream->second);
        }
    }

    void Audio::Resume(const std::string& _audioName)
    {
        auto stream = m_activeStreams.find(_audioName);
        if (stream != m_activeStreams.end())
        {
            SDL_ResumeAudioStreamDevice(stream->second);
        }
    }

    void Audio::SetAudioVolume(const std::string& _audioName, float _volume)
    {
        auto stream = m_activeStreams.find(_audioName);
        if (stream != m_activeStreams.end())
        {
            SDL_SetAudioStreamGain(stream->second, _volume);
        }
    }

    void Audio::StopAllAudio()
    {
        for (auto& pair : m_activeStreams)
        {
            SDL_UnbindAudioStream(pair.second);
            SDL_DestroyAudioStream(pair.second);
        }
        m_activeStreams.clear();
    }
}