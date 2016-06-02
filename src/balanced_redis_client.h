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
 * sets (set_count of them), each one storing a portion of data points.
 * Point is stored into one of the sets based on its id.
 * When quering all of the sets need to be queried.
 *
 * It is not so easy to controll which set ends up on which redis node,
 * so set_count should be few times larger then the node count to
 * ensure balanced load.
 * TODO: force each set on its own redis node.
 */
class BalancedRedisClient : public RedisClientBase {
public:
  BalancedRedisClient(
      typename Cluster<redisAsyncContext>::ptr_t cluster,
      std::string prefix,
      int set_count);

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

  std::string findMySet(const std::string& point_key);

  typename Cluster<redisAsyncContext>::ptr_t cluster_;
  std::string prefix_;
  int set_count_;
  std::string set_key_;
  std::vector<std::string> set_names_;
};

