#pragma once

#include <string>
#include <functional> // maybe use boost version
#include <sstream>
#include <vector>

#include "redis_client_base.h"
#include "asynchirediscommand.h"
#include "geopoint.h"

using RedisCluster::AsyncHiredisCommand;
using RedisCluster::Cluster;

class NativeRedisClient : public RedisClientBase {
public:
  NativeRedisClient(
      typename Cluster<redisAsyncContext>::ptr_t cluster,
      std::string prefix) :
      cluster_(cluster),
      prefix_(prefix),
      set_key_(prefix+std::string("set")) {}

  void update(
      const std::string& point_id,
      double lon,
      double lat,
      updateCallbackFn callbackFn) override;

  void rectangle_query(
      double lon_min, double lon_max, double lat_min, double lat_max,
      queryCallbackFn callbackFn) override;

  void radius_query(
      double radius, double lon, double lat, queryCallbackFn callbackFn);


private:
  static void redisUpdateCallback(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data );

  static void geoRadiusCallback(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data );


  struct UpdateCallbackData {
    updateCallbackFn callbackFn_;
    int cnt_;
    bool success_;

    UpdateCallbackData(updateCallbackFn callback, int cnt) :
      callbackFn_(callback), cnt_(cnt), success_(true) {}

    void call() { callbackFn_(success_); }
  };

  struct GeoRadiusCallbackData {
    queryCallbackFn callbackFn_;
    int cnt_;
    bool success_;
    std::vector <GeoPoint> *points_;

    GeoRadiusCallbackData(queryCallbackFn callback, int cnt) :
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
};

