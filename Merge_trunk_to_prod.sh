#!/bin/sh
svn update
from=`svnversion -n -c prod | awk -F: '{print $2}'`
dest=`svnversion -n trunk`
svn merge https://svn.abls-habitat.fr/repo/Watchdog/trunk prod
Changes=`svn log ^/trunk -r $from:$dest -v | egrep 'FCT:|COR:' `
svn ci -m "Merging changes from PROD Rev $from to TRUNK Rev $dest -> $Changes"
