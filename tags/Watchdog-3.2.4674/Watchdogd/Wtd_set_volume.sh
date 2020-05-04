#!/bin/sh
pid=$1
volume=$2

client=$(echo $(pactl list sink-inputs | grep -e "application.process.id = \"$pid\"" -e "#") | grep -o "#[0-9]*" | grep -o "[0-9]*")
echo "setting volume for pid $pid (client $client)"

