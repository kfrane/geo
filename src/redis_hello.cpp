#include <iostream>
#include <fstream>
#include <chrono>
#include <signal.h>
#include <event2/event.h>
#include <vector>
#include <algorithm>

#include "asynchirediscommand.h"

using namespace std;
using RedisCluster::Cluster;
using RedisCluster::AsyncHiredisCommand;

// Declare cluster pointer with redisAsyncContext as template parameter
Cluster<redisAsyncContext>::ptr_t cluster_p;

void cb(
    typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data ) {
  redisReply * reply = static_cast<redisReply*>( r );
  if (reply->type == REDIS_REPLY_ERROR) {
    cout << "Reply error " << reply->str << endl;
  }
  cout << "Set reply " << reply->str << endl;
  cluster_p->disconnect();
}



int main(int argc, char **argv) {
  const char *hostname = "127.0.0.1";
  int port = 30001;

  signal(SIGPIPE, SIG_IGN);
  // create libevent base
  struct event_base *base = event_base_new();
  // Create cluster passing acceptable address and port of one node of the cluster nodes
  cluster_p = AsyncHiredisCommand<>::createCluster(
      hostname, port, static_cast<void*>(base));
  auto slotConn = cluster_p->getConnection("test_key_1");
  cout << slotConn.first.first << "-" << slotConn.first.second << endl;
  cout << slotConn.second->c.tcp.host << endl;
  cout << slotConn.second->c.tcp.port << endl;

  AsyncHiredisCommand<>::Command(cluster_p,
      "test_key_1",                           // key accessed in current command
      cb,             // callback to process reply
      NULL,   // custom user data pointer
      "SET %s %s",
      "test_key_1", "bla");

  // Runs the main loop
  event_base_dispatch(base);

  delete cluster_p;
  event_base_free(base);
  return 0;
}
