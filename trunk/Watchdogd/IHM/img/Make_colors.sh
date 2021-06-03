#!/bin/sh

        for SRC in $( ls *_source.svg );
        do
            FILENAME=$(basename $SRC "_source.svg")
            echo Parsing $SRC into ${FILENAME}_colors.svg
            sed s/c8c8c8/00ff00/g $SRC > ${FILENAME}_vert.svg
            sed s/c8c8c8/ff0000/g $SRC > ${FILENAME}_rouge.svg
            sed s/c8c8c8/0000ff/g $SRC > ${FILENAME}_bleu.svg
            sed s/c8c8c8/ffff00/g $SRC > ${FILENAME}_jaune.svg
            sed s/c8c8c8/ffbe00/g $SRC > ${FILENAME}_orange.svg
            sed s/c8c8c8/006400/g $SRC > ${FILENAME}_kaki.svg
            sed s/c8c8c8/000000/g $SRC > ${FILENAME}_noir.svg
            sed s/c8c8c8/ffffff/g $SRC > ${FILENAME}_blanc.svg
            sed s/c8c8c8/7b7b7b/g $SRC > ${FILENAME}_gris.svg
            svn add ${FILENAME}_*.svg
    done
