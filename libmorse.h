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
 * The morse library (libmorse) provides a simple interface for creating
 * an audio stream of morse code based on a given character string. It
 * can be used to build code testers or just simple Morse Code tools. Have
 * fun!
 */
#define AUDIO_BUFFER_SIZE	16*1024

struct  morse	{
	/*
	 * The following parameters can be modified/examined. If you
	 * change any of these, make sure to call morse_calc_params()
	 * to make the appropriate adjustments.
	 *    wpm:            Words per minute (5->60)
	 *    farnsworth:     Set to non-zero for farnsworth speeds
	 *    amplitude:      Signal amplitude (0->100.0)
	 *    sample_rate:    Audio sample rate (usually 44.1kHz)
	 *    tone_frequency: Audio tone - 800Hz is a good value
	 */
	int				wpm;
	int				farnsworth;
	int				amplitude;
	int				sample_rate;
	double			tone_frequency;
	/*
	 * Do not modify any of the following parameters.
	 */
	int				setup_done;
	unsigned int	time_stamp;
	unsigned int	bit_time;
	unsigned int	char_delay;
	unsigned int	word_delay;
	unsigned int	sym_delay;
	unsigned short	word;
	int				offset;
	void			*audio;
	unsigned short	*buffer;
};

/*
 * Prototypes...
 */
struct morse	*morse_init(int);
void			morse_send_char(struct morse *, int);
void			morse_send_word(struct morse *, char *);
void			morse_send_string(struct morse *, char *);
void			morse_drain(struct morse *);
void			morse_close(struct morse *);
double			morse_timestamp(struct morse *);
void			morse_calc_params(struct morse *);
void			morse_audio_tone(struct morse *, int);
void			morse_audio_silence(struct morse *);
