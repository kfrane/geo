from geo_client import GeoClient
import geohash_utils

def car_member(car_id, x, y):
    return 'car:{} {} {}'.format(car_id, x, y)

def parse_car_member(member):
    member_parts = member.split()
    return (member_parts[0], float(member_parts[1]), float(member_parts[2]))

class SimpleGeoClient(GeoClient):
    def __init__(self, redis, key):
        self.redis = redis
        self.id_key = key
        self.geo_key = '{}:geo'.format(key)

    def update(self, car_id, x, y):
        car_id_key = '{}:{}'.format(self.id_key, car_id)
        old_position = self.redis.getset(car_id_key, '{} {}'.format(x, y))
        old_x, old_y = old_position.split()

        # TODO: These 2 can go in parallel.
        self.redis.zrem(
                self.geo_key,
                car_member(car_id, old_x, old_y))
        self.redis.zadd(
                self.geo_key,
                geohash_utils.score(x,y),
                car_member(car_id, x, y))

    def prefix_find(self, key, score, prefix_len, limit=None):
        max_score = geohash_utils.next_score(score, prefix_len)-1
        if limit is None:
            return self.redis.zrangebyscore(
                    key, score, max_score)
        else:
            return self.redis.zrangebyscore(
                    key, score, max_score, 0, limit)

    def range_find(self, key, score_start, score_end, limit=None):
        if limit is None:
            return self.redis.zrangebyscore(
                    key, score_start, score_end)
        else:
            return self.redis.zrangebyscore(
                    key, score_start, score_end, 0, limit)

    """ Return list of car ids that are inside a given rectangle. """
    def query_rect(self, x_min, y_min, x_max, y_max):
        hashes = geohash_utils.cover_rect(x_min, y_min, x_max, y_max)
        ret = set()
        for hash_range in hashes:
            car_members = self.range_find(
                    self.geo_key, hash_range[0], hash_range[1])
            for car_member in car_members:
                car_id, x, y = parse_car_member(car_member)
                if x_min <= x <= x_max and y_min <= y <= y_max:
                    ret.add((car_id, x, y))
        return list(ret)


if __name__ == '__main__':
    from redis import StrictRedis
    redis = StrictRedis(host='localhost', port=6379, db=0)
    geo_client = SimpleGeoClient(redis, 'car')
    geo_client.update(1, 30, -40)
    geo_client.update(2, 31, -40)
    geo_client.update(3, 31, -41)
    h = geohash_utils.score(30, -40, 50)
    print geo_client.prefix_find('car_geo', h, 50)
    print geo_client.prefix_find('car_geo', h, 10)
    print geo_client.query_rect(30, -40, 31, -39)
    print geo_client.query_rect(30, -40, 30.5, -39)
    print geo_client.query_rect(30.5, -40, 31, -39)
    print geo_client.query_rect(30.5, -42, 31, -39)
