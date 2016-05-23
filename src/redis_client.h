#pragma once

#include "asynchirediscommand.h"

using RedisCluster::AsyncHiredisCommand;
using RedisCluster::Cluster;

class RedisClient {
public:
  RedisClient(typename Cluster<redisAsyncContext>::ptr_t cluster) :
    cluster_(cluster) { }

  bool update(double lng, double lat);

private:
  typename Cluster<redisAsyncContext>::ptr_t cluster_;
};
