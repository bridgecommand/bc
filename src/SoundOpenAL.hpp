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

#ifndef __SOUNDOPENAL_HPP_INCLUDED__
#define __SOUNDOPENAL_HPP_INCLUDED__

#ifdef WITH_OPENAL

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <sndfile.h>
#include <string>
#include <vector>

#include "ISound.hpp"

// Sound source IDs for the standard ship sounds
enum SoundID {
    SOUND_ENGINE = 0,
    SOUND_WAVE,
    SOUND_HORN,
    SOUND_ALARM,
    SOUND_COUNT
};

class SoundOpenAL : public ISound {
public:
    SoundOpenAL();
    ~SoundOpenAL() override;

    bool init();
    void shutdown();

    // Load the standard ship sounds from WAV files via libsndfile
    bool loadSounds(const std::string& engineFile,
                    const std::string& waveFile,
                    const std::string& hornFile,
                    const std::string& alarmFile);

    // Playback control
    void play(SoundID id, bool loop = true);
    void stop(SoundID id);

    // Volume (0.0 to 1.0)
    void setVolume(SoundID id, float vol);
    float getVolume(SoundID id) const;

    // 3D spatial positioning (SoundID-based API)
    void setListenerPosition(float x, float y, float z) override;
    void setListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                 float upX, float upY, float upZ) override;
    void setSourcePosition(SoundID id, float x, float y, float z);
    void setSourceVelocity(SoundID id, float vx, float vy, float vz);

    // ISound 3D overrides (int-based, forwarding to SoundID)
    void setSourcePosition(int id, float x, float y, float z) override {
        setSourcePosition(static_cast<SoundID>(id), x, y, z);
    }
    void setSourceVelocity(int id, float vx, float vy, float vz) override {
        setSourceVelocity(static_cast<SoundID>(id), vx, vy, vz);
    }

    bool isInitialised() const { return initialised; }

    // HRTF (Head-Related Transfer Function) for VR audio
    // Only activate when VR head tracking is active -- HRTF sounds wrong without it.
    // Requires OpenAL Soft (not Apple's built-in OpenAL).
    bool enableHRTF() override;
    void disableHRTF() override;
    bool isHRTFEnabled() const override { return hrtfEnabled; }

    // ISound interface — wrappers over the SoundID-based API
    void load(std::string engineSoundFile, std::string waveSoundFile,
              std::string hornSoundFile, std::string alarmSoundFile) override;
    void StartSound() override;
    void setVolumeWave(float vol) override   { setVolume(SOUND_WAVE, vol); }
    void setVolumeEngine(float vol) override { setVolume(SOUND_ENGINE, vol); }
    void setVolumeHorn(float vol) override   { setVolume(SOUND_HORN, vol); }
    void setVolumeAlarm(float vol) override  { setVolume(SOUND_ALARM, vol); }
    float getVolumeWave() const override     { return getVolume(SOUND_WAVE); }
    float getVolumeEngine() const override   { return getVolume(SOUND_ENGINE); }
    float getVolumeHorn() const override     { return getVolume(SOUND_HORN); }
    float getVolumeAlarm() const override    { return getVolume(SOUND_ALARM); }
    void setEnginePitch(float pitch) override;

private:
    ALuint loadWavFile(const std::string& filename);

    ALCdevice* device;
    ALCcontext* context;
    bool initialised;
    bool hrtfEnabled = false;

    ALuint buffers[SOUND_COUNT];
    ALuint sources[SOUND_COUNT];
    float volumes[SOUND_COUNT];
};

#else // WITH_OPENAL

// Dummy implementation when OpenAL is not available
#include "ISound.hpp"

class SoundOpenAL : public ISound {
public:
    SoundOpenAL() {}
    ~SoundOpenAL() override {}
    bool init() { return false; }
    void shutdown() {}
    bool loadSounds(const std::string&, const std::string&,
                    const std::string&, const std::string&) { return false; }
    bool isInitialised() const { return false; }
    bool enableHRTF() override { return false; }
    void disableHRTF() override {}
    bool isHRTFEnabled() const override { return false; }

    // ISound interface — dummy
    void load(std::string, std::string, std::string, std::string) override {}
    void StartSound() override {}
    void setVolumeWave(float) override {}
    void setVolumeEngine(float) override {}
    void setVolumeHorn(float) override {}
    void setVolumeAlarm(float) override {}
    float getVolumeWave() const override { return 0; }
    float getVolumeEngine() const override { return 0; }
    float getVolumeHorn() const override { return 0; }
    float getVolumeAlarm() const override { return 0; }
};

#endif // WITH_OPENAL

#endif // __SOUNDOPENAL_HPP_INCLUDED__
