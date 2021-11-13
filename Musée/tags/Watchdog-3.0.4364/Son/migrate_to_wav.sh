#!/bin/sh
target=`echo $1 | sed 's/.mp3/.wav/g'`
echo from = $1 target $target
mpg123 -w $target $1
