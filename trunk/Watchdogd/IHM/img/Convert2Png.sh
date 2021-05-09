#!/bin/sh
        for SRC in $( ls *.svg );
        do
            FILENAME=$(basename $SRC ".svg")
            echo Parsing $SRC into ${FILENAME}.png
	    convert $SRC -thumbnail 128x128 ${FILENAME}.png
	done
	svn add *.png
