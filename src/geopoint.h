#pragma once

#include <string>
#include <cinttypes>
#include <cassert>
#include <cstdio>
#include <utility>
#include <vector>

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
    sscanf(hash, "%" SCNu64, &hash_bits);
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

  static std::string get_prefix(uint64_t hash, int split_level) {
    int prefix_end = HASH_BITS - 2*split_level;
    std::string prefix;
    for (int i = HASH_BITS-1; i >= prefix_end; i--) {
      if ((1LL<<i)&hash) {
        prefix += "1";
      } else {
        prefix += "0";
      }
    }
    return prefix;
  }

  static Range to_range(const GeoHashBits& hash);

  /**
   * Converts each geohash representing an area to a HASH_BITS range.
   * It also merges consecutive ranges.
   * Returnes vector of Ranges.
   */
  static std::vector<Range> merge_geohashes(
      const std::vector<GeoHashBits>& geohashes);

  /** Return Ranges where each hash in Range has the same prefix of len
   * 2*split_level.
   */
  static void to_ranges(
      std::vector<Range>& ret, const Range& range, int split_level);

  double distance(double lon1d, double lat1d) const;
  void bound_radius(
                    double radius,
                    double& min_lon,
                    double& max_lon,
                    double& min_lat,
                    double& max_lat) const;

  double getLongtitude() const { return lon_; }
  double getLatitude() const { return lat_; }
  std::string getId() const { return id_; }

  static const int HASH_STEP = 26;
  static const int HASH_BITS = HASH_STEP * 2; // In total 52 bits
  static const GeoHashRange lon_range;
  static const GeoHashRange lat_range;

private:
  static double rad_deg(double radians) {
    return radians / DEG_TO_RAD;
  }

  static double deg_rad(double deg) {
    return deg * DEG_TO_RAD;
  }

  double lon_, lat_;
  std::string id_;

  // @brief The usual PI/180 constant
  constexpr static double DEG_TO_RAD = 0.017453292519943295769236907684886;
  // @brief Earth's quatratic mean radius for WGS-84
  constexpr static double EARTH_RADIUS_IN_METERS = 6372797.560856;
};
