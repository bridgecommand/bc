/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2024 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#include "SoundOpenAL.hpp"

#ifdef WITH_OPENAL

#include <iostream>
#include <cstring>
#include <vector>

SoundOpenAL::SoundOpenAL()
    : device(nullptr), context(nullptr), initialised(false)
{
    std::memset(buffers, 0, sizeof(buffers));
    std::memset(sources, 0, sizeof(sources));
    for (int i = 0; i < SOUND_COUNT; i++) {
        volumes[i] = 0.0f;
    }
}

SoundOpenAL::~SoundOpenAL() {
    shutdown();
}

bool SoundOpenAL::init() {
    // Open default audio device
    device = alcOpenDevice(nullptr);
    if (!device) {
        std::cerr << "SoundOpenAL: Failed to open audio device" << std::endl;
        return false;
    }

    // Create audio context
    context = alcCreateContext(device, nullptr);
    if (!context) {
        std::cerr << "SoundOpenAL: Failed to create audio context" << std::endl;
        alcCloseDevice(device);
        device = nullptr;
        return false;
    }

    if (!alcMakeContextCurrent(context)) {
        std::cerr << "SoundOpenAL: Failed to make context current" << std::endl;
        alcDestroyContext(context);
        alcCloseDevice(device);
        context = nullptr;
        device = nullptr;
        return false;
    }

    // Generate sources
    alGenSources(SOUND_COUNT, sources);
    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "SoundOpenAL: Failed to generate sources" << std::endl;
        shutdown();
        return false;
    }

    // Set distance model for 3D attenuation
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    // Configure Doppler effect for realistic pitch shift on moving sources
    alDopplerFactor(1.0f);     // 1.0 = physically accurate Doppler
    alSpeedOfSound(343.3f);    // Speed of sound in air at ~20°C (m/s)

    // Set default attenuation parameters for each source
    for (int i = 0; i < SOUND_COUNT; i++) {
        alSourcef(sources[i], AL_REFERENCE_DISTANCE, 10.0f);  // Full volume within 10m
        alSourcef(sources[i], AL_MAX_DISTANCE, 1000.0f);      // Audible up to 1km
        alSourcef(sources[i], AL_ROLLOFF_FACTOR, 1.0f);
    }

    initialised = true;
    std::cout << "SoundOpenAL: Initialised successfully" << std::endl;
    return true;
}

void SoundOpenAL::shutdown() {
    if (!initialised) return;

    // Stop all sources
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (sources[i]) {
            alSourceStop(sources[i]);
        }
    }

    // Delete sources
    alDeleteSources(SOUND_COUNT, sources);
    std::memset(sources, 0, sizeof(sources));

    // Delete buffers
    alDeleteBuffers(SOUND_COUNT, buffers);
    std::memset(buffers, 0, sizeof(buffers));

    // Destroy context and close device
    alcMakeContextCurrent(nullptr);
    if (context) {
        alcDestroyContext(context);
        context = nullptr;
    }
    if (device) {
        alcCloseDevice(device);
        device = nullptr;
    }

    initialised = false;
}

ALuint SoundOpenAL::loadWavFile(const std::string& filename) {
    SF_INFO sfInfo;
    std::memset(&sfInfo, 0, sizeof(sfInfo));

    SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfInfo);
    if (!file) {
        std::cerr << "SoundOpenAL: Failed to open " << filename
                  << ": " << sf_strerror(nullptr) << std::endl;
        return 0;
    }

    // Read all samples as float, then convert to 16-bit PCM for OpenAL
    sf_count_t totalSamples = sfInfo.frames * sfInfo.channels;
    std::vector<float> floatData(totalSamples);
    sf_count_t readCount = sf_read_float(file, floatData.data(), totalSamples);
    sf_close(file);

    if (readCount < totalSamples) {
        std::cerr << "SoundOpenAL: Short read from " << filename << std::endl;
    }

    // Convert to 16-bit PCM
    std::vector<short> pcmData(readCount);
    for (sf_count_t i = 0; i < readCount; i++) {
        float sample = floatData[i];
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        pcmData[i] = static_cast<short>(sample * 32767.0f);
    }

    // Determine OpenAL format
    ALenum format;
    if (sfInfo.channels == 1) {
        format = AL_FORMAT_MONO16;
    } else if (sfInfo.channels == 2) {
        format = AL_FORMAT_STEREO16;
    } else {
        std::cerr << "SoundOpenAL: Unsupported channel count "
                  << sfInfo.channels << " in " << filename << std::endl;
        return 0;
    }

    // Create and fill OpenAL buffer
    ALuint buffer;
    alGenBuffers(1, &buffer);
    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "SoundOpenAL: Failed to generate buffer for " << filename << std::endl;
        return 0;
    }

    alBufferData(buffer, format, pcmData.data(),
                 static_cast<ALsizei>(readCount * sizeof(short)),
                 sfInfo.samplerate);

    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "SoundOpenAL: Failed to fill buffer for " << filename << std::endl;
        alDeleteBuffers(1, &buffer);
        return 0;
    }

    std::cout << "SoundOpenAL: Loaded " << filename
              << " (" << sfInfo.frames << " frames, "
              << sfInfo.channels << " ch, "
              << sfInfo.samplerate << " Hz)" << std::endl;

    return buffer;
}

