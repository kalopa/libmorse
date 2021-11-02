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
 * This is a simple test program for the morse library. It takes as
 * arguments an option Words/Minute value (in the range between 5 and
 * 60 WPM) as well as some text to be translated into morse code.
 *
 * The command-line options are as follows:
 *   -a NN      Set the output volume (0 -> 100)
 *   -f         Invoke "Farnsworth" mode - see the params.c file for info
 *   -s WPM     Set the WPM (a number between 5 and 60)
 *
 * Try:
 *   ./morse_play -f -s 5 CQ CQ CQ DE EI4HRB
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "libmorse.h"

void	usage();

/*
 * All life begins here...
 */
int
main(int argc, char *argv[])
{
	int i, len, wpm, ampl, fw;
	char *str;
	struct morse *mp;

	wpm = 18;
	opterr = fw = 0;
	ampl = -1;
	while ((i = getopt(argc, argv, "a:f:s:")) != EOF) {
		switch (i) {
		case 'a':
			if ((ampl = atoi(optarg)) < 0 || ampl > 100) {
				fprintf(stderr, "Amplitude between 0 and 100.\n");
				usage();
			}
			break;

		case 'f':
			fw = 1;
			if ((wpm = atoi(optarg)) < 5 || wpm > 60) {
				fprintf(stderr, "WPM value should be between 5 and 60.\n");
				usage();
			}
			break;

		case 's':
			fw = 0;
			if ((wpm = atoi(optarg)) < 5 || wpm > 60) {
				fprintf(stderr, "WPM value should be between 5 and 60.\n");
				usage();
			}
			break;

		default:
			usage();
			break;
		}
	}
	if ((argc - optind) < 1)
		usage();
	if ((mp = morse_init(wpm)) == NULL) {
		fprintf(stderr, "?Error - morse_init failed.\n");
		exit(1);
	}
	if (ampl >= 0)
		mp->amplitude = ampl;
	if (fw)
		mp->farnsworth = 1;
	for (i = len = optind; i < argc; i++)
		len += strlen(argv[i]) + 1;
	if ((str = (char *)malloc(len)) == NULL) {
		perror("morse_play: malloc");
		exit(1);
	}
	strcpy(str, argv[optind++]);
	for (; optind < argc; optind++) {
		strcat(str, " ");
		strcat(str, argv[optind]);
	}
	morse_send_string(mp, str);
	morse_drain(mp);
	printf("Total time: %.2f seconds.\n", morse_timestamp(mp));
	morse_close(mp);
	exit(0);
}

/*
 * Print a brief usage message and quit.
 */
void
usage()
{
	fprintf(stderr, "Usage: morse_play [-a AMPL][-f WPM][-s WPM] <word> [<word> ...]\n");
	fprintf(stderr, "\t-s WPM\tSet the rate in words per minute.\n");
	fprintf(stderr, "\t-f WPM\tInvoke 'Farnsworth' mode for easier learning.\n");
	fprintf(stderr, "\t-a AMPL\tAmplification - a number between 0 and 100.\n");
	exit(2);
}
