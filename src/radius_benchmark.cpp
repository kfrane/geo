#include <iostream>
#include <fstream>
#include <chrono>
#include <signal.h>
#include <event2/event.h>
#include <vector>
#include <algorithm>

#include "redis_client.h"
#include "smart_redis_client.h"
#include "balanced_redis_client.h"
#include "redis_client_base.h"
#include "native_redis_client.h"
#include "log_utility.h"

using namespace std;

/**
 * Avg number of points in radius is 4.
 *
 * Basic:
 * 9800 req/s.
 *
 * Smart:
 * 6500 req/s.
 *
 * Balanced:
 * 5400 req/s
 *
 * Native:
 * 16800 req/s.
 *
 */

ofstream* log_stream;

struct circle {
  double lon, lat;
  double radius;
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

void read_data(istream& data, vector <circle>& ret) {
  int i = 1;
  while (!data.eof()) {
    string line;
    getline(data, line);

    vector <string> parts = split(line, ',');
    if (parts.size() != 3) continue;

    circle p;

    if (sscanf(parts[2].c_str(), "%lf", &p.radius) != 1) {
      continue;
    }
    if (sscanf(parts[0].c_str(), "%lf", &p.lon) != 1) {
      continue;
    }
    if (sscanf(parts[1].c_str(), "%lf", &p.lat) != 1) {
      continue;
    }
    if (p.lon < -180 || p.lon > 180 ||
        p.lat < -90 || p.lat > 90 ||
        p.radius <= 0) continue;
    // cout << "Id " << p.id << ", lon " << p.lon << ", lat " << p.lat << endl;
    ret.push_back(p);
    i++;
  }
  cout << "All read" << endl;
}

vector <circle> points;
// Declare cluster pointer with redisAsyncContext as template parameter
Cluster<redisAsyncContext>::ptr_t cluster_p;
RedisClientBase *client;
size_t next_to_schedule = 0;
size_t completed = 0;
const size_t ROUND_SIZE = 10;
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

size_t round = min(next_to_schedule+ROUND_SIZE, points.size());
  for (; next_to_schedule < round; next_to_schedule++) {
    circle p = points[next_to_schedule];
    client->radius_query(p.radius, p.lon, p.lat,
        [=] (bool success, vector <GeoPoint> *results, size_t points_retrieved) {
          if (success == 0) {
            cout << "Update error" << endl;
          }
          total_returned += points_retrieved;
      //    cout << "Resylt size " << results->size() << endl;
          for (GeoPoint curr_p : *results) {
           // cout << curr_p.getId() << " " << curr_p.getLongtitude() << " " <<
           //  curr_p.getLatitude() << endl;
          }
          total_in_area += results->size();
          delete results;
          completed++;
          if (completed%100 == 0) {
            print_progress(completed, points.size());
          }
          if (completed == points.size()) {
            cout << endl << "About to call disconnect" << endl;
            cout << "Average # of returned points is "
                 << (double) total_returned / points.size() << endl
                 << "and average number of points in area "
                 << (double) total_in_area / points.size() << endl;
            *log_stream << "Average # of returned points is "
                 << (double) total_returned / points.size() << endl
                 << "and average number of points in area "
                 << (double) total_in_area / points.size() << endl;
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

void print_usage() {
  cerr << "Usage ./radius_benchmark key_prefix basic|smart|balanced|native "
       << "split_level|set_count"
       << endl;
  exit(1);
}

RedisClientBase *create_redis(
    const char*arg, const string& key_prefix, int split_level) {
  if (strcmp(arg, "basic") == 0) {
    return new RedisClient(cluster_p, key_prefix);
  } else if (strcmp(arg, "smart") == 0) {
    return new SmartRedisClient(cluster_p, key_prefix, split_level);
  } else if (strcmp(arg, "balanced") == 0) {
    int set_count = split_level;
    return new BalancedRedisClient(cluster_p, key_prefix, set_count);
  } else if (strcmp(arg, "native") == 0) {
    return new NativeRedisClient(cluster_p, key_prefix);
  }

  cerr << "Redis client should be basic or smart" << endl;
  print_usage();
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

  int split_level = 11;
  if (argc > 3) {
    sscanf(argv[3], "%d", &split_level);
  }


  read_data(cin, points);
  if (points.size() == 0) {
    cout << "No valid queries in input" << endl;
    return 0;
  }
  log_stream = &init_log("results/", argc, argv);

  signal(SIGPIPE, SIG_IGN);
  // create libevent base
  struct event_base *base = event_base_new();
  create_user_event(base);
  // Create cluster passing acceptable address and port of one node of the cluster nodes
  cluster_p = AsyncHiredisCommand<>::createCluster(
      hostname, port, static_cast<void*>(base));

  client = create_redis(redis_client_type, key_prefix, split_level);

  chrono::time_point<chrono::system_clock> start, end;
  start = chrono::system_clock::now();

  // Runs the main loop
  event_base_dispatch(base);

  end = chrono::system_clock::now();
  chrono::duration<double> elapsed_seconds = end-start;
  cout << "Done with all updates in " << elapsed_seconds.count()
    << "s which is " << (double)(points.size()) / elapsed_seconds.count()
    << "req/s" << endl;
  *log_stream << "Done with all queries in " << elapsed_seconds.count()
    << "s which is " << (double)(points.size()) / elapsed_seconds.count()
    << "req/s" << endl
    << "Queries #" << points.size() << endl;


  delete cluster_p;
  delete client;
  event_base_free(base);
  log_stream->close();
  delete log_stream;
  return 0;
}
