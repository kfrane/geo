#!/usr/bin/python
"""WSGI server example"""
from __future__ import print_function
from gevent.pywsgi import WSGIServer
import json
import urlparse

class GeoApplication:
    def __init__(self, geo_client):
        self.geo_client = geo_client

    def error_response(self, start_response):
        start_response('400 Bad Request', [('Content-Type', 'text/plain')])

    def application(self, env, start_response):
        path = env['PATH_INFO']
        data = env['wsgi.input'].readline()
        # print (path)
        # print (data)
        args = urlparse.parse_qs(data)

        if path == '/':
            start_response('200 OK', [('Content-Type', 'text/plain')])
            return [b"hello world"]
        elif path == '/update':
            try:
                query_obj = json.loads(data)
                x, y = float(query_obj['x']), float(query_obj['y'])
                car_id = str(query_obj['id'])
                self.geo_client.update(car_id, x, y)
            except (ValueError, KeyError) as e:
                print ('error during update', str(e))
                self.error_response(start_response)
                return [str(e)]
            start_response('200 OK', [('Content-Type', 'text/plain')])
            return [str(query_obj)]
        elif path == '/query_rectangle':
            try:
                query_obj = json.loads(data)
                x_min, y_min = map(float, query_obj[0])
                x_max, y_max = map(float, query_obj[1])
                query_res = self.geo_client.query_rect(
                        x_min, y_min, x_max, y_max)
            except (ValueError, KeyError) as e:
                self.error_response(start_response)
                return [str(e)]
            start_response('200 OK', [('Content-Type', 'text/plain')])
            return [str(query_res)]
        elif path == '/query_polygon':
            try:
                query_obj = json.loads(data)
                vertices = [(float(v[0]), float(v[1])) for v in query_obj]
            except (ValueError, KeyError) as e:
                self.error_response(start_response)
                return [str(e)]
            start_response('200 OK', [('Content-Type', 'text/plain')])
            return [str(vertices)]
        elif path == '/query_knn':
            try:
                query_obj = json.loads(data)
                x, y = float(query_obj['x']), float(query_obj['y'])
                k = int(query_obj['k'])
            except (ValueError, KeyError) as e:
                self.error_response(start_response)
                return [str(e)]
            start_response('200 OK', [('Content-Type', 'text/plain')])
            return [str(k)]
        else:
            start_response('404 Not Found', [('Content-Type', 'text/plain')])
            return [b'Not Found']

if __name__ == '__main__':
    from redis import StrictRedis
    from simple_geo_client import SimpleGeoClient

    redis_client = StrictRedis(host='localhost', port=6379, db=0)
    geo_client = SimpleGeoClient(redis_client, 'car')
    app = GeoApplication(geo_client)

    print('Serving on 8088...')
    WSGIServer(('', 8088), app.application, log=None).serve_forever()
