#include "geopoint.h"

const GeoHashRange GeoPoint::lon_range = GeoHashRange(180, -180);
const GeoHashRange GeoPoint::lat_range = GeoHashRange(90,-90);

GeoPoint::Range GeoPoint::to_range(const GeoHashBits& hash) {
  int missing_bits = 2*(GeoPoint::HASH_STEP - hash.step);
  uint64_t start = hash.bits << missing_bits;
  uint64_t end = ((hash.bits+1) << missing_bits)-1;
  return {start, end};
}


void GeoPoint::to_ranges(std::vector<GeoPoint::Range>& ret,
                const GeoPoint::Range& range,
                int split_level) {
  int suffix_len = GeoPoint::HASH_BITS-2*split_level;
  uint64_t prefix_start = range.first >> suffix_len;
  uint64_t prefix_end = range.second >> suffix_len;
  if (prefix_start == prefix_end) {
    ret.push_back(range);
    return;
  }

  auto next = [suffix_len] (uint64_t hash) -> uint64_t {
    return ((hash>>suffix_len)+1) << suffix_len;
  };

  ret.push_back({range.first, next(range.first)-1});
  uint64_t range_end;
  for (uint64_t start = next(range.first);;) {
    range_end = next(start)-1;
    if (range.second <= range_end) {
      ret.push_back({start, range.second});
      break;
    } else {
      ret.push_back({start, range_end});
    }
    start = range_end + 1;
  }

  return;
}
