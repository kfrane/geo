from geohash import decode_uint64, encode_uint64

""" Undocumented stuff from geohash library
    Even indicies are latitude(y).
    Odd indicies are longtitude(x).
    1 bit is >= 0, and 0 bit is < 0."""

""" We use 64 bit versions of functions on Geohash library, but we store 52
    highest bits in Redis."""
BIT_COUNT = 52

def score(lng, lat, precision=BIT_COUNT):
    h = encode_uint64(lat, lng) >> (64-BIT_COUNT)
    return (h >> (BIT_COUNT-precision)) << (BIT_COUNT-precision)

def next_score(score, precision=BIT_COUNT):
    last_n = BIT_COUNT - precision
    return ((score >> last_n) + 1) << last_n

""" Assumes geohash is BIT_COUNT len.
    Returns (longtitude, latitude)."""
def decode(geohash):
    geohash <<= (64-BIT_COUNT)
    lat, lng = decode_uint64(geohash)
    return (lng, lat)

def cover_rect(x_min, y_min, x_max, y_max):
    """ Return up to 4 geohash ranges that completely cover the given rectangle.
        TODO: There is room for improvement because it is probably possible to
        choose even smaller rectangles."""
    w = x_max-x_min
    h = y_max-y_min
    level_w, level_h = 360, 180
    hash_len = 0
    for _ in xrange(0, BIT_COUNT, 2):
        curr_w = level_w / 2.0
        curr_h = level_h / 2.0
        if curr_w < w or curr_h < h:
            break
        level_w, level_h = curr_w, curr_h
        hash_len += 2
    ret = set()
    vertices = [(x_min, y_min), (x_max, y_min), (x_min, y_max), (x_max, y_max)]
    for x, y in vertices:
        curr_score = score(x, y, hash_len)
        ret.add((curr_score, next_score(curr_score, hash_len)))
    return list(ret)

if __name__ == '__main__':
    rects = cover_rect(10, -40, 12, -39)
    rects.sort()
    for r in rects:
        print format(r[0], 'b')
        print format(r[1]+1, 'b')
        print
