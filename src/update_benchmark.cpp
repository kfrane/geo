#include <iostream>
#include <fstream>
#include <chrono>
#include <signal.h>
#include <event2/event.h>
#include <vector>

#include "redis_client.h"

using namespace std;

struct point {
  string id;
  double lon;
  double lat;
};

vector<string> &split(
          const string &s,
          char delim,
          vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

void read_data(istream& data, vector <point>& ret) {
  int i = 1;
  while (!data.eof()) {
    string line;
    getline(data, line);

    vector <string> parts = split(line, ',');
    if (parts.size() != 4) continue;

    point p;
    stringstream gen_id;
    gen_id << "P:" << i;
    p.id = gen_id.str();

    if (sscanf(parts[2].c_str(), "%lf", &p.lon) != 1) {
      continue;
    }
    if (sscanf(parts[3].c_str(), "%lf", &p.lat) != 1) {
      continue;
    }
    // cout << "Id " << p.id << ", lon " << p.lon << ", lat " << p.lat << endl;
    ret.push_back(p);
    i++;
  }
  cout << "All read" << endl;
}

int main(int argc, char **argv) {
  const char *hostname = "127.0.0.1";
  int port = 30001;

  string key_prefix = "test2:";
  if (argc > 1) {
    key_prefix = string(argv[1]);
  }

  vector <point> points;
  read_data(cin, points);
  if (points.size() == 0) {
    cout << "No valid point in input" << endl;
    return 0;
  }

   // Declare cluster pointer with redisAsyncContext as template parameter
  Cluster<redisAsyncContext>::ptr_t cluster_p;
  signal(SIGPIPE, SIG_IGN);
  // create libevent base
  struct event_base *base = event_base_new();
  // Create cluster passing acceptable address and port of one node of the cluster nodes
  cluster_p = AsyncHiredisCommand<>::createCluster(
      hostname, port, static_cast<void*>( base ) );

  RedisClient client(cluster_p, key_prefix);
  chrono::time_point<chrono::system_clock> start, end;
  start = chrono::system_clock::now();

  cout << "First inserted key id is: " << points[0].id << endl;
  int total = points.size(), completed = 0;
  for (int i = 0; i < total; i++) {
    point p = points[i];
    client.update(p.id, p.lon, p.lat, [cluster_p, total, &completed] (bool success) {
      if (success == 0) {
        cout << "Update error" << endl;
      }
      completed++;
      if (completed == total) {
        cluster_p->disconnect();
      }
    });
  }

  // process event loop
  event_base_dispatch(base);

  end = chrono::system_clock::now();
  chrono::duration<double> elapsed_seconds = end-start;
  cout << "Done with all updates in " << elapsed_seconds.count()
    << "s which is " << (double)(total) / elapsed_seconds.count()
    << "req/s" << endl;

  delete cluster_p;
  event_base_free(base);
  return 0;
}
