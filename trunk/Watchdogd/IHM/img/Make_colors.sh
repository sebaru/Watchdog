#!/bin/sh

        for SRC in $( ls *_source.svg );
        do
            FILENAME=$(basename $SRC "_source.svg")
            echo Parsing $SRC into ${FILENAME}_colors.svg
            sed s/c8c8c8/00ff00/g $SRC > ${FILENAME}_green.svg
            sed s/c8c8c8/ff0000/g $SRC > ${FILENAME}_red.svg
            sed s/c8c8c8/0000ff/g $SRC > ${FILENAME}_blue.svg
            sed s/c8c8c8/ffff00/g $SRC > ${FILENAME}_yellow.svg
            sed s/c8c8c8/ffbe00/g $SRC > ${FILENAME}_orange.svg
            sed s/c8c8c8/006400/g $SRC > ${FILENAME}_darkgreen.svg
            sed s/c8c8c8/000000/g $SRC > ${FILENAME}_black.svg
            sed s/c8c8c8/ffffff/g $SRC > ${FILENAME}_white.svg
            sed s/c8c8c8/7b7b7b/g $SRC > ${FILENAME}_gray.svg
            sed s/c8c8c8/add8e6/g $SRC > ${FILENAME}_lightblue.svg
	    svn add ${FILENAME}_*.svg
    done
