#!/bin/sh
            FILENAME=$(basename $1 ".svg")
            echo Parsing $1 into ${FILENAME}.png
            inkscape $1 -o ${FILENAME}.png
            svn add ${FILENAME}.png
