#include "geopoint.h"

const GeoHashRange GeoPoint::lon_range = GeoHashRange(180, -180);
const GeoHashRange GeoPoint::lat_range = GeoHashRange(90,-90);

GeoPoint::Range GeoPoint::to_range(const GeoHashBits& hash) {
  int missing_bits = 2*(GeoPoint::HASH_STEP - hash.step);
  uint64_t start = hash.bits << missing_bits;
  uint64_t end = ((hash.bits+1) << missing_bits)-1;
  return {start, end};
}