bool SoundOpenAL::loadSounds(const std::string& engineFile,
                              const std::string& waveFile,
                              const std::string& hornFile,
                              const std::string& alarmFile) {
    if (!initialised) return false;

    struct SoundFile {
        SoundID id;
        const std::string& file;
    };

    SoundFile files[] = {
        {SOUND_ENGINE, engineFile},
        {SOUND_WAVE, waveFile},
        {SOUND_HORN, hornFile},
        {SOUND_ALARM, alarmFile}
    };

    bool allLoaded = true;
    for (auto& sf : files) {
        ALuint buf = loadWavFile(sf.file);
        if (buf == 0) {
            std::cerr << "SoundOpenAL: Skipping " << sf.file << std::endl;
            allLoaded = false;
            continue;
        }
        buffers[sf.id] = buf;
        alSourcei(sources[sf.id], AL_BUFFER, buf);
    }

    // Set initial volumes
    volumes[SOUND_ENGINE] = 0.0f;
    volumes[SOUND_WAVE] = 1.0f;
    volumes[SOUND_HORN] = 0.0f;
    volumes[SOUND_ALARM] = 0.0f;

    return allLoaded;
}

void SoundOpenAL::play(SoundID id, bool loop) {
    if (!initialised || id >= SOUND_COUNT) return;
    if (buffers[id] == 0) return; // No buffer loaded

    alSourcei(sources[id], AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcePlay(sources[id]);
}

void SoundOpenAL::stop(SoundID id) {
    if (!initialised || id >= SOUND_COUNT) return;
    alSourceStop(sources[id]);
}

void SoundOpenAL::setVolume(SoundID id, float vol) {
    if (id >= SOUND_COUNT) return;
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
    volumes[id] = vol;
    if (initialised) {
        alSourcef(sources[id], AL_GAIN, vol);
    }
}

float SoundOpenAL::getVolume(SoundID id) const {
    if (id >= SOUND_COUNT) return 0.0f;
    return volumes[id];
}

void SoundOpenAL::setListenerPosition(float x, float y, float z) {
    if (!initialised) return;
    alListener3f(AL_POSITION, x, y, z);
}

void SoundOpenAL::setListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                          float upX, float upY, float upZ) {
    if (!initialised) return;
    float orientation[] = {forwardX, forwardY, forwardZ, upX, upY, upZ};
    alListenerfv(AL_ORIENTATION, orientation);
}

void SoundOpenAL::setSourcePosition(SoundID id, float x, float y, float z) {
    if (!initialised || id >= SOUND_COUNT) return;
    alSource3f(sources[id], AL_POSITION, x, y, z);
}

void SoundOpenAL::setSourceVelocity(SoundID id, float vx, float vy, float vz) {
    if (!initialised || id >= SOUND_COUNT) return;
    alSource3f(sources[id], AL_VELOCITY, vx, vy, vz);
}

// ISound interface wrappers

void SoundOpenAL::load(std::string engineSoundFile, std::string waveSoundFile,
                        std::string hornSoundFile, std::string alarmSoundFile) {
    if (!initialised) {
        if (!init()) return;
    }
    loadSounds(engineSoundFile, waveSoundFile, hornSoundFile, alarmSoundFile);
}

void SoundOpenAL::setEnginePitch(float pitch) {
    if (!initialised) return;
    if (pitch < 0.5f) pitch = 0.5f;
    if (pitch > 2.0f) pitch = 2.0f;
    alSourcef(sources[SOUND_ENGINE], AL_PITCH, pitch);
}

void SoundOpenAL::StartSound() {
    if (!initialised) return;
    // Start all loaded sounds looping (matching PortAudio behaviour)
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (buffers[i] != 0) {
            play(static_cast<SoundID>(i), true);
        }
    }
}

#endif // WITH_OPENAL
