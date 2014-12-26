#!/bin/sh
rev=`svnversion -n`
echo $rev > RevisionNumber.txt
echo "Setting Revision number to "$rev
