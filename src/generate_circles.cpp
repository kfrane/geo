#include <vector>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <random>
#include <iomanip>

using namespace std;

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

double kth(vector <double>& v, int k) {
  nth_element(v.begin(), v.begin()+k, v.end());
  return v[k];
}

typedef pair<double, double> range;

void read_data(istream& data, range& lon_range, range& lat_range, int& count) {
  vector <double> lons, lats;
  while (!data.eof()) {
    string line;
    getline(data, line);

    vector <string> parts = split(line, ',');
    if (parts.size() != 4) continue;

    double lon, lat;
    if (sscanf(parts[2].c_str(), "%lf", &lon) != 1) {
      continue;
    }
    if (sscanf(parts[3].c_str(), "%lf", &lat) != 1) {
      continue;
    }
    if (lon < -180 || lon > 180 || lat < -90 || lat > 90) continue;
    lons.push_back(lon);
    lats.push_back(lat);
  }
  count = lons.size();
  int k_min = max(10, int(0.01*lons.size()));
  if (lons.size() < 100) k_min = 0;
  int k_max = lons.size()-k_min-1;
  lon_range = range(kth(lons, k_min), kth(lons, k_max));
  lat_range = range(kth(lats, k_min), kth(lats, k_max));
}

void generate_circles(
                double mean,
                double deviation,
                range lon_range,
                range lat_range,
                int count) {
  std::default_random_engine generator;
  std::uniform_real_distribution<double>
      lon_distribution(lon_range.first, lon_range.second);
  std::uniform_real_distribution<double>
      lat_distribution(lat_range.first, lat_range.second);
  std::normal_distribution<double>
      radius_distribution(mean, deviation);
for (int i = 0; i < count; i++) {
    double lon_start = lon_distribution(generator);
    double lat_start = lat_distribution(generator);
    double radius;
    for (radius = 0; radius < 1; radius = radius_distribution(generator));
    cout << lon_start << "," << lat_start << ","
         << radius << endl;
  }
}

/**
 * First argument is expected number of points in rectangle.
 */
int main(int argc, char **argv) {
  cin.sync_with_stdio(false);
  std::cout << std::setprecision(10);

  double mean = 50;
  if (argc > 1) {
    sscanf(argv[1], "%lf", &mean);
  }

  double deviation = 50;
  if (argc > 2) {
    sscanf(argv[2], "%lf", &deviation);
  }


  range lon_range, lat_range;
  int count;
  read_data(cin, lon_range, lat_range, count);
  std::cerr << lon_range.first << " " << lon_range.second << std::endl;
  std::cerr << lat_range.first << " " << lat_range.second << std::endl;
  count = 1000000;
  generate_circles(
      mean,
      deviation,
      lon_range,
      lat_range,
      count);
}
