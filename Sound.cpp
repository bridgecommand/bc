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

Sound::Sound() {
	
	char buf[1024];
	sf_command(NULL, SFC_GET_LIB_VERSION, buf, sizeof(buf));

	portAudioError = Pa_Initialize();
	if (portAudioError == paNoError) {
		std::cout << Pa_GetVersionText() << std::endl;
	}

	/* Open the soundfile */
	data.file = sf_open("Sounds/Bwave.wav", SFM_READ, &data.info);

	if (sf_error(file) != SF_ERR_NO_ERROR)
	{
		fprintf(stderr, "%s\n", sf_strerror(file));
	}


	/* Open PaStream with values read from the file */
	portAudioError = Pa_OpenDefaultStream(&stream
		, 0                     /* no input */
		, data.info.channels         /* stereo out */
		, paFloat32             /* floating point */
		, data.info.samplerate
		, FRAMES_PER_BUFFER
		, callback
		, &data);        /* our sndfile data struct */
	if (portAudioError != paNoError)
	{
		fprintf(stderr, "Problem opening Default Stream\n");
		
	}


	

}

void Sound::StartSound() {
	/* Start the stream */
	
	portAudioError = Pa_StartStream(stream);
	if (portAudioError != paNoError)
	{
		fprintf(stderr, "Problem opening starting Stream\n");
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
	sf_close(file);
}