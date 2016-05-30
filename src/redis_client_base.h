#pragma once

#include <string>
#include <functional> // maybe use boost version
#include <vector>

#include "geopoint.h"

class RedisClientBase {
public:
  typedef std::function<void(bool)> updateCallbackFn;
  typedef std::function<void(bool, std::vector <GeoPoint> *)> queryCallbackFn;

  virtual void update(
      const std::string& point_id,
      double lon,
      double lat,
      updateCallbackFn callbackFn) = 0;

  virtual void rectangle_query(
      double lon_min, double lon_max, double lat_min, double lat_max,
      queryCallbackFn callbackFn) = 0;
};

