#pragma once

#include <string>
#include <functional> // maybe use boost version
#include <sstream>
#include <vector>

#include "asynchirediscommand.h"
#include "redis_client_base.h"

#include "geohash.h"
#include "geopoint.h"

using RedisCluster::AsyncHiredisCommand;
using RedisCluster::Cluster;

/**
 * Instead of having only one sorted set, this class creates multiple sorted
 * sets, each one containing all the points with the same hash prefix of length
 * 2 * split_level.
 * When adding a point its location is updated by looking at the points key.
 * The old locations is deleted from the appropriate set, and new location is
 * added (old location is deleted only if in different set).
 *
 * Query can be split among multiple sets. If query area is not large, the
 * number of splits between sets won't be big (or maybe won't happen at all).
 *
 * This is why choosing the appropriate split_level is important.
 */
class SmartRedisClient : public RedisClientBase {
public:
  SmartRedisClient(
      typename Cluster<redisAsyncContext>::ptr_t cluster,
      std::string prefix,
      int split_level) :
      cluster_(cluster),
      prefix_(prefix),
      split_level_(split_level),
      set_key_(prefix+std::string("set:")) {}

  void update(
      const std::string& point_id,
      double lon,
      double lat,
      updateCallbackFn callbackFn) override;

  void rectangle_query(
      double lon_min, double lon_max, double lat_min, double lat_max,
      queryCallbackFn callbackFn) override;

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
  int split_level_; // Prefix length after which we split
  std::string set_key_;
  static const int MAX_KEY_LEN = 50;
};

