#!/bin/python

import redis

class GeoStrictRedis(redis.StrictRedis):
    def geoadd(self, key, longtitude, latitude, member):
        return self.execute_command(
                'geoadd {} {} {} {}'.format(key, longtitude, latitude, member))

    def geopos(self, key, member):
        return self.execute_command(
               'geopos {} {}'.format(key, member))

    def georadius(self, key, longtitude, latitude, radius, unit):
        return self.execute_command(
                'georadius {} {} {} {} {}'.format(
                 key, longtitude, latitude, radius, unit))

if __name__ == '__main__':
    print 'Testing geostrictredis'
    redis = GeoStrictRedis(host='localhost', port=6379, db=0)
    import random
    key='test_{}'.format(random.randint(0, 10**9))
    print redis.geoadd(key, 20, 30, 't1')
    print redis.geopos(key, 't1')
    print redis.georadius(key, 21, 30, 100, 'km')
    redis.delete(key)
