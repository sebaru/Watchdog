#!/bin/sh
svn update
from=`svnversion -n -c prod | awk -F: '{print $2}'`
dest=`svnversion -n trunk`
Changes=`svn log ^/trunk -r $from:$dest -v | egrep 'FCT:|COR:' `
echo -e $Changes
svn merge https://svn.abls-habitat.fr/repo/Watchdog/trunk prod
svn ci -m "Merging changes from PROD Rev $from to TRUNK Rev $dest -> $Changes"
