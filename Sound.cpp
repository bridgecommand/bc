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
	std::cout << buf << std::endl;

	portAudioError = Pa_Initialize();
	if (portAudioError == paNoError) {
		std::cout << Pa_GetVersionText() << std::endl;
	}

	
}

Sound::~Sound() {
	portAudioError = Pa_Terminate();
	if (portAudioError == paNoError) {
		std::cout << "PortAudio terminated" << std::endl;
	}
}