CC	= g++

CFLAGS = -std=c++11 -O2
SYSLIB = -rdynamic -lhiredis -lpthread -levent
REDIS_BASE_DIR = ../cpp-hiredis-cluster
INCLUDES = -I$(REDIS_BASE_DIR)/include

all: update_benchmark rectangle_benchmark generate_rectangles
tests: geohash_test geopoint_test

geohash.o: src/geohash.cpp src/geohash.h
	$(CC) -c $(CFLAGS) $<

geopoint.o: src/geopoint.cpp src/geopoint.h
	$(CC) -c $(CFLAGS) $<

redis_client.o: src/redis_client.cpp src/redis_client.h src/redis_client_base.h src/geohash.h src/geopoint.h
	$(CC) -c $(CFLAGS) $(INCLUDES) $< $(SYSLIB)

smart_redis_client.o: src/smart_redis_client.cpp src/smart_redis_client.h src/redis_client_base.h src/geohash.h src/geopoint.h
	$(CC) -c $(CFLAGS) $(INCLUDES) $< $(SYSLIB)

update_benchmark: src/update_benchmark.cpp redis_client.o smart_redis_client.o geohash.o geopoint.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(SYSLIB) -o$@

rectangle_benchmark: src/rectangle_benchmark.cpp redis_client.o smart_redis_client.o geohash.o geopoint.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(SYSLIB) -o$@

geohash_test: src/geohash_test.cpp geohash.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(SYSLIB) -o$@

geopoint_test: src/geopoint_test.cpp geohash.o geopoint.o
	$(CC) $(CFLAGS) $^ -o$@

generate_rectangles: src/generate_rectangles.cpp
	$(CC) $(CFLAGS) $^ -o$@

.PHONY: clean

clean:
	rm *.o update_benchmark rectangle_benchmark geohash_test
