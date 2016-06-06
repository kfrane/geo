#include <iostream>
#include <chrono>
#include <signal.h>
#include <event2/event.h>

#include "smart_redis_client.h"
#include "geohash.h"

using std::cout;
using std::endl;
using std::vector;

// declare a callback to process reply to redis command
void SmartRedisClient::redisUpdateCallback(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data ) {
  redisReply * reply = static_cast<redisReply*>( r );
  SmartRedisClient::UpdateCallbackData *callbackData =
    static_cast<SmartRedisClient::UpdateCallbackData*>(data);
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
void SmartRedisClient::redisRectangleCallback(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data ) {
  redisReply * reply = static_cast<redisReply*>( r );
  SmartRedisClient::RectangleCallbackData *callbackData =
    static_cast<SmartRedisClient::RectangleCallbackData*>(data);
  if (reply->type == REDIS_REPLY_ERROR) {
    cout << "Reply error " << reply->str << endl;
    callbackData->success_ = false;
  }
  if (reply->type != REDIS_REPLY_ARRAY) {
    std::cout << "Not array " << reply->type << endl
              << strerror(errno) << endl;
  }
  assert (reply->type == REDIS_REPLY_ARRAY);
  for (size_t i = 0; i < reply->elements; i+=2) {
    // array consists of (id, hash) pairs.
    const char* curr_id = reply->element[i]->str;
    const char* curr_hash = reply->element[i+1]->str;
    callbackData->add_point(GeoPoint::from_hash(curr_id, curr_hash));
  }

  callbackData->cnt_--;
  if (callbackData->cnt_ == 0) {
    callbackData->call();
    delete callbackData; // Only delete after calling the callbackFn
  }
}


void SmartRedisClient::update(
    const std::string& point_id,
    double lon,
    double lat,
    updateCallbackFn callbackFn) {
  std::string point_key(prefix_ + point_id);
  std::stringstream point_data;
  point_data << lon << " " << lat;

  UpdateCallbackData *callbackData = new UpdateCallbackData(callbackFn, 2);
  // TODO: Use GETSET to delete the old value from set.
  AsyncHiredisCommand<>::Command(cluster_,
      point_key,                          // key accessed in current command
      redisUpdateCallback,                        // callback to process reply
      static_cast<void*>(callbackData),   // custom user data pointer
      "SET %s %s",                        // command
      point_key.c_str(),                          // paramener - key
      point_data.str().c_str());              // parameter - value

  uint64_t hash;
  hash = GeoPoint::encode(lon, lat);
  std::string my_set_key = set_key_ + GeoPoint::get_prefix(hash, split_level_);
  AsyncHiredisCommand<>::Command(cluster_,
      my_set_key,                          // key accessed in current command
      redisUpdateCallback,                        // callback to process reply
      static_cast<void*>(callbackData),   // custom user data pointer
      "ZADD %s %lld %s",                  // set_key, geohash, point_key
      my_set_key.c_str(),
      hash,
      point_key.c_str());
}

void SmartRedisClient::rectangle_query(
          double lon_min,
          double lon_max,
          double lat_min,
          double lat_max,
          queryCallbackFn callbackFn) {
  vector <GeoHashBits> geo_hashes = cover_rectangle(
      GeoPoint::lat_range, GeoPoint::lon_range,
      lat_min, lat_max, lon_min, lon_max);
  vector <GeoPoint::Range> global_ranges = GeoPoint::merge_geohashes(geo_hashes);

  vector <GeoPoint::Range> ranges;
  for (GeoPoint::Range range: global_ranges) {
    int old_len = ranges.size();
    GeoPoint::to_ranges(ranges, range, split_level_);
    int new_len = ranges.size();
    if (new_len-old_len > 1) {
     // cout << "Podijelio na " << new_len-old_len << " dijelova" << endl;
    }
  }

  RectangleCallbackData *callbackData = new RectangleCallbackData(
                                              callbackFn, ranges.size());
  for (GeoPoint::Range score_range : ranges) {
    assert (GeoPoint::get_prefix(score_range.first, split_level_) ==
            GeoPoint::get_prefix(score_range.second, split_level_));
    std::string my_set_key =
      set_key_ + GeoPoint::get_prefix(score_range.first, split_level_);
    AsyncHiredisCommand<>::Command(cluster_,
        my_set_key,                           // key accessed in current command
        redisRectangleCallback,             // callback to process reply
        static_cast<void*>(callbackData),   // custom user data pointer
        "ZRANGEBYSCORE %s %lld %lld WITHSCORES",
        my_set_key.c_str(),
        score_range.first,
        score_range.second);
  }
}
