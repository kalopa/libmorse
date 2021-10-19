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
CFLAGS= -Wall -O

SRCS=	init.c morse.c audio.c params.c
OBJS=	$(SRCS:.c=.o)
LIB=	libmorse.a

all:	$(LIB) morse_send

install: $(LIB) morse_send
	$(INSTALL) $(LIB) /usr/local/lib
	$(INSTALL) morse_send  /usr/local/bin

clean:
	rm -f morse_send $(LIB) $(OBJS) main.o tags

tags:	$(SRCS)
	ctags $(SRCS)

$(LIB):	$(OBJS)
	$(AR) r $@ $?

morse_send: main.o $(LIB)
	$(CC) -o morse_send main.o -L. -lmorse -lasound -lm

$(OBJS): libmorse.h
