#
# Copyright (c) 2020-21, Kalopa Robotics Limited.  All rights
# reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
# NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
# SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ABSTRACT
# The build system for libmorse.a.
#
LIBTOOL=ar
#CFLAGS= -Wall -O -DSDL2
CFLAGS= -Wall -O -DALSA

#SND_SRC=sdl2.c
#SND_INC=`sdl2-config --cflags`
#SND_LIB=`sdl2-config --libs`
SND_SRC=alsa.c
SND_INC=
SND_LIB=-lasound

SRCS=	init.c morse.c audio.c params.c $(SND_SRC)
OBJS=	$(SRCS:.c=.o)
LIB=	libmorse.a

all:	$(LIB) morse_play

install: $(LIB) morse_play
	install -c $(LIB) /usr/local/lib
	install -c morse_play  /usr/local/bin

clean:
	rm -f morse_play $(LIB) $(OBJS) main.o tags

tags:	$(SRCS)
	ctags $(SRCS)

$(LIB):	$(OBJS)
	$(AR) r $@ $?

morse_play: main.o $(LIB)
	$(CC) -o morse_play main.o -L. -lmorse $(SND_LIB) -lm

sdl2.o:	sdl2.c
	$(CC) $(CFLAGS) $(SND_INC) -c -o $@ sdl2.c

$(OBJS): libmorse.h
