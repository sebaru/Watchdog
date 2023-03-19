#!/bin/sh
git pull
echo "login to docker.io"
podman login docker.io
VERSION=`git describe | cut -d - -f 1-2 | sed -e 's/v//g'`
echo "Starting build of version $VERSION ok ?"
read
podman image rm abls-agent:$VERSION
podman build --no-cache -t abls-agent:$VERSION .
podman tag abls-agent:$VERSION abls-agent:latest
echo "Ready to push to docker.io ?"
read
podman push abls-agent:$VERSION docker.io/sebaru/abls-agent:$VERSION
podman push abls-agent:latest docker.io/sebaru/abls-agent:latest
