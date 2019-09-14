#!/bin/sh
curl -H "Content-Type: application/json" -d "{'mode':'GET','type':'$2','tech_id':'$3','acronyme':'$4','num':'-1'}" "http://$1:5560/memory"

