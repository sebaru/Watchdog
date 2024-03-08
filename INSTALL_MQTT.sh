#!/bin/sh
cp mosquitto.conf /etc/mosquitto/
systemctl enable --now mosquitto
