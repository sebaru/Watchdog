#!/bin/sh
svn update
svn merge https://svn.abls-habitat.fr/repo/Watchdog/trunk .
svn ci -m "Merge depuis le trunk"
