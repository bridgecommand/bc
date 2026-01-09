/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2018 James Packer

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

//Based on sample code from https://github.com/hosackm/wavplayer/blob/master/main.c

#include "Sound.hpp"

#ifndef WITH_SOUND
//Dummy implementation of public interface

Sound::Sound() {}
Sound::~Sound()  {}
void Sound::load(std::string engineSoundFile, std::string waveSoundFile, std::string hornSoundFile) {}
void Sound::StartSound() {}
void Sound::setVolumeWave(float vol) {}
void Sound::setVolumeEngine(float vol) {}
void Sound::setVolumeHorn(float vol) {}
float Sound::getVolumeWave() const {return 0;}
float Sound::getVolumeEngine() const {return 0;}
float Sound::getVolumeHorn() const {return 0;}

#else // WITH_SOUND

#include <iostream>

//Volumes, should be in range 0-1
float Sound::hornVolume = 0.0;
float Sound::waveVolume = 1.0;
float Sound::engineVolume=0.0;
float Sound::alarmVolume=0.0;

bool Sound::waveSoundLoaded = false;
bool Sound::hornSoundLoaded = false;
bool Sound::alarmSoundLoaded = false;

Sound::Sound() {

}

void Sound::load(std::string aBasePath)
{

  if(!aBasePath.empty())
    {
      mBasePath = aBasePath;
      setOwnShipSounds();
    }
  
  soundLoaded = false;

  char buf[1024];
  sf_command(NULL, SFC_GET_LIB_VERSION, buf, sizeof(buf));

  portAudioError = Pa_Initialize();
  if (portAudioError != paNoError) {
    std::cerr << "Pa_Initialize failed." << std::endl;
    std::cerr << "Error: " << Pa_GetErrorText(portAudioError) << std::endl;
    return;
  }

  data.fileEngine = 0;
  data.fileWave = 0;
  data.fileHorn = 0;
  data.fileAlarm = 0;

  data.infoEngine.format = 0;
  data.fileEngine = sf_open(mEnginePath.c_str(), SFM_READ, &data.infoEngine);
  if (sf_error(data.fileEngine) != SF_ERR_NO_ERROR) {
    std::cerr << "sf_error on engineSoundFile " << mEnginePath.c_str() << std::endl;
    return;
  }

  /* Open the soundfiles */
  data.infoWave.format = 0;

  data.fileWave = sf_open(mBwavePath.c_str(), SFM_READ, &data.infoWave);
  if (sf_error(data.fileWave) != SF_ERR_NO_ERROR) {
    std::cerr << "sf_error on waveSoundFile " << mBwavePath.c_str() << std::endl;
    return;
  }

  data.infoHorn.format = 0;
  data.fileHorn = sf_open(mHornPath.c_str(), SFM_READ, &data.infoHorn);
  if (sf_error(data.fileHorn) != SF_ERR_NO_ERROR) {
    std::cerr << "sf_error on hornSoundFile " << mHornPath.c_str() << " Error: " << sf_strerror(data.fileHorn) << std::endl;
    return;
  }

  data.infoAlarm.format = 0;
  data.fileAlarm = sf_open(mAlarmPath.c_str(), SFM_READ, &data.infoAlarm);
  if (sf_error(data.fileAlarm) != SF_ERR_NO_ERROR) {
    std::cerr << "sf_error on alarmSoundFile " << mAlarmPath.c_str() << " Error: " << sf_strerror(data.fileAlarm) << std::endl;
    return;
  }

  //Check key parameters are the same
  if (data.infoWave.channels != data.infoEngine.channels || data.infoWave.samplerate != data.infoEngine.samplerate) {
    //Check wave vs engine
    std::cerr << "Inconsistent formats of wave and engine sounds, wave sound not loaded." << std::endl;
  } else {
    waveSoundLoaded = true;
  }

  if (data.infoHorn.channels != data.infoEngine.channels || data.infoHorn.samplerate != data.infoEngine.samplerate ) {
    //Check wave vs engine
    std::cerr << "Inconsistent formats of horn and engine sounds, horn sound not loaded." << std::endl;
  } else {
    hornSoundLoaded = true;
  }

  if (data.infoAlarm.channels != data.infoEngine.channels || data.infoAlarm.samplerate != data.infoEngine.samplerate ) {
    //Check alarm vs engine
    std::cerr << "Inconsistent formats of alarm and engine sounds, alarm sound not loaded." << std::endl;
  } else {
    alarmSoundLoaded = true;
  }

  /* Open PaStream with values read from the file - all should be same, so any can be used*/
  portAudioError = Pa_OpenDefaultStream(&stream
					, 0                     /* no input */
					, data.infoEngine.channels         /* stereo out */
					, paFloat32             /* floating point */
					, data.infoEngine.samplerate
					, FRAMES_PER_BUFFER
					, callback
					, &data);        /* our sndfile data struct */
  if (portAudioError != paNoError)
    {
      std::cerr << "Pa_OpenDefaultStream failed." << std::endl;
      std::cerr << "Error: " << Pa_GetErrorText(portAudioError) << std::endl;
      return;
    }

  soundLoaded = true; // All OK if we've got here
  std::cout << "Sound::load succeeded" << std::endl;
}

