#include <iostream>
#include <chrono>
#include <signal.h>
#include <event2/event.h>

#include "redis_client.h"
#include "geohash.h"

using std::cout;
using std::endl;

// declare a callback to process reply to redis command
void RedisClient::redisCallback(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data ) {
  // declare local pointer to work with reply
  redisReply * reply = static_cast<redisReply*>( r );
  // cast data that you pass as callback parameter below (not necessary)
  RedisClient::CallbackData *callbackData = static_cast<RedisClient::CallbackData*>(data);
  // check redis reply usual
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

void RedisClient::update(
    const std::string& point_id,
    double lon,
    double lat,
    geoCallbackFn callbackFn) {
  std::string point_key(prefix_ + point_id);
  std::stringstream point_data;
  point_data << lon << " " << lat;

  CallbackData *callbackData = new CallbackData(callbackFn, 2);
  AsyncHiredisCommand<>::Command(cluster_,
      point_key,                          // key accessed in current command
      redisCallback,                        // callback to process reply
      static_cast<void*>(callbackData),   // custom user data pointer
      "SET %s %s",                        // command
      point_key.c_str(),                          // paramener - key
      point_data.str().c_str());              // parameter - value

  GeoHashBits hash;
  assert(geohash_fast_encode(
        lat_range_, lon_range_, lat, lon, HASH_BITS, &hash) == 0);
  AsyncHiredisCommand<>::Command(cluster_,
      set_key_,                          // key accessed in current command
      redisCallback,                        // callback to process reply
      static_cast<void*>(callbackData),   // custom user data pointer
      "ZADD %s %lld %s",                  // set_key, geohash, point_key
      set_key_.c_str(),
      hash.bits,
      point_key.c_str());
}

/*
 * Usage example.
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
      hostname, port, static_cast<void*>( base ) );

  RedisClient client(cluster_p, "test2:");
  client.update("client1", 20, -30, [cluster_p] (bool success) {
    if (success == 0) {
      cout << "Update error" << endl;
    }
    cluster_p->disconnect();
  });

  // process event loop
  event_base_dispatch(base);

  delete cluster_p;
  event_base_free(base);
  return 0;
}
*/
