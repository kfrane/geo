#include <cmath>
#include <algorithm>
#include "geopoint.h"

const GeoHashRange GeoPoint::lon_range = GeoHashRange(180, -180);
const GeoHashRange GeoPoint::lat_range = GeoHashRange(90,-90);

GeoPoint::Range GeoPoint::to_range(const GeoHashBits& hash) {
  int missing_bits = 2*(GeoPoint::HASH_STEP - hash.step);
  uint64_t start = hash.bits << missing_bits;
  uint64_t end = ((hash.bits+1) << missing_bits)-1;
  return {start, end};
}

std::vector<GeoPoint::Range> GeoPoint::merge_geohashes(
      const std::vector<GeoHashBits>& geohashes) {
  std::vector<GeoPoint::Range> ranges;
  for (const GeoHashBits& geohash : geohashes) {
    ranges.push_back(to_range(geohash));
  }
  std::sort(ranges.begin(), ranges.end());

  std::vector<GeoPoint::Range> merged_ranges;
  GeoPoint::Range last_range = ranges[0];
  for (size_t range_index = 1; range_index < ranges.size(); range_index++) {
    GeoPoint::Range current_range = ranges[range_index];
    assert (current_range.first > last_range.second);
    if (last_range.second+1 == current_range.first) {
      // Expand last range
      last_range.second = current_range.second;
    } else {
      merged_ranges.push_back(last_range);
      last_range = current_range;
    }
  }
  merged_ranges.push_back(last_range);
  /*
  if (merged_ranges.size() < ranges.size()) {
    std::cout << merged_ranges.size() << " " << ranges.size() << std::endl;
    for (GeoPoint::Range range : ranges) {
      std::cout << "(" << range.first << " " << range.second << ") ";
    }
    std::cout << std::endl;
    for (GeoPoint::Range range : merged_ranges) {
      std::cout << "(" << range.first << " " << range.second << ") ";
    }
    std::cout << std::endl;
  }
  */
  return merged_ranges;
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

void GeoPoint::bound_radius(double radius,
                            double& min_lon,
                            double& max_lon,
                            double& min_lat,
                            double& max_lat) const {
  double lat_rad = deg_rad(lat_);
  double lon_rad = deg_rad(lon_);
  if (radius > GeoPoint::EARTH_RADIUS_IN_METERS) {
    radius = GeoPoint::EARTH_RADIUS_IN_METERS;
  }
  double distance = radius / GeoPoint::EARTH_RADIUS_IN_METERS;
  min_lat = lat_rad - distance;
  max_lat = lat_rad + distance;

  /* Note: we're being lazy and not accounting for coordinates near poles */
  double difference_longitude = asin(sin(distance) / cos(lat_rad));
  min_lon = lon_rad - difference_longitude;
  max_lon = lon_rad + difference_longitude;

  min_lon = rad_deg(min_lon);
  min_lat = rad_deg(min_lat);
  max_lon = rad_deg(max_lon);
  max_lat = rad_deg(max_lat);
}


/* Calculate distance using haversin great circle distance formula. */
double GeoPoint::distance(double lon1d, double lat1d) const {
  double lat1r, lon1r, lat2r, lon2r, u, v;
  lat1r = deg_rad(lat1d);
  lon1r = deg_rad(lon1d);
  lat2r = deg_rad(lat_);
  lon2r = deg_rad(lon_);
  u = sin((lat2r - lat1r) / 2);
  v = sin((lon2r - lon1r) / 2);
  return 2.0 * GeoPoint::EARTH_RADIUS_IN_METERS *
         asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}
