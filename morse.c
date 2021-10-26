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
 * This code converts a character of text into a morse sequence. It does
 * this by looking up the character in the ASCII table below. It then
 * generates the correct sequence of dits and dahs, with the appropriate
 * silences between them.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "libmorse.h"

/*
 * Morse code table. Each entry is nine bits long. The first three bits
 * specify the length of the sequence (usually somewhere between 1 and 6).
 * The next six bits are the actual morse values, in reverse order. A one
 * is a dah and a zero is a dit.
 */
unsigned short	morse_table[128] = {
/*00*/	0000,0000,0000,0000,0000,0000,0000,0000,
/*08*/	0000,0000,0412,0000,0000,0412,0000,0000,
/*10*/	0000,0000,0000,0000,0000,0000,0000,0000,
/*18*/	0000,0000,0000,0000,0000,0000,0000,0000,
/*20*/	0000,0665,0622,0000,0000,0000,0502,0636,
/*28*/	0515,0000,0000,0512,0663,0000,0652,0511,
/*30*/	0537,0536,0534,0530,0520,0500,0501,0503,
/*38*/	0507,0517,0607,0625,0000,0521,0000,0614,
/*40*/	0000,0202,0401,0405,0301,0100,0404,0303,
/*48*/	0400,0200,0416,0305,0402,0203,0201,0307,
/*50*/	0406,0413,0302,0300,0101,0304,0410,0306,
/*58*/	0411,0415,0403,0000,0000,0000,0000,0000,
/*60*/	0000,0202,0401,0405,0301,0100,0404,0303,
/*68*/	0400,0200,0416,0305,0402,0203,0201,0307,
/*70*/	0406,0413,0302,0300,0101,0304,0410,0306,
/*78*/	0411,0415,0403,0000,0000,0000,0000,0000
};

void	_morse_commence(struct morse *);

/*
 * Transmit one character of text as Morse Code. Use the above table to
 * figure out how many elements or symbols and the remaining bits for the
 * actual data.
 */
void
morse_send_char(struct morse *mp, int ch)
{
	int nsyms, bitreg;

	/*
	 * First time through? Then do some last-minute config, including
	 * configuring the audio channel.
	 */
	if (!mp->setup_done)
		_morse_commence(mp);
	bitreg = morse_table[ch & 0x7f];
	if ((nsyms = (bitreg >> 6) & 07) == 0)
		nsyms = 8;
	bitreg &= 077;
	while (nsyms-- > 0) {
		morse_audio_silence(mp);
		if (bitreg & 01)
			morse_audio_tone(mp, mp->bit_time * 3);
		else
			morse_audio_tone(mp, mp->bit_time);
		bitreg >>= 1;
		mp->sym_delay = mp->bit_time;
	}
	if (!mp->prosign)
		mp->sym_delay = mp->char_delay;
}

/*
 * Send a word by sending each character in turn and then waiting for the
 * word delay.
 */
void
morse_send_word(struct morse *mp, char *strp)
{
	if (strp == NULL || *strp == '\0')
		return;
	if (*strp == '<') {
		char *cp;

		strp++;
		if ((cp = strchr(strp, '>')) != NULL)
			*cp = '\0';
		mp->prosign = 1;
	} else
		mp->prosign = 0;
	while (*strp != '\0')
		morse_send_char(mp, *strp++);
	mp->prosign = 0;
	mp->sym_delay = mp->word_delay;
}

/*
 * Send a string of characters/words as Morse code. Really the work is
 * done in the previous two functions. This code just splits the string
 * into words.
 */
void
morse_send_string(struct morse *mp, char *strp)
{
	char *cp;

	while (strp != NULL && *strp != '\0') {
		if ((cp = strpbrk(strp, " \t")) != NULL) {
			*cp++ = '\0';
			while (isspace(*cp))
				cp++;
		}
		morse_send_word(mp, strp);
		strp = cp;
	}
}
