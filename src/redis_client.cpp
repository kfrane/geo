#include "redis_client.h"

/*
bool RedisClient::update(double lng, double lat) {
  return true;
}
*/

int main() {
  return 0;
}

/*
// declare a callback to process reply to redis command
static void setCallback( typename Cluster<redisAsyncContext>::ptr_t cluster_p, void *r, void *data )
{
  // declare local pointer to work with reply
  redisReply * reply = static_cast<redisReply*>( r );
  // cast data that you pass as callback parameter below (not necessary)
  string *demoData = static_cast<string*>( data );
  // check redis reply usual
  if (reply->type == REDIS_REPLY_STATUS) {
    // process reply
//    cout << " Reply to SET FOO BAR " << endl;
//    cout << reply->str << endl;
  } else {
    cout << "Reply error" << endl;
  }

  // process callback parameter if you want (not necessary)
  // cout << *demoData << endl;
  delete demoData;

  processed++;
  if (processed == TOTAL) {
    // disconnecting cluster will brake the event loop
    cluster_p->disconnect();
  }
}

// declare a functions that invokes redis commanf
void processAsyncCommand(const char *hostname, const int port) {
  // Declare cluster pointer with redisAsyncContext as template parameter
  Cluster<redisAsyncContext>::ptr_t cluster_p;
  // ignore sigpipe as we use libevent
  signal(SIGPIPE, SIG_IGN);
  // create libevent base
  struct event_base *base = event_base_new();
  // Create cluster passing acceptable address and port of one node of the cluster nodes
  cluster_p = AsyncHiredisCommand<>::createCluster( hostname, port, static_cast<void*>( base ) );
  // send command to redis passing created cluster pointer, key which you wish to access in the command
  // callback function, that just already declared above, pointer to any user defined data
  // and command itself with parameters with printf like syntax

  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  for (int i = 0; i < TOTAL; i++) {
    // create custom data that will be passed to callback (not necessary)
    string *demoData = new string("Demo data is ok");
    AsyncHiredisCommand<>::Command( cluster_p,                      // cluster pointer
        "FOOAA",                             // key accessed in current command
        setCallback,                       // callback to process reply
        static_cast<void*>( demoData ),    // custom user data pointer
        "SET %s %s",                       // command
        "FOOAA",                             // paramener - key
        "BAR1" );                          // parameter - value
  }
  // process event loop
  event_base_dispatch(base);


  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  cout << "Done with processClusterKeysSubset in " << elapsed_seconds.count()
    << " which is " << (double)(TOTAL) / elapsed_seconds.count() << endl;


  // delete cluster object
  delete cluster_p;
  // free event base
  event_base_free(base);
}

int main() {
  processAsyncCommand("127.0.0.1", 30001);
  return 0;
}

*/
