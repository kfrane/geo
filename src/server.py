#!/usr/bin/python
"""WSGI server example"""
from __future__ import print_function
from gevent.pywsgi import WSGIServer
import json
import urlparse

def error_response(start_response):
    start_response('400 Bad Request', [('Content-Type', 'text/plain')])

def application(env, start_response):
    path = env['PATH_INFO']
    data = env['wsgi.input'].readline()
    print (path)
    print (data)
    args = urlparse.parse_qs(data)

    if path == '/':
        start_response('200 OK', [('Content-Type', 'text/plain')])
        return [b"hello world"]
    elif path == '/update':
        try:
            query_obj = json.loads(data)
            x, y = float(query_obj['x']), float(query_obj['y'])
            car_id = str(query_obj['id'])
        except (ValueError, KeyError) as e:
            error_response(start_response)
            return [str(e)]
        start_response('200 OK', [('Content-Type', 'text/plain')])
        return [str(query_obj)]
    elif path == '/query_polygon':
        try:
            query_obj = json.loads(data)
            vertices = [(float(v[0]), float(v[1])) for v in query_obj]
        except (ValueError, KeyError) as e:
            error_response(start_response)
            return [str(e)]
        start_response('200 OK', [('Content-Type', 'text/plain')])
        return [str(vertices)]
    elif path == '/query_knn':
        try:
            query_obj = json.loads(data)
            x, y = float(query_obj['x']), float(query_obj['y'])
            k = int(query_obj['k'])
        except (ValueError, KeyError) as e:
            error_response(start_response)
            return [str(e)]
        start_response('200 OK', [('Content-Type', 'text/plain')])
        return [str(k)]
    else:
        start_response('404 Not Found', [('Content-Type', 'text/plain')])
        return [b'Not Found']

if __name__ == '__main__':
    print('Serving on 8088...')
    WSGIServer(('', 8088), application).serve_forever()
