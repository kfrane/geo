#pragma once

#include <string>
#include <functional> // maybe use boost version
#include <sstream>
#include <vector>

#include "asynchirediscommand.h"
#include "geohash.h"
#include "geopoint.h"

using RedisCluster::AsyncHiredisCommand;
using RedisCluster::Cluster;

class RedisClient {
public:
  typedef std::function<void(bool)> updateCallbackFn;
  typedef std::function<void(bool, std::vector <GeoPoint> *)> queryCallbackFn;

  RedisClient(
      typename Cluster<redisAsyncContext>::ptr_t cluster,
      std::string prefix) :
      cluster_(cluster),
      prefix_(prefix),
      set_key_(prefix+std::string("set")) {}

  void update(
      const std::string& point_id,
      double lon,
      double lat,
      updateCallbackFn callbackFn);

  void rectangle_query(
      double lon_min, double lon_max, double lat_min, double lat_max,
      queryCallbackFn callbackFn);


private:
  static void redisUpdateCallback(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data );

  static void redisRectangleCallback(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data );


  struct UpdateCallbackData {
    updateCallbackFn callbackFn_;
    int cnt_;
    bool success_;

    UpdateCallbackData(updateCallbackFn callback, int cnt) :
      callbackFn_(callback), cnt_(cnt), success_(true) {}

    void call() { callbackFn_(success_); }
  };

  struct RectangleCallbackData {
    queryCallbackFn callbackFn_;
    int cnt_;
    bool success_;
    std::vector <GeoPoint> *points_;

    RectangleCallbackData(queryCallbackFn callback, int cnt) :
      callbackFn_(callback), cnt_(cnt), success_(true),
      points_(new std::vector<GeoPoint>()) {}

    void add_point(const GeoPoint& p) {
      points_->push_back(p);
    }

    void call() { callbackFn_(success_, points_); }
  };



  typename Cluster<redisAsyncContext>::ptr_t cluster_;
  std::string prefix_;
  std::string set_key_;
  static const int MAX_KEY_LEN = 50;
};

