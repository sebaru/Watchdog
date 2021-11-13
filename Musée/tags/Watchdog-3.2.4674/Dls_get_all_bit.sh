#!/bin/sh
curl -v -H "Content-Type: application/json" -d "{'tech_id':'$2'}" "http://$1:5560/memory"

