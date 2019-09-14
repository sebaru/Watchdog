#!/bin/sh
find /var/WatchdogHome -name "CAM*" -mtime +40 -delete

