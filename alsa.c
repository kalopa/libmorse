/*
 * Copyright (c) 2022, Kalopa Robotics Limited.  All rights
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
 * This code handles the actual operation of creating a tone (or silence)
 * for morse output. It relies heavily on the ALSA sound library for the
 * dirty work of getting audio out.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "libmorse.h"

#ifdef ALSA
#include <alsa/asoundlib.h>

static char		*device = "default";

/*
 * Initialise the ALSA library.
 */
void
sound_open(struct morse *mp)
{
	int err;
	snd_pcm_t *handle = NULL;

	if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "libmorse init: snd_pcm_open: %s\n", snd_strerror(err));
		exit(1);
	}
	mp->audio = (void *)handle;
}

/*
 * Just before we begin audio out, we need to set up some bits and pieces
 * like the audio buffer and some of the offsets. Recompute the parameters
 * for good measure, too.
 */
void
sound_commence(struct morse *mp)
{
	int err;
	snd_pcm_t *handle = (snd_pcm_t *)mp->audio;

	if ((err = snd_pcm_set_params(handle,
				SND_PCM_FORMAT_S16_LE,
				SND_PCM_ACCESS_RW_INTERLEAVED,
				1,
				mp->sample_rate,
				1,
				500000)) < 0) {
		fprintf(stderr, "libmorse: snd_pcm_set_params: %s\n", snd_strerror(err));
		exit(1);
	}
}

/*
 * Output a 16-bit audio sample. It is buffered locally and then written
 * whenever the buffer is full. We also track a time stamp so we know how
 * much audio has been written.
 */
void
sound_out(struct morse *mp, int value)
{
	snd_pcm_t *handle = (snd_pcm_t *)mp->audio;

	mp->buffer[mp->offset++] = value;
	if (mp->offset >= AUDIO_BUFFER_SIZE) {
		snd_pcm_writei(handle, mp->buffer, AUDIO_BUFFER_SIZE);
		mp->offset = 0;
	}
	mp->time_stamp++;
}

/*
 * Called prior to close. This ensure that any buffered audio is written and
 * we wait until the audio has actually been sent. Don't bother with this
 * code if you just want to exit or close down the library.
 */
void
sound_drain(struct morse *mp)
{
	int err;
	snd_pcm_t *handle = (snd_pcm_t *)mp->audio;

	if (mp->offset > 0)
		snd_pcm_writei(handle, mp->buffer, mp->offset);
	if ((err = snd_pcm_drain(handle)) < 0)
		fprintf(stderr, "libmorse drain: snd_pcm_drain: %s\n", snd_strerror(err));
}

/*
 * Close the library. Doesn't do much except release the ALSA audio channel.
 */
void
sound_close(struct morse *mp)
{
	snd_pcm_t *handle = (snd_pcm_t *)mp->audio;

	snd_pcm_close(handle);
}
#endif
#ifdef SDL
#include <SDL_audio.h>
#endif
