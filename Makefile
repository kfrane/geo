CC	= g++

CFLAGS = -std=c++11 -g
SYSLIB = -rdynamic -lhiredis -lpthread -levent
REDIS_BASE_DIR = ../cpp-hiredis-cluster
INCLUDES = -I$(REDIS_BASE_DIR)/include

all: update_benchmark rectangle_benchmark

geohash.o: src/geohash.cpp src/geohash.h
	$(CC) -c $<

geopoint.o: src/geopoint.cpp src/geopoint.h
	$(CC) -c $<

redis_client.o: src/redis_client.cpp src/redis_client.h src/geohash.h src/geopoint.h
	$(CC) -c $(CFLAGS) $(INCLUDES) $< $(SYSLIB)

update_benchmark: src/update_benchmark.cpp redis_client.o geohash.o geopoint.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(SYSLIB) -o$@

rectangle_benchmark: src/rectangle_benchmark.cpp redis_client.o geohash.o geopoint.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(SYSLIB) -o$@

test: redis_client.o geohash.o geopoint.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(SYSLIB) -o$@

.PHONY: clean

clean:
	rm *.o update_benchmark rectangle_benchmark
