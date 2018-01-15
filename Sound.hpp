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

#ifndef __SOUND_HPP_INCLUDED__
#define __SOUND_HPP_INCLUDED__

#define FRAMES_PER_BUFFER   (512)

#include <sndfile.h>
#include <portaudio.h>
#include <string.h>
#include <iostream>

class Sound
{
public:

	Sound();
	~Sound();
	void StartSound();

private:

	typedef struct
	{
		SNDFILE     *file;
		SF_INFO      info;
	} callback_data_s;

	static int callback
	(const void                     *input
		, void                           *output
		, unsigned long                   frameCount
		, const PaStreamCallbackTimeInfo *timeInfo
		, PaStreamCallbackFlags           statusFlags
		, void                           *userData
	)
	{
		std::cout << "At start of callback" << std::endl;
		float           *out;
		callback_data_s *p_data = (callback_data_s*)userData;
		sf_count_t       num_read;

		std::cout << "Callback 1" << std::endl;

		out = (float*)output;
		p_data = (callback_data_s*)userData;
		std::cout << p_data << std::endl;

		std::cout << "Callback 2" << std::endl;

		/* clear output buffer */
		memset(out, 0, sizeof(float) * frameCount * p_data->info.channels);

		std::cout << "Callback 3" << std::endl;

		/* read directly into output buffer */
		num_read = sf_read_float(p_data->file, out, frameCount * p_data->info.channels);

		std::cout << "Callback 4" << std::endl;

		/*  If we couldn't read a full frameCount of samples we've reached EOF */
		if (num_read < frameCount)
		{
			return paComplete;
		}

		std::cout << "Callback 5" << std::endl;

		return paContinue;
	}

	PaError portAudioError;
	PaStream *stream;
	SNDFILE *file;
	callback_data_s data;

};

#endif