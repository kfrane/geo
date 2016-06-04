CC	= g++

CFLAGS = -std=c++11 -Wall
SYSLIB = -rdynamic -lhiredis -lpthread -levent
REDIS_BASE_DIR = ../cpp-hiredis-cluster
INCLUDES = -I$(REDIS_BASE_DIR)/include

all: update_benchmark rectangle_benchmark radius_benchmark generate_rectangles generate_circles
tests: geohash_test geopoint_test
debug: CFLAGS += -DDEBUG -g
debug: all

geohash.o: src/geohash.cpp src/geohash.h
	$(CC) -c $(CFLAGS) $<

geopoint.o: src/geopoint.cpp src/geopoint.h
	$(CC) -c $(CFLAGS) $<

redis_client.o: src/redis_client.cpp src/redis_client.h src/redis_client_base.h src/geohash.h src/geopoint.h
	$(CC) -c $(CFLAGS) $(INCLUDES) $< $(SYSLIB)

smart_redis_client.o: src/smart_redis_client.cpp src/smart_redis_client.h src/redis_client_base.h src/geohash.h src/geopoint.h
	$(CC) -c $(CFLAGS) $(INCLUDES) $< $(SYSLIB)

balanced_redis_client.o: src/balanced_redis_client.cpp src/balanced_redis_client.h src/redis_client_base.h src/geohash.h src/geopoint.h
	$(CC) -c $(CFLAGS) $(INCLUDES) $< $(SYSLIB)

update_benchmark: src/update_benchmark.cpp redis_client.o smart_redis_client.o balanced_redis_client.o geohash.o geopoint.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(SYSLIB) -o$@

rectangle_benchmark: src/rectangle_benchmark.cpp redis_client.o smart_redis_client.o balanced_redis_client.o geohash.o geopoint.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(SYSLIB) -o$@

radius_benchmark: src/radius_benchmark.cpp redis_client.o smart_redis_client.o balanced_redis_client.o geohash.o geopoint.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(SYSLIB) -o$@

geohash_test: src/geohash_test.cpp geohash.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(SYSLIB) -o$@

geopoint_test: src/geopoint_test.cpp geohash.o geopoint.o
	$(CC) $(CFLAGS) $^ -o$@

generate_rectangles: src/generate_rectangles.cpp
	$(CC) $(CFLAGS) $^ -o$@

generate_circles: src/generate_circles.cpp
	$(CC) $(CFLAGS) $^ -o$@

# Only for local testing
local: redis_hello
redis_hello: src/redis_hello.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $< $(SYSLIB) -o$@

.PHONY: clean

clean:
	rm *.o update_benchmark rectangle_benchmark geohash_test
