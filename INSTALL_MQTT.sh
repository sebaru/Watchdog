#!/bin/sh
dnf install mosquitto
cp mosquitto.conf /etc/mosquitto/
systemctl enable mosquitto
systemctl restart mosquitto
