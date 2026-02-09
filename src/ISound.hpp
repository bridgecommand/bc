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

#ifndef __ISOUND_HPP_INCLUDED__
#define __ISOUND_HPP_INCLUDED__

#include <string>

// Sound source indices for 3D positioning (used by both backends)
enum SoundSource {
    SOUND_SRC_ENGINE = 0,
    SOUND_SRC_WAVE = 1,
    SOUND_SRC_HORN = 2,
    SOUND_SRC_ALARM = 3
};

// Abstract sound interface used by SimulationModel and main.
// Implemented by Sound (PortAudio) and SoundOpenAL (OpenAL).
class ISound {
public:
    virtual ~ISound() = default;

    virtual void load(std::string engineSoundFile, std::string waveSoundFile,
                      std::string hornSoundFile, std::string alarmSoundFile) = 0;
    virtual void StartSound() = 0;

    virtual void setVolumeWave(float vol) = 0;
    virtual void setVolumeEngine(float vol) = 0;
    virtual void setVolumeHorn(float vol) = 0;
    virtual void setVolumeAlarm(float vol) = 0;

    virtual float getVolumeWave() const = 0;
    virtual float getVolumeEngine() const = 0;
    virtual float getVolumeHorn() const = 0;
    virtual float getVolumeAlarm() const = 0;

    // Engine pitch varies with RPM (0.5 = idle, 1.0 = full power)
    // Default no-op for backends that don't support pitch/3D (PortAudio)
    virtual void setEnginePitch(float pitch) { (void)pitch; }

    // 3D spatial audio — no-op for non-spatial backends
    virtual void setListenerPosition(float x, float y, float z) { (void)x; (void)y; (void)z; }
    virtual void setListenerOrientation(float fwdX, float fwdY, float fwdZ,
                                         float upX, float upY, float upZ) {
        (void)fwdX; (void)fwdY; (void)fwdZ; (void)upX; (void)upY; (void)upZ;
    }
    virtual void setSourcePosition(int id, float x, float y, float z) {
        (void)id; (void)x; (void)y; (void)z;
    }
    virtual void setSourceVelocity(int id, float vx, float vy, float vz) {
        (void)id; (void)vx; (void)vy; (void)vz;
    }

    // HRTF for VR — no-op for non-spatial backends
    virtual bool enableHRTF() { return false; }
    virtual void disableHRTF() {}
    virtual bool isHRTFEnabled() const { return false; }
};

#endif // __ISOUND_HPP_INCLUDED__
