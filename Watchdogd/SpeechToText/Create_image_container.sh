#!/bin/sh
git pull
VERSION=`git describe | cut -d - -f 1-2 | sed -e 's/v//g'`
IMAGE=abls-speech-to-mqtt
echo "Starting build of $IMAGE version $VERSION ok ?"
read
podman image rm $IMAGE:$VERSION
podman build -t $IMAGE:$VERSION .
podman tag $IMAGE:$VERSION $IMAGE:latest
echo "Ready to push to docker.io ?"
read
echo "login to docker.io"
podman login -u sebaru docker.io
podman push $IMAGE:$VERSION docker.io/sebaru/$IMAGE:$VERSION
podman push $IMAGE:latest docker.io/sebaru/$IMAGE:latest
