#!/bin/bash
if [ "$1" = "-h" ]; then
  echo "Usage: ./query_rectangle.sh '[[15,-30], [20, -28.8]]'"
  echo "Returns a list of points inside rectangle defined by (x_min,y_min), (x_max, y_max)"
else
    curl -w "\nHttp status: %{http_code}\n" -H "Content-Type: application/json" -X POST -d "$@" http://geo.v8boinc.fer.hr/query_rectangle
fi
