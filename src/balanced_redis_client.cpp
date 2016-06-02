#include <iostream>
#include <chrono>
#include <signal.h>
#include <event2/event.h>

#include "slothash.h"

#include "balanced_redis_client.h"
#include "geohash.h"

using std::cout;
using std::endl;
using std::vector;
using RedisCluster::SlotHash;

BalancedRedisClient::BalancedRedisClient(
            typename Cluster<redisAsyncContext>::ptr_t cluster,
            std::string prefix,
            int set_count) :
            cluster_(cluster),
            prefix_(prefix),
            set_count_(set_count),
            set_key_(prefix+std::string("set:")) {
  for (int set_index = 0; set_index < set_count; set_index++) {
    std::string set_name = set_key_ + std::to_string(set_index);
    set_names_.push_back(set_name);
  }
}



// declare a callback to process reply to redis command
void BalancedRedisClient::redisUpdateCallback(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data ) {
  redisReply * reply = static_cast<redisReply*>( r );
  BalancedRedisClient::UpdateCallbackData *callbackData =
    static_cast<BalancedRedisClient::UpdateCallbackData*>(data);
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
void BalancedRedisClient::redisRectangleCallback(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data ) {
  redisReply * reply = static_cast<redisReply*>( r );
  BalancedRedisClient::RectangleCallbackData *callbackData =
    static_cast<BalancedRedisClient::RectangleCallbackData*>(data);
  if (reply->type == REDIS_REPLY_ERROR) {
    cout << "Reply error " << reply->str << endl;
    callbackData->success_ = false;
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


void BalancedRedisClient::update(
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

  uint64_t hash;
  hash = GeoPoint::encode(lon, lat);
  std::string my_set_key = findMySet(point_id);
  AsyncHiredisCommand<>::Command(cluster_,
      my_set_key,                          // key accessed in current command
      redisUpdateCallback,                        // callback to process reply
      static_cast<void*>(callbackData),   // custom user data pointer
      "ZADD %s %lld %s",                  // set_key, geohash, point_key
      my_set_key.c_str(),
      hash,
      point_key.c_str());
}

void BalancedRedisClient::rectangle_query(
          double lon_min,
          double lon_max,
          double lat_min,
          double lat_max,
          queryCallbackFn callbackFn) {
  vector <GeoHashBits> geo_hashes = cover_rectangle(
      GeoPoint::lat_range, GeoPoint::lon_range,
      lat_min, lat_max, lon_min, lon_max);

  vector <GeoPoint::Range> ranges;
  for (GeoHashBits geo_hash : geo_hashes) {
    GeoPoint::Range range = GeoPoint::to_range(geo_hash);
    ranges.push_back(range);
  }

  RectangleCallbackData *callbackData =
    new RectangleCallbackData( callbackFn, ranges.size() * set_count_);
  for (const GeoPoint::Range& score_range : ranges) {
    for (const std::string& my_set_key : set_names_) {
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
}

std::string BalancedRedisClient::findMySet(const std::string& point_key) {
  int key_hash = SlotHash::SlotByKey(point_key.c_str(), point_key.size());
  return set_names_[key_hash % set_count_];
}
