#! /bin/sh
svn update
svnversion -n | sed -e "s/M//g" > RevisionNumber.txt
