#!/bin/sh
            FILENAME=$(basename $1 ".svg")
            echo Parsing $1 into ${FILENAME}.png
	    convert $1 -thumbnail 128x128 ${FILENAME}.png
            svn add ${FILENAME}.png
