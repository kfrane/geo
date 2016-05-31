#include <iostream>
#include <fstream>
#include <chrono>
#include <signal.h>
#include <event2/event.h>
#include <vector>
#include <algorithm>

#include "redis_client.h"
#include "smart_redis_client.h"
#include "redis_client_base.h"

using namespace std;

/**
 * Able to make approximately 7300-7600 req/s when results count is <= 5.
 */

struct rectangle {
  double lon_min, lon_max;
  double lat_min, lat_max;
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

void read_data(istream& data, vector <rectangle>& ret) {
  int i = 1;
  while (!data.eof()) {
    string line;
    getline(data, line);

    vector <string> parts = split(line, ',');
    if (parts.size() != 4) continue;

    rectangle p;

    if (sscanf(parts[0].c_str(), "%lf", &p.lon_min) != 1) {
      continue;
    }
    if (sscanf(parts[1].c_str(), "%lf", &p.lon_max) != 1) {
      continue;
    }
    if (sscanf(parts[2].c_str(), "%lf", &p.lat_min) != 1) {
      continue;
    }
    if (sscanf(parts[3].c_str(), "%lf", &p.lat_max) != 1) {
      continue;
    }

    if (p.lon_min < -180 || p.lon_max > 180 ||
        p.lat_min < -90 || p.lat_max > 90) continue;
    // cout << "Id " << p.id << ", lon " << p.lon << ", lat " << p.lat << endl;
    ret.push_back(p);
    i++;
  }
  cout << "All read" << endl;
}

vector <rectangle> points;
// Declare cluster pointer with redisAsyncContext as template parameter
Cluster<redisAsyncContext>::ptr_t cluster_p;
RedisClientBase *client;
size_t next_to_schedule = 0;
size_t completed = 0;
const size_t ROUND_SIZE = 100;
int total_returned = 0, total_in_area = 0;

struct event* main_loop_ev;
struct timeval zero_seconds = {0,0};

void print_progress(int completed, int total) {
  cout << "\r";
  cout << "Queries completed " << completed << "/" << total;
  cout.flush();
}

void main_loop(evutil_socket_t fd, short what, void *arg) {
  // print_progress(completed, points.size());
  if (!evtimer_pending(main_loop_ev, NULL)) {
    event_del(main_loop_ev);
    if (next_to_schedule >= points.size()) {
      cout << "Done with sending data to redis." << endl;
      return;
    }
    evtimer_add(main_loop_ev, &zero_seconds);
  }

  int round = min(next_to_schedule+ROUND_SIZE, points.size());
  for (; next_to_schedule < round; next_to_schedule++) {
    rectangle p = points[next_to_schedule];
    client->rectangle_query(p.lon_min, p.lon_max, p.lat_min, p.lat_max,
        [=] (bool success, vector <GeoPoint> *results) {
          if (success == 0) {
            cout << "Update error" << endl;
          }
          total_returned += results->size();
          for (const auto& r : *results) {
            if (r.getLongtitude() < p.lon_min || r.getLongtitude() > p.lon_max ||
              r.getLatitude() < p.lat_min || r.getLatitude() > p.lat_max) {
              continue;
            }
            total_in_area++;
          }
          delete results;
          completed++;
          if (completed%100 == 0) {
            print_progress(completed, points.size());
          }
          if (completed == points.size()) {
            cout << endl << "About to call disconnect" << endl;
            cout << "Average # of returned points is "
                 << total_returned / points.size() << endl
                 << "and average number of points in area "
                 << total_in_area / points.size() << endl;
            cluster_p->disconnect();
          }
        });
  }
}


void create_user_event(event_base *base) {
  // User event
  main_loop_ev = evtimer_new(base, main_loop, NULL);
  evtimer_add(main_loop_ev, &zero_seconds);
}

RedisClientBase *create_redis(const char*arg, const string& key_prefix) {
  if (strcmp(arg, "basic") == 0) {
    return new RedisClient(cluster_p, key_prefix);
  } else if (strcmp(arg, "smart") == 0) {
    return new SmartRedisClient(cluster_p, key_prefix, 11);
  }
  cerr << "Redis client should be basic or smart" << endl;
  return NULL;
}

int main(int argc, char **argv) {
  const char *hostname = "127.0.0.1";
  int port = 30001;

  string key_prefix = "test2:";
  if (argc > 1) {
    key_prefix = string(argv[1]);
  }
  const char* redis_client_type = "basic";
  if (argc > 2) {
    redis_client_type = argv[2];
  }

  read_data(cin, points);
  if (points.size() == 0) {
    cout << "No valid queries in input" << endl;
    return 0;
  }

  signal(SIGPIPE, SIG_IGN);
  // create libevent base
  struct event_base *base = event_base_new();
  create_user_event(base);
  // Create cluster passing acceptable address and port of one node of the cluster nodes
  cluster_p = AsyncHiredisCommand<>::createCluster(
      hostname, port, static_cast<void*>(base));

  client = create_redis(redis_client_type, key_prefix);

  chrono::time_point<chrono::system_clock> start, end;
  start = chrono::system_clock::now();

  // Runs the main loop
  event_base_dispatch(base);

  end = chrono::system_clock::now();
  chrono::duration<double> elapsed_seconds = end-start;
  cout << "Done with all updates in " << elapsed_seconds.count()
    << "s which is " << (double)(points.size()) / elapsed_seconds.count()
    << "req/s" << endl;

  delete cluster_p;
  delete client;
  event_base_free(base);
  return 0;
}
