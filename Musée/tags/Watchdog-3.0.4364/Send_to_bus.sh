#!/bin/sh
curl -H "Content-Type: application/json" -d "{'host':'$2','thread':'$3','tag':'$4','text':'$5'}" "http://$1:5560/bus"
