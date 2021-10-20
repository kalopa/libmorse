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
 * These functions are used to compute various aspects of the Morse Code
 * timing and to make sure the audio setup makes sense. The Farnsworth piece
 * is tricky.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <alsa/asoundlib.h>

#include "libmorse.h"

#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))

/*
 * Apply some heuristics to the set parameters and compute the morse timing.
 * The maths is a little intense. For more information, there is a great
 * ARRL reference by Jon Bloom (KE3Z) entitled "A Standard for Morse
 * Timing Using the Farnsworth Technique".
 * http://www.arrl.org/files/file/Technology/x9004008.pdf
 */
void
morse_calc_params(struct morse *mp)
{
	int fw = 0;
	double my_wpm, effective_wpm, element_time;

	/*
	 * Validate the configurable parameters.
	 */
	mp->wpm = MIN(MAX(mp->wpm, 5), 60);
	mp->amplitude = MIN(MAX(mp->amplitude, 0), 100);
	mp->word = (int )((double )mp->amplitude * 327.67);
	/*
	 * In Farnsworth mode, the morse rate is pegged at 18WPM but we adjust
	 * the inter-character and inter-word delays to reduce the speed.
	 */
	my_wpm = (double )mp->wpm;
	if (mp->farnsworth && mp->wpm < 18) {
		/*
		 * At speeds below 18WPM and assuming Farnsworth mode has been
		 * requested, we use a "real" WPM of 18 and do some computation
		 * later on to compute the gaps.
		 */
		fw = 1;
		effective_wpm = 18.0;
	} else {
		/*
		 * Either we're too fast or Farnsworth isn't requested. Do this the
		 * easy way.
		 */
		fw = 0;
		effective_wpm = my_wpm;
	}
	/*
	 * We use the word PARIS as a reference. It has 31 active elements and
	 * 19 gap (char/word) elements for a total of 50 elements per word. By
	 * deduction, each element takes 1.2/WPM seconds to transmit. At the
	 * same time, from the sample rate we can determine the number of
	 * samples for each element. We can also compute the character and word
	 * delays as a function of the number of audio samples to be skipped.
	 */
	element_time = 1.2 / effective_wpm;
	mp->bit_time = (int )((double )mp->sample_rate * element_time + 0.5);
	if (fw) {
		/*
		 * Farnsworth inter-character and inter-word
		 * delays. Unfortunately there is a slight round-off
		 * error here which can result in an inexact
		 * implementation.
		 */
		element_time = (60.0/my_wpm - 37.2/effective_wpm) / 19.0;
	}
	mp->char_delay = (int )((double )mp->sample_rate * element_time * 3.0 + 0.5);
	mp->word_delay = (int )((double )mp->sample_rate * element_time * 7.0 + 0.5);
}

/*
 * Just before we begin audio out, we need to set up some bits and pieces
 * like the audio buffer and some of the offsets. Recompute the parameters
 * for good measure, too.
 *
 * This function is called automatically.
 */
void
_morse_commence(struct morse *mp)
{
	int err;
	snd_pcm_t *handle = (snd_pcm_t *)mp->audio;

	morse_calc_params(mp);
	mp->sym_delay = 0;
	mp->time_stamp = 0;
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
	mp->buffer = (unsigned short *)malloc(AUDIO_BUFFER_SIZE * sizeof(unsigned short));
	if (mp->buffer == NULL) {
		perror("libmorse: malloc");
		exit(1);
	}
	mp->offset = 0;
	mp->setup_done = 1;
}
