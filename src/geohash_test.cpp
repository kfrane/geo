#include "geohash.h"
#include <iostream>
#include <algorithm>

using namespace std;

GeoHashRange lat_range(90, -90);
GeoHashRange lon_range(180, -180);


void test1() {
  cout << "changed" << endl;
  vector<GeoHashBits> ret =
    cover_rectangle(lat_range, lon_range, 1, 2, 5, 6);
  sort(ret.begin(), ret.end(), [] (GeoHashBits a, GeoHashBits b)
      {return a.bits < b.bits; });
  for (GeoHashBits rectangle_hash : ret) {
    cout << rectangle_hash.bits << endl << (int)rectangle_hash.step << endl;
    GeoHashArea area;
    geohash_fast_decode(lat_range, lon_range, rectangle_hash, &area);
    cout << area.latitude.min << "-" << area.latitude.max << " "
         << area.longitude.min << "-" << area.longitude.max << endl;
  }
}

int main() {
  test1();
  return 0;
}
