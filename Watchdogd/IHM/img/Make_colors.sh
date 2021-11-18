#!/bin/sh

        for SRC in $( ls *_source.svg );
        do
            FILENAME=$(basename $SRC "_source.svg")
            echo Parsing $SRC into ${FILENAME}_colors.svg
            nice -10 sed s/c8c8c8/00ff00/g $SRC > ${FILENAME}_green.svg
            nice -10 sed s/c8c8c8/ff0000/g $SRC > ${FILENAME}_red.svg
            nice -10 sed s/c8c8c8/0000ff/g $SRC > ${FILENAME}_blue.svg
            nice -10 sed s/c8c8c8/ffff00/g $SRC > ${FILENAME}_yellow.svg
            nice -10 sed s/c8c8c8/ffbe00/g $SRC > ${FILENAME}_orange.svg
            nice -10 sed s/c8c8c8/006400/g $SRC > ${FILENAME}_darkgreen.svg
            nice -10 sed s/c8c8c8/000000/g $SRC > ${FILENAME}_black.svg
            nice -10 sed s/c8c8c8/ffffff/g $SRC > ${FILENAME}_white.svg
            nice -10 sed s/c8c8c8/7b7b7b/g $SRC > ${FILENAME}_gray.svg
            nice -10 sed s/c8c8c8/add8e6/g $SRC > ${FILENAME}_lightblue.svg
            nice -10 git add ${FILENAME}_*.svg
        done
