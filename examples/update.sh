#!/bin/bash
if [ "$1" = "-h" ]; then
  echo "Usage: ./update.sh 9539 20 30"
  echo "Update the coordinates of point with id 9539 ti x=20,y=30."
else
  curl -H "Content-Type: application/json" -X POST -d '{"id":'$1',"x":'$2',"y":'$3'}' http://geo.v8boinc.fer.hr/update
fi
