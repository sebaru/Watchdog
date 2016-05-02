#!/bin/sh
svn update
svn merge https://svn.connect4all.fr/root/Watchdog/trunk .
svn ci -m "Merge depuis le trunk"
