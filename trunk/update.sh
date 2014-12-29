#! /bin/sh
rm RevisionNumber.txt
svn update
svnversion -n | sed -e "s/M//g" > RevisionNumber.txt
