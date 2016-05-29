#pragma once

#include <string>
#include <inttypes.h>
#include <cassert>
#include <cstdio>

#include "geohash.h"

class GeoPoint {
public:
  double lon_, lat_;
  std::string id_;

  GeoPoint(double lon, double lat, const std::string& id)
    : lon_(lon), lat_(lat), id_(id) {}

  GeoPoint(double lon, double lat, const char* id)
    : lon_(lon), lat_(lat), id_(id) {}


  static GeoPoint from_hash(const char* id, const char* hash) {
    uint64_t hash_bits;
    sscanf(hash, "%lld", &hash_bits);
    GeoHashBits geohash(hash_bits, HASH_BITS);
    GeoHashArea area;
    geohash_fast_decode(lat_range_, lon_range_, geohash, &area);
    return GeoPoint(area.longitude.get(), area.latitude.get(), id);
  }

  static uint64_t encode(double lon, double lat) {
    GeoHashBits hash;
    assert(geohash_fast_encode(lat_range_, lon_range_, lat, lon, HASH_BITS, &hash) == 0);
    return hash.bits;
  }

  double getLongtitude() { return lon_; }
  double getLatitude() { return lat_; }
  std::string getId() { return id_; }

private:
  static const int HASH_BITS = 26; // In total 52 bits
  static const GeoHashRange lon_range_;
  static const GeoHashRange lat_range_;
};
