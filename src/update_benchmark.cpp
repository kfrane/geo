#include <iostream>
#include <fstream>
#include <chrono>
#include <signal.h>
#include <event2/event.h>
#include <vector>
#include <algorithm>

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
    if (p.lon < -180 || p.lon > 180 || p.lat < -90 || p.lat > 90) continue;
    // cout << "Id " << p.id << ", lon " << p.lon << ", lat " << p.lat << endl;
    ret.push_back(p);
    i++;
  }
  cout << "All read" << endl;
}

vector <point> points;
// Declare cluster pointer with redisAsyncContext as template parameter
Cluster<redisAsyncContext>::ptr_t cluster_p;
RedisClient *client;
size_t next_to_schedule = 0;
size_t completed = 0;
const size_t ROUND_SIZE = 1999;

struct event* main_loop_ev;
struct timeval zero_seconds = {0,0};

void print_progress(int completed, int total) {
  cout << "\r";
  cout << "Queries completed " << completed << "/" << total;
  cout.flush();
}

void main_loop(evutil_socket_t fd, short what, void *arg) {
  print_progress(completed, points.size());
  if (!evtimer_pending(main_loop_ev, NULL)) {
    event_del(main_loop_ev);
    if (next_to_schedule >= points.size()) return;
    evtimer_add(main_loop_ev, &zero_seconds);
  }

  int round = min(next_to_schedule+ROUND_SIZE, points.size());
  for (; next_to_schedule < round; next_to_schedule++) {
    point p = points[next_to_schedule];
    client->update(p.id, p.lon, p.lat, [] (bool success) {
      if (success == 0) {
        cout << "Update error" << endl;
      }
      completed++;
  //    cout << "Completed " << completed << " out of " << points.size() << endl;
      if (completed == points.size()) {
        cout << endl << "About to call disconnect" << endl;
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

int main(int argc, char **argv) {
  const char *hostname = "127.0.0.1";
  int port = 30001;

  string key_prefix = "test2:";
  if (argc > 1) {
    key_prefix = string(argv[1]);
  }

  read_data(cin, points);
  if (points.size() == 0) {
    cout << "No valid point in input" << endl;
    return 0;
  }

  signal(SIGPIPE, SIG_IGN);
  // create libevent base
  struct event_base *base = event_base_new();
  create_user_event(base);
  // Create cluster passing acceptable address and port of one node of the cluster nodes
  cluster_p = AsyncHiredisCommand<>::createCluster(
      hostname, port, static_cast<void*>(base));

  client = new RedisClient(cluster_p, key_prefix);

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
