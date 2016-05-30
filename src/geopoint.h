#pragma once

#include <string>
#include <inttypes.h>
#include <cassert>
#include <cstdio>
#include <utility>

#include "geohash.h"

class GeoPoint {
public:
  typedef std::pair<uint64_t, uint64_t> Range;

  GeoPoint(double lon, double lat, const std::string& id)
    : lon_(lon), lat_(lat), id_(id) {}

  GeoPoint(double lon, double lat, const char* id)
    : lon_(lon), lat_(lat), id_(id) {}


  static GeoPoint from_hash(const char* id, const char* hash) {
    uint64_t hash_bits;
    sscanf(hash, "%lld", &hash_bits);
    GeoHashBits geohash(hash_bits, HASH_STEP);
    GeoHashArea area;
    geohash_fast_decode(lat_range, lon_range, geohash, &area);
    return GeoPoint(area.longitude.get(), area.latitude.get(), id);
  }

  static uint64_t encode(double lon, double lat) {
    GeoHashBits hash;
    assert(geohash_fast_encode(lat_range, lon_range, lat, lon, HASH_STEP, &hash) == 0);
    return hash.bits;
  }

  static Range to_range(const GeoHashBits& hash);

  double getLongtitude() const { return lon_; }
  double getLatitude() const { return lat_; }
  std::string getId() const { return id_; }

  static const int HASH_STEP = 26;
  static const int HASH_BITS = HASH_STEP * 2; // In total 52 bits
  static const GeoHashRange lon_range;
  static const GeoHashRange lat_range;

private:
  double lon_, lat_;
  std::string id_;
};
