#!/bin/bash

if [ "$1" = "-h" ]; then
  echo "Usage: ./query_knn.sh 20 30 3"
  echo "Return list of the 3 closest points from x=20,y=30."
else
  curl -H "Content-Type: application/json" -X POST -d '{"x":'$1',"y":'$2',"k":'$3'}' http://geo.v8boinc.fer.hr/query_knn
fi
