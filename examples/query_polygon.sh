#!/bin/bash
if [ "$1" = "-h" ]; then
  echo "Usage: ./query_poligon.sh [[20,-30], [15,40], [50,50]]"
  echo "Returns a list of points inside polygon defined by this list of vertices."
else
    curl -H "Content-Type: application/json" -X POST -d "$@" http://geo.v8boinc.fer.hr/query_polygon
fi
