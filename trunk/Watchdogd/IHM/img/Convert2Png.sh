#!/bin/sh
            FILENAME=$(basename $1 ".svg")
            echo Parsing $SRC into ${FILENAME}.png
	    inkscape $1 -o ${FILENAME}.png
            svn add ${FILENAME}.png
