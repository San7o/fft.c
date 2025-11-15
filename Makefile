# SPDX-License-Identifier: MIT
# Author:  Giovanni Santini
# Mail:    giovanni.santini@proton.me
# License: MIT

#
# Compiler flags
#
CFLAGS      = -Wall -Werror -Wextra -Wpedantic -std=c99
DEBUG_FLAGS = -ggdb
LDFLAGS     = -lm -lSDL3
CC?         = gcc

#
# Project files
#
OUT_NAME = fft
OBJ      = miniaudio_impl.o\
           example.o

#
# Commands
#
all: $(OUT_NAME)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(OUT_NAME)

run: $(OUT_NAME)
	chmod +x $(OUT_NAME)
	./$(OUT_NAME)

clean:
	rm -f $(OBJ)

distclean:
	rm -f $(OUT_NAME)

$(OUT_NAME): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(CFLAGS) -o $(OUT_NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