void Sound::StartSound() {
  /* Start the stream */

  if (soundLoaded) {
    portAudioError = Pa_StartStream(stream);
    if (portAudioError != paNoError)
      {
	std::cerr << "Problem opening starting Stream" << std::endl;
	std::cerr << "Error: " << Pa_GetErrorText(portAudioError) << std::endl;
      }
  }

}

void Sound::setVolumeWave(float vol) {
  if (vol >= 0 && vol <= 1.0) {
    Sound::waveVolume = vol;
  }
}

void Sound::setVolumeEngine(float vol) {
  if (vol >= 0 && vol <= 1.0) {
    Sound::engineVolume = vol;
  }
}

void Sound::setVolumeHorn(float vol) {
  if (vol >= 0 && vol <= 1.0) {
    Sound::hornVolume = vol;
  }
}

void Sound::setVolumeAlarm(float vol) {
  if (vol >= 0 && vol <= 1.0) {
    Sound::alarmVolume = vol;
  }
}

float Sound::getVolumeWave() const {
  return Sound::waveVolume;
}

float Sound::getVolumeEngine() const {
  return Sound::engineVolume;
}

float Sound::getVolumeHorn() const {
  return Sound::hornVolume;
}

float Sound::getVolumeAlarm() const {
  return Sound::alarmVolume;
}

void Sound::startHorn() {setVolumeHorn(1);}

void Sound::endHorn() {setVolumeHorn(0);}

void Sound::setAlarm(bool alarmState)
{
  if (alarmState) {
    setVolumeAlarm(1.0);
  } else {
    setVolumeAlarm(0.0);
  }
}


void Sound::setOwnShipSounds(void)
{
  std::string parts[16] = {"/Engine.wav","/engine.wav","sounds/Engine.wav","sounds/engine.wav",
			   "/Bwave.wav","/bwave.wav","sounds/Bwave.wav","sounds/bwave.wav",
			   "/Horn.wav","/horn.wav","sounds/Horn.wav","sounds/horn.wav",
			   "/Alarm.wav","/alarm.wav","sounds/Alarm.wav","sounds/alarm.wav",
  };
  
  std::string *pSoundPath;
  std::string soundPath;
  std::ifstream file;
  
  for(unsigned char i=0,j=0;i<16,j<4;i+=4,j++)
    {
      if(0==j) pSoundPath = &mEnginePath;
      else if(1==j) pSoundPath = &mBwavePath;
      else if(2==j) pSoundPath = &mHornPath;
      else if(3==j) pSoundPath = &mAlarmPath;
      else return;
      //Define into model folder
      
      //Upper case
      soundPath = mBasePath;
      soundPath.append(parts[i]);
      file.open(soundPath.c_str());
      if(file.good()) *pSoundPath = soundPath;

      //Lower case
      soundPath = mBasePath;
      soundPath.append(parts[i+1]);  
      file.open(soundPath.c_str());
      if(file.good()) *pSoundPath = soundPath;

      //Default if not define into model folder

      //Upper case
      soundPath = parts[i+2];
      file.open(soundPath.c_str());
      if(file.good()) *pSoundPath = soundPath;

      //Lower case
      soundPath = parts[i+3];
      file.open(soundPath.c_str());
      if(file.good()) *pSoundPath = soundPath;

    }
}

Sound::~Sound() {

  if (soundLoaded && stream) {
    portAudioError = Pa_CloseStream(stream);
  }

  portAudioError = Pa_Terminate();

  /* Close the soundfile */
  if (data.fileWave) {sf_close(data.fileWave);}
  if (data.fileEngine) {sf_close(data.fileEngine);}
  if (data.fileHorn) {sf_close(data.fileHorn);}
  if (data.fileAlarm) {sf_close(data.fileAlarm);}
}

#endif // WITH_SOUND
