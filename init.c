/*
 * Copyright (c) 2020-21, Kalopa Robotics Limited.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ABSTRACT
 * Initialize the library. Also the close and drain functions.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <alsa/asoundlib.h>

#include "libmorse.h"

static char		*device = "default";

/*
 * Initialize the Morse Code library. Called with the desired words per
 * minute (an integer in the range of 5 <= wpm <= 60).
 */
struct morse *
morse_init(int wpm)
{
	int err;
	snd_pcm_t *handle;
	struct morse *mp;

	/*
	 * Initialize the basic elements.
	 */
	if ((mp = (struct morse *)malloc(sizeof(struct morse))) == NULL)
		return(NULL);
	mp->wpm = wpm;
	mp->setup_done = 0;
	mp->farnsworth = 0;
	mp->prosign = 0;
	mp->amplitude = 85;
	mp->sample_rate = 44100;
	mp->tone_frequency = 800.0;
	morse_calc_params(mp);
	/*
	 * Now initialize the audio output.
	 */
	if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "libmorse init: snd_pcm_open: %s\n", snd_strerror(err));
		exit(1);
	}
	mp->audio = (void *)handle;
	return(mp);
}

/*
 * Called prior to close. This ensure that any buffered audio is written and
 * we wait until the audio has actually been sent. Don't bother with this
 * code if you just want to exit or close down the library.
 */
void
morse_drain(struct morse *mp)
{
	int err;
	snd_pcm_t *handle = (snd_pcm_t *)mp->audio;

	morse_audio_silence(mp);
	if (mp->offset > 0)
		snd_pcm_writei(handle, mp->buffer, mp->offset);
	if ((err = snd_pcm_drain(handle)) < 0)
		fprintf(stderr, "libmorse drain: snd_pcm_drain: %s\n", snd_strerror(err));
}

/*
 * Close the library. Doesn't do much except release the ALSA audio channel.
 */
void
morse_close(struct morse *mp)
{
	snd_pcm_t *handle = (snd_pcm_t *)mp->audio;

	snd_pcm_close(handle);
}
