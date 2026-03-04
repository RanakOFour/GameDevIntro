#ifndef IOAUDIO_H
#define IOAUDIO_H

#include <string>
#include <map>

#include "SDL3/SDL_audio.h"

namespace RanakEngine::IO
{
    class Audio
    {
        private:
        SDL_AudioDeviceID m_audioDevice;
        std::map<std::string, SDL_AudioStream> m_streams;
        
        public:
        Audio();
        ~Audio();

        void Start(std::string _name);
        void Stop(std::string _name);
        void Pause(std::string _name);
        void Resume(std::string _name);
        void SetAudioVolume(std::string _name, float _volume);
        void StopAllAudio();
    };
}

#endif