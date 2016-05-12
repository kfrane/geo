#!/usr/bin/python

import random

all_lines = []

def read_file(filename):
    global all_lines
    with open(filename) as f:
        all_lines+=f.readlines()

def gen_csv():
    random.shuffle(all_lines)
    for line in all_lines:
        car_id, date_time, x, y = line.strip().split(',')
        car_id = random.randrange(10**6)
        print '{{"id":{},"x":{},"y":{}}}'.format(car_id, x, y)

if __name__=='__main__':
    import sys
    for filename in sys.argv[1:]:
        read_file(filename)
    gen_csv()
