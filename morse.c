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
/*00*/	0b000000000,0b000000000,0b000000000,0b000000000,
/*04*/	0b000000000,0b000000000,0b000000000,0b000000000,
/*08*/	0b000000000,0b000000000,0b000000000,0b000000000,
/*0C*/	0b000000000,0b000000000,0b000000000,0b000000000,
/*10*/	0b000000000,0b000000000,0b000000000,0b000000000,
/*14*/	0b000000000,0b000000000,0b000000000,0b000000000,
/*18*/	0b000000000,0b000000000,0b000000000,0b000000000,
/*1C*/	0b000000000,0b000000000,0b000000000,0b000000000,
/*20*/	0b000000000,0b110110101,0b110010010,0b000000000,
/*24*/	0b000000000,0b000000000,0b000000000,0b110011110,
/*28*/	0b000000000,0b000000000,0b000000000,0b000000000,
/*2C*/	0b110110011,0b000000000,0b110101010,0b101001001,
/*30*/	0b101011111,0b101011110,0b101011100,0b101011000,
/*34*/	0b101010000,0b101000000,0b101000001,0b101000011,
/*38*/	0b101000111,0b101001111,0b110000111,0b110010101,
/*3C*/	0b000000000,0b101010001,0b000000000,0b110001100,
/*40*/	0b000000000,0b010000010,0b100000001,0b100000101,
/*44*/	0b011000001,0b001000000,0b100000100,0b011000011,
/*48*/	0b100000000,0b010000000,0b100001110,0b011000101,
/*4C*/	0b100000010,0b010000011,0b010000001,0b011000111,
/*50*/	0b100000110,0b100001011,0b011000010,0b011000000,
/*54*/	0b001000001,0b011000100,0b100001000,0b011000110,
/*58*/	0b100001001,0b100001101,0b100000011,0b000000000,
/*5C*/	0b000000000,0b000000000,0b000000000,0b000000000,
/*60*/	0b000000000,0b010000010,0b100000001,0b100000101,
/*64*/	0b011000001,0b001000000,0b100000100,0b011000011,
/*68*/	0b100000000,0b010000000,0b100001110,0b011000101,
/*6C*/	0b100000010,0b010000011,0b010000001,0b011000111,
/*70*/	0b100000110,0b100001011,0b011000010,0b011000000,
/*74*/	0b001000001,0b011000100,0b100001000,0b011000110,
/*78*/	0b100001001,0b100001101,0b100000011,0b000000000,
/*7C*/	0b000000000,0b000000000,0b000000000,0b000000000,
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
	if ((bitreg = morse_table[ch & 0x7f]) == 0x00)
		return;
	nsyms = (bitreg >> 6) & 07;
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
	while (*strp != '\0')
		morse_send_char(mp, *strp++);
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
