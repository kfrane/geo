#pragma once

#include <string>
#include <functional> // maybe use boost version
#include <sstream>

#include "asynchirediscommand.h"
#include "geohash.h"

using RedisCluster::AsyncHiredisCommand;
using RedisCluster::Cluster;

class RedisClient {
public:
  typedef std::function<void(bool)> geoCallbackFn;

  RedisClient(
      typename Cluster<redisAsyncContext>::ptr_t cluster,
      std::string prefix) :
      cluster_(cluster),
      prefix_(prefix),
      set_key_(prefix+std::string("set")) {
        lon_range_ = {.max=180, .min=-180};
        lat_range_ = {.max=90, .min=-90};
      }

  void update(
      const std::string& point_id,
      double lon,
      double lat,
      geoCallbackFn callbackFn);

private:
  static void redisCallback(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data );

  struct CallbackData {
    geoCallbackFn callbackFn_;
    int cnt_;
    bool success_;

    CallbackData(geoCallbackFn callback, int cnt) :
      callbackFn_(callback), cnt_(cnt), success_(true) {}

    void call() { callbackFn_(success_); }
  };

  typename Cluster<redisAsyncContext>::ptr_t cluster_;
  std::string prefix_;
  std::string set_key_;
  static const int MAX_KEY_LEN = 50;
  static const int HASH_BITS = 26; // In total 52 bits
  GeoHashRange lon_range_;
  GeoHashRange lat_range_;
};

