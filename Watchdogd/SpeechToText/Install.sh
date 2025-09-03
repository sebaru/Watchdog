#!/bin/sh

#Install VOSK Model
rm -rf app/vosk-model
wget -O app/vosk-fr.zip https://alphacephei.com/vosk/models/vosk-model-small-fr-0.22.zip
unzip app/vosk-fr.zip -d app/
mv app/vosk-model-small-fr-0.22 app/vosk-model
rm app/vosk-fr.zip
