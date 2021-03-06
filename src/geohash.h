/*
 *Copyright (c) 2013-2014, yinqiwen <yinqiwen@gmail.com>
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of Redis nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 *BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GEOHASH_H_
#define GEOHASH_H_

#include <stdint.h>
#include <vector>


typedef enum
{
  GEOHASH_NORTH = 0,
  GEOHASH_EAST,
  GEOHASH_WEST,
  GEOHASH_SOUTH,
  GEOHASH_SOUTH_WEST,
  GEOHASH_SOUTH_EAST,
  GEOHASH_NORT_WEST,
  GEOHASH_NORT_EAST,
  GEOHASH_DIR_COUNT // Only used as enum size
} GeoDirection;

struct GeoHashBits {
  uint64_t bits;
  uint8_t step;

  GeoHashBits() : bits(0), step(0) {}
  GeoHashBits(uint64_t bits_, uint8_t step_) : bits(bits_), step(step_) {}
};

struct GeoHashRange {
  double max;
  double min;

  GeoHashRange() {}
  GeoHashRange(double max_, double min_) : max(max_), min(min_) {}

  double get() { return (max+min)/2.0; }

  bool intersects(const GeoHashRange &other) {
    return other.max > min && other.min < max;
  }
};

struct GeoHashArea {
  GeoHashBits hash;
  GeoHashRange latitude;
  GeoHashRange longitude;

  GeoHashArea() {}
  bool intersects(const GeoHashArea &other) {
    return latitude.intersects(other.latitude) &&
           longitude.intersects(other.longitude);
  }
};

typedef struct
{
  GeoHashBits north;
  GeoHashBits east;
  GeoHashBits west;
  GeoHashBits south;
  GeoHashBits north_east;
  GeoHashBits south_east;
  GeoHashBits north_west;
  GeoHashBits south_west;
} GeoHashNeighbors;

/*
 * 0:success
 * -1:failed
 */
int geohash_encode(GeoHashRange lat_range, GeoHashRange lon_range, double latitude, double longitude, uint8_t step, GeoHashBits* hash);
int geohash_decode(GeoHashRange lat_range, GeoHashRange lon_range, GeoHashBits hash, GeoHashArea* area);

/*
 * Fast encode/decode version, more magic in implementation.
 * 0 bit is used for <= middle value.
 * 1 bit otherwise.
 *
 * Highest bit is the longtitude bit.
 */
int geohash_fast_encode(const GeoHashRange& lat_range, const GeoHashRange& lon_range, double latitude, double longitude, uint8_t step, GeoHashBits* hash);
int geohash_fast_decode(const GeoHashRange& lat_range, const GeoHashRange& lon_range, GeoHashBits hash, GeoHashArea* area);

int geohash_get_neighbors(GeoHashBits hash, GeoHashNeighbors* neighbors);
std::vector<GeoHashBits> geohash_get_neighbors(GeoHashBits hash);
int geohash_get_neighbor(GeoHashBits hash, GeoDirection direction, GeoHashBits* neighbor);
std::vector <GeoHashBits> cover_rectangle(
    const GeoHashRange& lat_range, const GeoHashRange& lon_range,
    double lat_min, double lat_max, double lon_min, double lon_max);

/** Go level deeper (split area in 4). */
GeoHashBits geohash_next_leftbottom(GeoHashBits bits);
GeoHashBits geohash_next_rightbottom(GeoHashBits bits);
GeoHashBits geohash_next_lefttop(GeoHashBits bits);
GeoHashBits geohash_next_righttop(GeoHashBits bits);


#endif /* GEOHASH_H_ */
