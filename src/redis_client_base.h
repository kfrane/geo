#pragma once

#include <string>
#include <functional> // maybe use boost version
#include <vector>
#include <cmath>

#include "geopoint.h"

class RedisClientBase {
public:
  typedef std::function<void(bool)> updateCallbackFn;
  typedef std::function<void(bool, std::vector <GeoPoint> *)> queryCallbackFn;
  typedef std::function<
    void(bool, std::vector <GeoPoint> *, size_t points_retrieved)>
    radiusCallbackFn;


  virtual void update(
      const std::string& point_id,
      double lon,
      double lat,
      updateCallbackFn callbackFn) = 0;

  virtual void rectangle_query(
      double lon_min, double lon_max, double lat_min, double lat_max,
      queryCallbackFn callbackFn) = 0;

  virtual void radius_query(
      double radius, double lon, double lat, radiusCallbackFn callbackFn) {
    GeoPoint center_point(lon, lat, "");
    double min_lon, max_lon, min_lat, max_lat;
    center_point.bound_radius(radius, min_lon, max_lon, min_lat, max_lat);
    if (!(min_lat >= 38) || !(max_lat <= 43) || !(min_lon >= 114) || !(max_lon <= 120)) {
      std::cout << "lon " << lon << " lat " << lat << " r " << radius << std::endl;
      std::cout << "min_lon " << min_lon << " max lon " << max_lon << " min lat " <<
        min_lat << " max_lat " << max_lat << std::endl;

    }

    assert (min_lat >= 38);
    assert (max_lat <= 43);
    assert (min_lon >= 114);
    assert (max_lon <= 120);
    rectangle_query(min_lon, max_lon, min_lat, max_lat,
      [callbackFn, lon, lat, radius]
      (bool success, std::vector <GeoPoint>* candidates) {
      size_t points_in_rectangle = candidates->size();
    //  std::cout << "candidates size " << candidates->size() << std::endl;
      std::vector <GeoPoint>* ret = new std::vector<GeoPoint>();
      for (const GeoPoint& point : *candidates) {
        if (point.distance(lon, lat) <= radius) {
          ret->push_back(point);
        }
      }
      delete candidates;
      callbackFn(success, ret, points_in_rectangle);
    });
  }

  virtual ~RedisClientBase() {}

private:
};

