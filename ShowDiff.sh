#/bin/sh
svn up
from=`svnversion -n -c prod | awk -F: '{print $2}'`
dest=`svnversion -n trunk`
svn log ^/trunk -r $from:$dest | more
svn diff --old ^/prod --new ^/trunk | more
