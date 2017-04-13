#!/bin/sh
svn update
from=`svnversion -n -c | awk -F: '{print $2}'`
dest=`svnversion -n`
svn merge https://svn.abls-habitat.fr/repo/Watchdog/trunk .
Changes=`svn log ^/trunk -r $from:$dest -v`
svn ci -m $Changes
