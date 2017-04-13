#!/bin/sh
svn update
svn merge https://svn.abls-habitat.fr/repo/Watchdog/trunk .
from=`svnversion -n -c | awk -F: '{print $2}'`
dest=`svnversion -n`
svn ci -m `svn log ^/trunk -r $from:$dest -v`
