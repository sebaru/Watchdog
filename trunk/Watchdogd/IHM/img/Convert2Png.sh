#!/bin/sh
        for SRC in $( ls *.svg );
        do
            FILENAME=$(basename $SRC ".svg")
            echo Parsing $SRC into ${FILENAME}.png
            inkscape $SRC -o ${FILENAME}.png
            svn add ${FILENAME}.png
	done
