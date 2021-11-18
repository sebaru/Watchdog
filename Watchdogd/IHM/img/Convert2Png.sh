#!/bin/sh
for SRC in $( ls *.svg );
        do
            FILENAME=$(basename $SRC ".svg")
            echo Parsing ${SRC} into ${FILENAME}.png
            nice -10 ionice -c 3 sh -c 'inkscape '${SRC}' -o '${FILENAME}'.png;' &
       done

for job in `jobs -p`
        do
           wait $job
        done
git add *.png
