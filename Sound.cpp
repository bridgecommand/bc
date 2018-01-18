//Proof of principle - source from https://github.com/hosackm/wavplayer/blob/master/main.c
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

#include "Sound.hpp"
#include <iostream>

//Volumes, should be in range 0-1
float Sound::hornVolume = 0.0;
float Sound::waveVolume = 1.0;
float Sound::engineVolume=0.25;

Sound::Sound() {

	soundLoaded = false;

	char buf[1024];
	sf_command(NULL, SFC_GET_LIB_VERSION, buf, sizeof(buf));

	portAudioError = Pa_Initialize();
	if (portAudioError != paNoError) {
		return;
	}

	/* Open the soundfiles */
	data.fileWave = sf_open("Sounds/Bwave.wav", SFM_READ, &data.infoWave);
	if (sf_error(data.fileWave) != SF_ERR_NO_ERROR) {
		return;
	}

	data.fileEngine = sf_open("Sounds/Engine.wav", SFM_READ, &data.infoEngine);
	if (sf_error(data.fileEngine) != SF_ERR_NO_ERROR) {
		return;
	}

	data.fileHorn = sf_open("Sounds/horn.wav", SFM_READ, &data.infoHorn);
	if (sf_error(data.fileHorn) != SF_ERR_NO_ERROR) {
		return;
	}

	//Check key parameters are the same
	if (data.infoWave.channels != data.infoEngine.channels || data.infoWave.channels != data.infoHorn.channels || data.infoWave.samplerate != data.infoEngine.samplerate || data.infoWave.samplerate != data.infoHorn.samplerate) {
		std::cout << "Inconsistent formats of wave, engine or horn sounds, sound not loaded." << std::endl;
		return;
	}

	/* Open PaStream with values read from the file - all should be same, so any can be used*/
	portAudioError = Pa_OpenDefaultStream(&stream
		, 0                     /* no input */
		, data.infoWave.channels         /* stereo out */
		, paFloat32             /* floating point */
		, data.infoWave.samplerate
		, FRAMES_PER_BUFFER
		, callback
		, &data);        /* our sndfile data struct */
	if (portAudioError != paNoError)
	{
		return;
	}

	soundLoaded = true; // All OK if we've got here

}

void Sound::StartSound() {
	/* Start the stream */
	
	if (soundLoaded) {
		portAudioError = Pa_StartStream(stream);
		if (portAudioError != paNoError)
		{
			fprintf(stderr, "Problem opening starting Stream\n");
		}
	}

}

Sound::~Sound() {
	
	portAudioError = Pa_CloseStream(stream);
	if (portAudioError != paNoError)
	{
		fprintf(stderr, "Problem closing stream\n");
	}
	
	portAudioError = Pa_Terminate();
	if (portAudioError == paNoError) {
		std::cout << "PortAudio terminated" << std::endl;
	}

	/* Close the soundfile */
	sf_close(data.fileWave);
	sf_close(data.fileEngine);
	sf_close(data.fileHorn);
}