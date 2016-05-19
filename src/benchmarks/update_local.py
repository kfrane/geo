#!/usr/bin/python
import add_to_path
add_to_path.add()

import measure_utils
import random

def setup_client(host, port, key_prefix):
    from redis import StrictRedis
    from simple_geo_client import SimpleGeoClient

    redis_client = StrictRedis(host=host, port=port, db=0)
    return SimpleGeoClient(redis_client, key_prefix)

def read_input(in_file, randomize_ids):
    ret = []
    for line in in_file:
        car_id, date_time, x, y = line.strip().split(',')
        if randomize_ids:
            car_id = random.randrange(10**9)
        ret.append((car_id, x, y))
    return ret

@measure_utils.measure
def insert_data(geo_client, all_data):
    counter=measure_utils.Counter()
    for car_id, x, y in all_data:
        geo_client.update(car_id, float(x), float(y))
        counter.add_request()
    print 'Query throughput was {}/s'.format(counter.rate())


if __name__ == '__main__':
    USAGE = './update_local -p REDIS_PORT -h redis_host -k key_prefix -i\n' + \
            '-i is meant to keep use ids from input (default is to randomize)'
    import sys
    import getopt

    port=6379
    host='localhost'
    key_prefix='bench'
    randomize_ids = True
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'p:h:k:i')
    except getopt.GetoptError as e:
        print e
        print USAGE
        sys.exit(1)
    for opt, arg in opts:
        if opt == '-p':
            port = int(arg)
        elif opt == '-h':
            host = arg
        elif opt == '-k':
            key_prefix = arg
        elif opt == '-i':
            randomize_ids = False
        else:
            print 'Unknown option'
            sys.exit(1)

    geo_client = setup_client(host, port, key_prefix)
    test_data = read_input(sys.stdin, randomize_ids)
    insert_data(geo_client, test_data)
