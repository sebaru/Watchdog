#!/bin/sh
export XDG_RUNTIME_DIR=/run/user/`id -u`
gtts-cli -l $1 "$2" -o temp.mp3
mpg123 temp.mp3
#mpg123 -w temp.wav temp.mp3
#aplay -D $3 temp.wav
