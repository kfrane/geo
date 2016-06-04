#include <iostream>
#include <chrono>
#include <signal.h>
#include <event2/event.h>

#include "native_redis_client.h"
#include "geohash.h"

using std::cout;
using std::endl;
using std::vector;

// declare a callback to process reply to redis command
void NativeRedisClient::redisUpdateCallback(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data ) {
  redisReply * reply = static_cast<redisReply*>( r );
  NativeRedisClient::UpdateCallbackData *callbackData =
    static_cast<NativeRedisClient::UpdateCallbackData*>(data);
  if (reply->type == REDIS_REPLY_ERROR) {
    cout << "Reply error " << reply->str << endl;
    callbackData->success_ = false;
  }

  callbackData->cnt_--;
  if (callbackData->cnt_ == 0) {
    callbackData->call();
    delete callbackData; // Only delete after calling the callbackFn
  }
}

// declare a callback to process reply to redis command
void NativeRedisClient::geoRadiusCallback(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data ) {
  redisReply * reply = static_cast<redisReply*>( r );
  NativeRedisClient::GeoRadiusCallbackData *callbackData =
    static_cast<NativeRedisClient::GeoRadiusCallbackData*>(data);
  if (reply->type == REDIS_REPLY_ERROR) {
    cout << "Reply error " << reply->str << endl;
    callbackData->success_ = false;
  }

  assert (reply->type == REDIS_REPLY_ARRAY);
  for (size_t i = 0; i < reply->elements; i++) {
    redisReply* single_point = reply->element[i];
    assert (single_point->type == REDIS_REPLY_ARRAY);
    assert (single_point->elements == 2);

    // array consists of (id, hash) pairs.
    const char* curr_id = single_point->element[0]->str;

    redisReply* coordinates = single_point->element[1];
    assert (single_point->element[1]->type == REDIS_REPLY_ARRAY);
    double lon, lat;
    sscanf(coordinates->element[0]->str, "%lf", &lon);
    sscanf(coordinates->element[1]->str, "%lf", &lat);
    callbackData->add_point(GeoPoint(lon, lat, curr_id));
  }

  callbackData->cnt_--;
  if (callbackData->cnt_ == 0) {
    callbackData->call();
    delete callbackData; // Only delete after calling the callbackFn
  }
}


void NativeRedisClient::update(
    const std::string& point_id,
    double lon,
    double lat,
    updateCallbackFn callbackFn) {
  std::string point_key(prefix_ + point_id);
  std::stringstream point_data;
  point_data << lon << " " << lat;

  UpdateCallbackData *callbackData = new UpdateCallbackData(callbackFn, 2);
  AsyncHiredisCommand<>::Command(cluster_,
      point_key,                          // key accessed in current command
      redisUpdateCallback,                        // callback to process reply
      static_cast<void*>(callbackData),   // custom user data pointer
      "SET %s %s",                        // command
      point_key.c_str(),                          // paramener - key
      point_data.str().c_str());              // parameter - value

  AsyncHiredisCommand<>::Command(cluster_,
      set_key_,                          // key accessed in current command
      redisUpdateCallback,                        // callback to process reply
      static_cast<void*>(callbackData),   // custom user data pointer
      "GEOADD %s %f %f %s",                  // set_key, geohash, point_key
      set_key_.c_str(),
      lon,
      lat,
      point_key.c_str());
}

void NativeRedisClient::rectangle_query(
          double lon_min,
          double lon_max,
          double lat_min,
          double lat_max,
          queryCallbackFn callbackFn) {
  std::cerr << "Not implemented" << std::endl;
}

void NativeRedisClient::radius_query(
      double radius, double lon, double lat, queryCallbackFn callbackFn) {
  GeoRadiusCallbackData *callbackData = new GeoRadiusCallbackData(
                                              callbackFn, 1);
  AsyncHiredisCommand<>::Command(cluster_,
      set_key_,                           // key accessed in current command
      geoRadiusCallback,             // callback to process reply
      static_cast<void*>(callbackData),   // custom user data pointer
      "GEORADIUS %s %f %f %f m WITHCOORD",
      set_key_.c_str(),
      lon,
      lat,
      radius);
}

