#!/bin/sh
git pull
VERSION=`git describe | cut -d - -f 1-2 | sed -e 's/v//g'`
echo "Starting build of version $VERSION ok ?"
read
podman image rm abls-agent:$VERSION
podman build -t abls-agent:$VERSION .
podman tag abls-agent:$VERSION abls-agent:latest
echo "Ready to push to docker.io ?"
read
echo "login to docker.io"
podman login -u sebaru docker.io
podman push abls-agent:$VERSION docker.io/sebaru/abls-agent:$VERSION
podman push abls-agent:latest docker.io/sebaru/abls-agent:latest
