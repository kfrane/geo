#include <iostream>
#include <chrono>
#include <signal.h>
#include <event2/event.h>

#include "redis_client.h"
#include "geohash.h"

using std::cout;
using std::endl;
using std::vector;

// declare a callback to process reply to redis command
void RedisClient::redisUpdateCallback(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data ) {
  redisReply * reply = static_cast<redisReply*>( r );
  RedisClient::UpdateCallbackData *callbackData =
    static_cast<RedisClient::UpdateCallbackData*>(data);
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
void RedisClient::redisRectangleCallback(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data ) {
  redisReply * reply = static_cast<redisReply*>( r );
  RedisClient::RectangleCallbackData *callbackData =
    static_cast<RedisClient::RectangleCallbackData*>(data);
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


void RedisClient::update(
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
  AsyncHiredisCommand<>::Command(cluster_,
      set_key_,                          // key accessed in current command
      redisUpdateCallback,                        // callback to process reply
      static_cast<void*>(callbackData),   // custom user data pointer
      "ZADD %s %lld %s",                  // set_key, geohash, point_key
      set_key_.c_str(),
      hash,
      point_key.c_str());
}

void RedisClient::rectangle_query(
          double lon_min,
          double lon_max,
          double lat_min,
          double lat_max,
          queryCallbackFn callbackFn) {
  vector <GeoHashBits> geo_hashes = cover_rectangle(
      GeoPoint::lat_range, GeoPoint::lon_range,
      lat_min, lat_max, lon_min, lon_max);
  vector <GeoPoint::Range> geo_ranges = GeoPoint::merge_geohashes(geo_hashes);

  RectangleCallbackData *callbackData = new RectangleCallbackData(
                                              callbackFn, geo_ranges.size());
  for (const GeoPoint::Range& score_range: geo_ranges) {
    AsyncHiredisCommand<>::Command(cluster_,
        set_key_,                           // key accessed in current command
        redisRectangleCallback,             // callback to process reply
        static_cast<void*>(callbackData),   // custom user data pointer
        "ZRANGEBYSCORE %s %lld %lld WITHSCORES",
        set_key_.c_str(),
        score_range.first,
        score_range.second);
  }
}

void cb_func(evutil_socket_t fd, short what, void *arg) {
  const char *data = (char *)arg;
  printf("Got an event on socket %d:%s%s%s%s [%s]\n",
      (int) fd,
      (what&EV_TIMEOUT) ? " timeout" : "",
      (what&EV_READ)    ? " read" : "",
      (what&EV_WRITE)   ? " write" : "",
      (what&EV_SIGNAL)  ? " signal" : "",
      data);
}

/*
 * Usage example.
 */
/*
int main() {
  const char *hostname = "127.0.0.1";
  int port = 30001;

   // Declare cluster pointer with redisAsyncContext as template parameter
  Cluster<redisAsyncContext>::ptr_t cluster_p;
  signal(SIGPIPE, SIG_IGN);
  // create libevent base
  struct event_base *base = event_base_new();
  // Create cluster passing acceptable address and port of one node of the cluster nodes
  cluster_p = AsyncHiredisCommand<>::createCluster(
      hostname, port, static_cast<void*>(base));

  struct event *ev1;
  struct timeval five_seconds = {0,0};
  // User event
  ev1 = event_new(base, -1, EV_TIMEOUT, cb_func,
             (char*)"Reading event");
  event_add(ev1, &five_seconds);

  RedisClient client(cluster_p, "test2:");
  client.update("client1", 20, -30, [cluster_p] (bool success) {
    if (success == 0) {
      cout << "Update error" << endl;
    }
    cout << "Update finished" << endl;
//    cluster_p->disconnect();
  });

  // process event loop
  event_base_dispatch(base);

  delete cluster_p;
  event_base_free(base);
  return 0;
}
*/
