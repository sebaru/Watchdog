#!/bin/sh
find . -name "*.mp3" -exec ./migrate_to_wav.sh {} \;
