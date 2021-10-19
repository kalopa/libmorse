/*
 * Copyright (c) 2020-21, Kalopa Robotics Limited.  All rights
 * reserved. Written by Dermot Tynan, EI4HRB <dtynan@kalopa.com>.
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

#include <alsa/asoundlib.h>

#include "libmorse.h"

/*
 * Output a 16-bit audio sample. It is buffered locally and then written
 * whenever the buffer is full. We also track a time stamp so we know how
 * much audio has been written.
 */
static void
_audio_out(struct morse *mp, int value)
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
 * Generate a sinusoidal tone at the desired frequency. We use a small curve
 * at the end of the wave form to avoid clicks. The maths here might not be
 * perfect...
 */
void
morse_audio_tone(struct morse *mp, int len)
{
	int i, fade = 0;
	double value, theta;

	fade = mp->sample_rate / 440;
	for (i = 0; i < len; i++) {
		if (i > (len - fade)) {
			/*
			 * Need to decay the last 100 or so samples to avoid clicking.
			 */
			theta = (double )(i - len + fade) * M_PI_2 / (double )fade;
			value = (1 - sin(theta)) * (double )mp->word;
		} else
			value = (double )mp->word;
		theta = (2.0 * M_PI * (double )i * (double )mp->tone_frequency / mp->sample_rate);
		_audio_out(mp, (int )(value * sin(theta) + 0.5));
	}
}

/*
 * Add a block of silence. Note that it is the larger of the element delay,
 * the character delay, and the word delay. Whichever was last.
 */
void
morse_audio_silence(struct morse *mp)
{
	while (mp->sym_delay-- > 0)
		_audio_out(mp, 0);
}

/*
 * Return the time stamp in seconds. Simply the number of values transmitted
 * divided by the samples per second.
 */
double
morse_timestamp(struct morse *mp)
{
	return((double )mp->time_stamp / (double )mp->sample_rate);
}
