#ifndef IOAUDIO_H
#define IOAUDIO_H

#include <string>
#include <map>
#include <memory>

#include "SDL3/SDL_audio.h"

namespace RanakEngine::Asset
{
    class Audio;
};

namespace RanakEngine::IO
{
    class Audio
    {
        private:
        SDL_AudioDeviceID m_audioDevice;
        std::map<std::string, SDL_AudioStream*> m_activeStreams;
        
        public:
        Audio();
        ~Audio();

        bool Play(std::weak_ptr<Asset::Audio> _audio, bool _loop);
        void Stop(const std::string& _audioName);
        void Pause(const std::string& _audioName);
        void Resume(const std::string& _audioName);
        void SetAudioVolume(const std::string& _audioName, float _volume);
        void StopAllAudio();
    };
}

#endif