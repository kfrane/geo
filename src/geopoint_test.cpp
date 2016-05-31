#include "geopoint.h"
#include <iostream>
#include <cassert>

using namespace std;

void test_to_range() {
  GeoHashBits h(1LL<<41, 21);
  GeoPoint::Range range = GeoPoint::to_range(h);
  assert(range.first == (1LL<<51));
  assert(range.second == (1LL<<51)+(1LL<<10)-1);
}

string to_bits(uint64_t b, int bits) {
  string ret;
  for (int i = bits-1; i >= 0; i--) {
    if ((1LL<<i)&b) {
      ret += "1";
    } else {
      ret += "0";
    }
  }
  return ret;
}

void test_to_ranges() {
  int split_level = 10;
  GeoHashBits h((1LL<<17) + 327, 9);
  cout << "hash " << to_bits(h.bits, 18) << endl;
  GeoPoint::Range range = GeoPoint::to_range(h);
  cout << range.first << " " << range.second << endl;

  vector <GeoPoint::Range> ranges;
  GeoPoint::to_ranges(ranges, range, split_level);
  for (auto r : ranges) {
    cout << to_bits(r.first, 52) << " " << to_bits(r.second, 52) << endl;
  }
}

int main() {
  test_to_range();
  test_to_ranges();
  return 0;
}
