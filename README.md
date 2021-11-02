# libmorse - C library (and utility) for generating Morse code.

Library to generate and play morse code.
Needs ALSA for audio playout.
The intent here is to produce other packages,
such as something which scans /usr/dict/words looking for 5-letter words,
and emits them.
Then it could collect user input, and score the result.
By putting all the Morse "heavy lifting" into the library, it should be
easy to produce other applications which don't have to worry about Morse
or indeed creating audio output.

## Compiling

You will need a C compiler (such as gcc), make, and the ALSA sound library installed.
On Ubuntu, you can do something like:

    sudo apt install gcc make libasound2-dev

Once you have the prerequisites, just type:

    make

## morse\_play

This is a simple test program for the morse library.
It takes as arguments an option Words/Minute value (in the range between 5 and 60 WPM)
as well as some text to be translated into morse code.

The command-line options are as follows:
*  **-a NN**      Set the output volume (0 -> 100)
*  **-f WPM**     Invoke "Farnsworth" mode - see the params.c file for info
*  **-s WPM**     Set the WPM (a number between 5 and 60)

For example, try:

    ./morse_play -f -s 5 CQ CQ CQ DE EI4HRB

## The Farnsworth Technique

This technique involves playing back Morse at a speed such as 18 words per minute,
to avoid symbol-counting.

Here's a quote from the
[ARRL paper](http://www.arrl.org/files/file/Technology/x9004008.pdf):

> "The problem in learning Morse is that at speeds above a few WPM,
> Morse is most easily read by ear when the characters are recognized
> rhythmically rather than by counting the dots and dashes.
> But the person just learning Morse starts at very slow speeds,
> where counting is easier than recognizing the slow rhythm of the
> characters.
> So in order to increase their ability to read Morse above a few
> words per minute, students are forced to shift from the counting
> mode to the rhythm recognition mode."

The **-f** option to morse\_play enables the Farnsworth mode if the
speed is less than 18 words per minute.
Try the example from the previous section with and without the **-f**
option to see the difference.

In theory the time taken to send a sequence with or without Farnsworth mode
should be the same.
In practice, some round-off errors intervene.
